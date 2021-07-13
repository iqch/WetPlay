//
// This file was part of the libWetCloth open source project, 
// but heavily modified later by iqcook@gmail.com
//
// Copyright 2018 Yun (Raymond) Fei, Christopher Batty, Eitan Grinspun, and
// Changxi Zheng
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef PARTICLE_SIMULATION_H
#define PARTICLE_SIMULATION_H

#include "Core/SceneStepper.h"
#include "Core/TwoDScene.h"
#include "Core/WetClothCore.h"

#include "TwoDSceneSerializer.h"

class ParticleSimulation 
{
 public:
  ParticleSimulation(const std::shared_ptr<TwoDScene>& scene,
                     const std::shared_ptr<SceneStepper>& scene_stepper);

  virtual ~ParticleSimulation() {};
  /////////////////////////////////////////////////////////////////////////////
  // Simulation Control Functions

  void stepSystem(const scalar& dt);

  const TwoDScene& getScene() const { return *(m_core->getScene()); };

  /////////////////////////////////////////////////////////////////////////////
  // Serialization Functions
  //void serializeScene(const std::string& fn_clothes,
  //                    const std::string& fn_hairs, const std::string& fn_fluid,
  //                    const std::string& fn_internal_boundaries,
  //                    const std::string& fn_external_boundaries,
  //                    const std::string& fn_spring);

  //void serializePositionOnly(const std::string& fn_pos);

  //void readPos(const std::string& fn_pos);
  /////////////////////////////////////////////////////////////////////////////
  // Status Functions

  //std::string getSolverName();

  //void finalInit();

  //void printDDA();

  //const LiquidInfo& getLiquidInfo();

 private:
  std::shared_ptr<WetClothCore> m_core;
  //TwoDSceneSerializer m_scene_serializer;
};

#endif
