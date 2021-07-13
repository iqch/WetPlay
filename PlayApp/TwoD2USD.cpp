///////////////////////////////////////////////////////////////////
//
// wetcloth library standalone simulation app - usd-serializer class
//
// ©2021 Egor N. Chashchin iqcook@gmail.com
// 
//
// wetCloth lib : Copyright 2018 Yun (Raymond) Fei, Christopher Batty, Eitan Grinspun, and
// Changxi Zheng
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.



// CRT

#include <iostream>
#include <string>
#include <numeric>
using namespace std;

// WET

// USD
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/common.h>
//#include <pxr/usd/usd/treeIterator.h>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>

#include <pxr/usd/ar/resolver.h>

#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/path.h>

#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/scope.h>

#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdGeom/xform.h>

//#include <pxr/usd/usdGeom/curves.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/points.h>

using namespace pxr;


#include "ParticleSimulation.h"
#include "TwoD2USD.h"

TwoD2USD::TwoD2USD(const std::string& name, const ParticleSimulation& sim)
    : m_stage(UsdStage::CreateNew(name.c_str()))
{
    assert(m_stage);

    TfToken upAxis(string("Y"));
    UsdGeomSetStageUpAxis(m_stage, upAxis);

    m_stage->SetStartTimeCode(0);

    SdfPath simpath("/Simulation");
    TfToken scopetype("Scope");

    m_stage->DefinePrim(simpath, scopetype);


    // DEFINE CLOTH
    SdfPath clothpath("/Simulation/Cloth");
    TfToken meshtype("Mesh");

    UsdPrim cloth = m_stage->DefinePrim(clothpath, meshtype);

    UsdGeomMesh mesh(cloth);

    // SETUP CLOTH TOPOLOGY
    m_cloth_p_attr = mesh.GetPointsAttr();
    UsdAttribute fc_attr(mesh.GetFaceVertexCountsAttr());
    UsdAttribute fi_attr(mesh.GetFaceVertexIndicesAttr());

    m_cloth_ext_attr = mesh.GetExtentAttr();

    // ADOPT
    const TwoDScene& scene = sim.getScene();

    const VectorXs& x = scene.getX();

    //const std::vector<int> group = scene.getParticleGroup();
    const int num_edges = scene.getNumEdges();
    const int num_soft_elasto = scene.getNumSoftElastoParticles();
    const int num_faces = scene.getNumFaces();
    const MatrixXi& faces = scene.getFaces();


    VtIntArray fc(num_faces);
    m_cloth_fi.resize(num_faces * 3);

    int cloth_maxf = -1;


    for (int j = 0; j < num_faces; j++)
    {
        const Vector3iT& f = faces.row(j);

        m_cloth_fi[3 * j] = f[2]; //indd++; //face->v[0]->index;
        m_cloth_fi[3 * j + 1] = f[1]; // indd; indd++; //face->v[1]->index;
        m_cloth_fi[3 * j + 2] = f[0]; // indd; indd++; //face->v[2]->index;

        cloth_maxf = std::max(f[0], cloth_maxf);
        cloth_maxf = std::max(f[1], cloth_maxf);
        cloth_maxf = std::max(f[2], cloth_maxf);

        fc[j] = 3;
    };


    m_cloth_P.resize(cloth_maxf + 1);

    for (int ind : m_cloth_fi)
    {
        Vector3s X = x.segment<3>(ind * 4);
        m_cloth_P[ind].Set(X[0], X[1], X[2]);
    };

    // SET

    fc_attr.Set(fc);
    fi_attr.Set(m_cloth_fi);

    m_cloth_p_attr.Set(m_cloth_P, 0);
    //m_cloth_v_attr.Set(V, 1);

    VtVec3fArray Ext;
    mesh.ComputeExtent(m_cloth_P, &Ext);
    m_cloth_ext_attr.Set(Ext, 0);

	// DEFINE LIQUID
    SerializePacket LIQUID;
    m_nliquid = collectLiquid(scene, LIQUID);

    if (m_nliquid > 0)
    {
        TfToken pointstype("Points");
        SdfPath liquidpath("/Simulation/Liquid");

        UsdPrim liquidPrim = m_stage->DefinePrim(liquidpath, pointstype);

        UsdGeomPoints liquid(liquidPrim);

        m_liquid_P.resize(m_nliquid);
        m_liquid_p_attr = liquid.CreatePointsAttr();

        m_liquid_width.resize(m_nliquid);
        m_liquid_width_attr = liquid.CreateWidthsAttr();

        for (int i = 0; i < m_nliquid; i++)
        {
            m_liquid_P[i][0] = LIQUID.m_fluid_vertices[i][0];
            m_liquid_P[i][1] = LIQUID.m_fluid_vertices[i][1];
            m_liquid_P[i][2] = LIQUID.m_fluid_vertices[i][2];

            m_liquid_width[i] = LIQUID.m_fluid_radii[i];
        }

        m_liquid_p_attr.Set(m_liquid_P, 0);
        m_liquid_width_attr.Set(m_liquid_width,0);

        liquid.SetWidthsInterpolation(UsdGeomTokens->vertex);

        VtVec3fArray Ext;
        m_liquid_ext_attr = liquid.CreateExtentAttr();
        liquid.ComputeExtent(m_liquid_P, m_liquid_width, &Ext);
        m_liquid_ext_attr.Set(Ext, 0);
    }
    // 
	// ...DEFINE SPRINGS


	// ...DEFINE HAIRS
    SerializePacket HAIRS;
    m_nhairs = collectHairs(scene, HAIRS);

    if (m_nhairs > 0)
    {
        TfToken curvestype("BasisCurves");
        SdfPath hairpath("/Simulation/Hairs");

        UsdPrim hairPrim = m_stage->DefinePrim(hairpath, curvestype);

        UsdGeomBasisCurves hairs(hairPrim);

        hairs.CreateTypeAttr().Set(UsdGeomTokens->linear);

        UsdAttribute h_vc_attr(hairs.CreateCurveVertexCountsAttr());
        VtIntArray h_vc(m_nhairs);

        m_hairs_P.resize(2 * m_nhairs);
        m_hairs_p_attr = hairs.CreatePointsAttr();

        m_hairs_width.resize(2 * m_nhairs);
        m_hairs_width_attr = hairs.CreateWidthsAttr();

        for (int i = 0; i < m_nhairs; i++)
        {
            h_vc[i] = 2;
            
            auto IDX = HAIRS.m_hair_indices[i];

            m_hairs_width[2 * i] = HAIRS.m_hair_radii[IDX[0]][0];
            m_hairs_width[2*i+1] = HAIRS.m_hair_radii[IDX[1]][1];

            // P0
            auto P0 = HAIRS.m_hair_vertices[IDX[0]];

            m_hairs_P[i * 2][0] = P0[0];
            m_hairs_P[i * 2][1] = P0[1];
            m_hairs_P[i * 2][2] = P0[2];

            // P1
            auto P1 = HAIRS.m_hair_vertices[IDX[1]];

            m_hairs_P[i * 2+1][0] = P1[0];
            m_hairs_P[i * 2+1][1] = P1[1];
            m_hairs_P[i * 2+1][2] = P1[2];
        };
        
        h_vc_attr.Set(h_vc);

        m_hairs_p_attr.Set(m_hairs_P, 0);

        hairs.SetWidthsInterpolation(UsdGeomTokens->varying);
        m_hairs_width_attr.Set(m_hairs_width,0);

        VtVec3fArray Ext;
        m_hairs_ext_attr = hairs.CreateExtentAttr();
        hairs.ComputeExtent(m_hairs_P, m_hairs_width, &Ext);
        m_hairs_ext_attr.Set(Ext, 1);
    }

	// ...DEFINE INTERNALS
	// ...DEFINE EXTERNAL


	m_stage->SetEndTimeCode(0);

	m_stage->GetRootLayer()->Save();
};

