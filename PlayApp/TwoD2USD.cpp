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

using namespace pxr;


#include "ParticleSimulation.h"
#include "TwoD2USD.h"

TwoD2USD::TwoD2USD(const std::string& name, const ParticleSimulation& sim)
	: m_stage(UsdStage::CreateNew((name + ".usdc").c_str()))
{
	assert(m_stage);

	TfToken upAxis(string("Y"));
	UsdGeomSetStageUpAxis(m_stage, upAxis);

	m_stage->SetStartTimeCode(1);

	SdfPath simpath("/Simulation");
	TfToken scopetype("Scope");

	m_stage->DefinePrim(simpath, scopetype);


	// DEFINE CLOTH
	SdfPath clothpath("/Simulation/Cloth");
	TfToken meshtype("Mesh");

	UsdPrim cloth = m_stage->DefinePrim(clothpath,meshtype);

    UsdGeomMesh mesh(cloth);

	// ...SETUP CLOTH TOPOLOGY

    m_cloth_p_attr = mesh.GetPointsAttr();
    UsdAttribute fc_attr(mesh.GetFaceVertexCountsAttr());
    UsdAttribute fi_attr(mesh.GetFaceVertexIndicesAttr());

    //m_cloth_v_attr = mesh.GetVelocitiesAttr();

    m_cloth_ext_attr = mesh.GetExtentAttr();

    // ...ADOPT

    const TwoDScene& scene = sim.getScene();

    //scene.dump_geometry()


        //threadutils::for_each(0, num_soft_elasto, [&](int pidx)

        //    const std::vector<std::pair<int, scalar> >& p2f =
        //    scene.getParticleFaces(pidx);


    const VectorXs& x = scene.getX();

    //const std::vector<int> group = scene.getParticleGroup();
    const int num_edges = scene.getNumEdges();
    const int num_soft_elasto = scene.getNumSoftElastoParticles();
    const int num_faces = scene.getNumFaces();
    const MatrixXi& faces = scene.getFaces();




    //template <typename Index, typename Callable>
    //static void for_each(Index start, Index end, Callable func) {

    //    for (Index i = 0; i < num_faces; ++i)
    //    {
    //        func = [&](int fidx);
    //    }

    //}




//////////////////////


    VtIntArray fc(num_faces);
    
    
    m_cloth_fi.resize(num_faces * 3);

    int cloth_maxf = -1;

    //VtVec3fArray V(fcnt * 3);


    for (int j = 0; j < num_faces; j++)
    {
        //const Face* face; // = mesh.faces[j];
        const Vector3iT& f = faces.row(j);

        m_cloth_fi[3 * j] = f[2]; //indd++; //face->v[0]->index;
        m_cloth_fi[3 * j + 1] = f[1]; // indd; indd++; //face->v[1]->index;
        m_cloth_fi[3 * j + 2] = f[0]; // indd; indd++; //face->v[2]->index;

        cloth_maxf = std::max(f[0], cloth_maxf);
        cloth_maxf = std::max(f[1], cloth_maxf);
        cloth_maxf = std::max(f[2], cloth_maxf);

        //if(j < 15) cout << "F : " << f[0] << " " << f[1] << " " << f[2] << endl;

        //Vector3s t0 = (x.segment<3>(f[1] * 4) - x.segment<3>(f[0] * 4));
        //Vector3s t1 = (x.segment<3>(f[2] * 4) - x.segment<3>(f[0] * 4));

        fc[j] = 3;

        //Vec3 X0 = face->v[0]->node->x;
        //P[3 * j].Set(X0[0], X0[1], X0[2]);
        //Vec3 X1 = face->v[1]->node->x;
        //P[3 * j + 1].Set(X1[0], X1[1], X1[2]);
        //Vec3 X2 = face->v[2]->node->x;
        //P[3 * j + 2].Set(X2[0], X2[1], X2[2]);

        //Vec3 V0 = face->v[0]->node->v;
        //V[3 * j].Set(V0[0], V0[1], V0[2]);
        //Vec3 V1 = face->v[1]->node->v;
        //V[3 * j + 1].Set(V1[0], V1[1], V1[2]);
        //Vec3 V2 = face->v[2]->node->v;
        //V[3 * j + 2].Set(V2[0], V2[1], V2[2]);
    };


    m_cloth_P.resize(cloth_maxf +1);

    for (int ind : m_cloth_fi)
    {
        Vector3s X = x.segment<3>(ind * 4);
        m_cloth_P[ind].Set(X[0], X[1], X[2]);
    };

    // SET

    //if (false)
    {
        fc_attr.Set(fc);
        fi_attr.Set(m_cloth_fi);

        m_cloth_p_attr.Set(m_cloth_P, 1);
        //m_cloth_v_attr.Set(V, 1);

        VtVec3fArray Ext;
        mesh.ComputeExtent(m_cloth_P, &Ext);
        m_cloth_ext_attr.Set(Ext, 1);
    };




	// ...DEFINE LIQUID
	// ...DEFINE SPRINGS
	// ...DEFINE HAIRS
	// ...DEFINE INTERNALS
	// ...DEFINE EXTERNAL


	m_stage->SetEndTimeCode(1);

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

    // ...SETUP CLOTH
    const TwoDScene& scene = sim.getScene();

    const VectorXs& x = scene.getX();

    for (int ind : m_cloth_fi)
    {
        Vector3s X = x.segment<3>(ind * 4);
        m_cloth_P[ind].Set(X[0], X[1], X[2]);
    }

    m_cloth_p_attr.Set(m_cloth_P, frame);
    //m_cloth_v_attr.Set(V, 1);

    VtVec3fArray Ext;

    UsdGeomMesh mesh(m_cloth_ext_attr.GetPrim());
    mesh.ComputeExtent(m_cloth_P, &Ext);
    m_cloth_ext_attr.Set(Ext, frame);


	m_stage->SetEndTimeCode(frame);

    if(frame%10 == 0) m_stage->GetRootLayer()->Save();

	return true;
};
