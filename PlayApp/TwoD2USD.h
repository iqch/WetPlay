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

#pragma once

// CTR
#include <iostream>

// USD
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/attribute.h>
using namespace pxr;


class ParticleSimulation;

struct TwoD2USD
{
	TwoD2USD(const std::string& name, const ParticleSimulation& sim);
	virtual ~TwoD2USD();

	bool valid();

	bool save(const ParticleSimulation& sim, int frame);

private :

	//bool finalize();
	UsdStageRefPtr m_stage;

	// CLOTH
	VtIntArray m_cloth_fi;
	VtVec3fArray m_cloth_P;
	UsdAttribute m_cloth_p_attr;
	UsdAttribute m_cloth_ext_attr;
	//UsdAttribute m_cloth_v_attr;

	// LIQUID
	int m_nliquid;
	int collectLiquid(const TwoDScene& scene, SerializePacket& fluid);
	VtVec3fArray m_liquid_P;
	UsdAttribute m_liquid_p_attr;
	VtFloatArray m_liquid_width;
	UsdAttribute m_liquid_width_attr;
	UsdAttribute m_liquid_ext_attr;

	// HAIR
	int m_nhairs;
	int collectHairs(const TwoDScene&, SerializePacket&);
	VtVec3fArray m_hairs_P;
	UsdAttribute m_hairs_p_attr;
	VtFloatArray m_hairs_width;
	UsdAttribute m_hairs_width_attr;
	UsdAttribute m_hairs_ext_attr;

	//UsdPrim m_springs;
	//UsdPrim m_external;
	//UsdPrim m_internals;

};