TwoD2USD::~TwoD2USD()
{
	m_stage->GetRootLayer()->Save();
};

bool TwoD2USD::valid()
{
	return (m_stage!=NULL);
};

bool TwoD2USD::save(const ParticleSimulation& sim, int frame)
{
    cout << "SAVE USD FRAME " << frame << endl;

    const TwoDScene& scene = sim.getScene();

    // CLOTH
    {
        const VectorXs& x = scene.getX();

        for (int ind : m_cloth_fi)
        {
            Vector3s X = x.segment<3>(ind * 4);
            m_cloth_P[ind].Set(X[0], X[1], X[2]);
        };

        m_cloth_p_attr.Set(m_cloth_P, frame);

        VtVec3fArray Ext;

        UsdGeomMesh mesh(m_cloth_ext_attr.GetPrim());
        mesh.ComputeExtent(m_cloth_P, &Ext);
        m_cloth_ext_attr.Set(Ext, frame);
    }

    // HAIRS
    if(m_nhairs > 0)
    {
        SerializePacket HAIRS;
        m_nhairs = collectHairs(scene, HAIRS);

        for (int i = 0; i < m_nhairs; i++)
        {
            auto IDX = HAIRS.m_hair_indices[i];

            m_hairs_width[2 * i] = HAIRS.m_hair_radii[IDX[0]][0];
            m_hairs_width[2 * i + 1] = HAIRS.m_hair_radii[IDX[1]][1];


            // P0
            auto P0 = HAIRS.m_hair_vertices[IDX[0]];

            m_hairs_P[i * 2][0] = P0[0];
            m_hairs_P[i * 2][1] = P0[1];
            m_hairs_P[i * 2][2] = P0[2];

            // P1
            auto P1 = HAIRS.m_hair_vertices[IDX[1]];

            m_hairs_P[i * 2 + 1][0] = P1[0];
            m_hairs_P[i * 2 + 1][1] = P1[1];
            m_hairs_P[i * 2 + 1][2] = P1[2];
        };

        m_hairs_p_attr.Set(m_hairs_P, frame);

        m_hairs_width_attr.Set(m_hairs_width, frame);

        VtVec3fArray Ext;
        UsdGeomBasisCurves(m_hairs_ext_attr.GetPrim())
            .ComputeExtent(m_hairs_P, m_hairs_width, &Ext);
        m_hairs_ext_attr.Set(Ext, frame);
    }

    // LIQUID
    if (m_nliquid > 0)
    {
        SerializePacket LIQUID;
        collectLiquid(scene, LIQUID);

        for (int i = 0; i < m_nliquid; i++)
        {
            m_liquid_P[i][0] = LIQUID.m_fluid_vertices[i][0];
            m_liquid_P[i][1] = LIQUID.m_fluid_vertices[i][1];
            m_liquid_P[i][2] = LIQUID.m_fluid_vertices[i][2];

            m_liquid_width[i] = LIQUID.m_fluid_radii[i];
        }

        m_liquid_p_attr.Set(m_liquid_P, frame);
        m_liquid_width_attr.Set(m_liquid_width, frame);

        VtVec3fArray Ext;
        UsdGeomPoints(m_liquid_ext_attr.GetPrim())
            .ComputeExtent(m_liquid_P, m_liquid_width, &Ext);
        m_liquid_ext_attr.Set(Ext, frame);
    }

    // SAVE
	m_stage->SetEndTimeCode(frame);

    //if(frame%10 == 0) 
        m_stage->GetRootLayer()->Save();

	return true;
};

int TwoD2USD::collectHairs(const TwoDScene& scene , SerializePacket& hair)
{
    const VectorXs& x = scene.getX();
    const VectorXs& r = scene.getRadius();
    //const VectorXs& vol = scene.getVol();
    //const VectorXs& fvol = scene.getFluidVol();
    //const std::vector<int>& group = scene.getParticleGroup();

    const int num_soft_elasto = scene.getNumSoftElastoParticles();
    const MatrixXi& edges = scene.getEdges();
    if (num_soft_elasto == 0 || edges.rows() == 0) return 0;

    std::vector<int> hair_indicator(num_soft_elasto);
    threadutils::for_each(0, num_soft_elasto, [&](int pidx) 
    {
        const std::vector<int>& p2e = scene.getParticleEdges(pidx);

        if (p2e.size() > 0) hair_indicator[pidx] = 1;
        else hair_indicator[pidx] = 0;
    });

    std::partial_sum(hair_indicator.begin(), hair_indicator.end(),
        hair_indicator.begin());

    const int num_hair_parts = hair_indicator[hair_indicator.size() - 1];

    if (num_hair_parts == 0) return 0;

    int nhair = edges.rows();

    hair.m_hair_vertices.resize(num_hair_parts);
    hair.m_hair_indices.resize(nhair);
    //hair.m_hair_sat.resize(num_hair_parts);
    hair.m_hair_radii.resize(num_hair_parts);
    //hair.m_hair_group.resize(num_hair_parts);

    threadutils::for_each(0, num_soft_elasto, [&](int pidx)
    {
        const std::vector<int>& p2e = scene.getParticleEdges(pidx);
        if (p2e.size() == 0) return 0;

        const int mapped_idx = hair_indicator[pidx] - 1;
        hair.m_hair_vertices[mapped_idx] = x.segment<3>(pidx * 4);
        hair.m_hair_radii[mapped_idx] = r.segment<2>(pidx * 2);
        //hair.m_hair_sat[mapped_idx] = fvol(pidx) / std::max(1e-16, vol(pidx));
        //hair.m_hair_group[mapped_idx] = group[pidx];
    });

    threadutils::for_each(0, (int)edges.rows(), [&](int eidx)
    {
        const int mapped_idx_0 = hair_indicator[edges(eidx, 0)] - 1;
        const int mapped_idx_1 = hair_indicator[edges(eidx, 1)] - 1;

        hair.m_hair_indices[eidx] = Vector2i(mapped_idx_0, mapped_idx_1);
    });

    return nhair;
};

int TwoD2USD::collectLiquid(const TwoDScene& scene, SerializePacket& fluid)
{
    int num = scene.getNumFluidParticles();

    if (num == 0) return 0;

    fluid.m_fluid_vertices.resize(num);
    fluid.m_fluid_radii.resize(num);

    const std::vector<int>& indices = scene.getFluidIndices();
    const VectorXs& x = scene.getX();
    const VectorXs& r = scene.getRadius();

    threadutils::for_each(0, num, [&](int idx)
    {
        fluid.m_fluid_vertices[idx] = x.segment<3>(indices[idx] * 4);
        fluid.m_fluid_radii[idx] = r(indices[idx] * 2 + 0);
    });

    return num;
};
