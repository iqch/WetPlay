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

#ifndef TWO_D_SCENE_XML_PARSER_H
#define TWO_D_SCENE_XML_PARSER_H

// CRT
#include <fstream>
#include <iostream>
#include <limits>

// LIBS
#include <Eigen/StdVector>
#include "rapidxml/rapidxml.hpp"


// WET
#include "Core/CohesionForce.h"
#include "Core/DER/StrandForce.h"
#include "Core/ElasticParameters.h"
#include "Core/DistanceFields.h"
#include "Core/JunctionForce.h"
#include "Core/LevelSetForce.h"
#include "Core/LinearizedImplicitEuler.h"
#include "Core/SimpleGravityForce.h"
#include "Core/SpringForce.h"
#include "Core/StringUtilities.h"
#include "Core/ThinShell/ThinShellForce.h"
#include "Core/TwoDScene.h"

#include "ParticleSimulation.h"

// REALLY USEFULL TODOs
//   TODO: Improve error messages to display all valid options, etc. Could
//   define an option class that knows its valid options and bounds on values.

// LESS USEFULL TODOs
//   TODO: Write method for computing number of a given property
//   TODO: Add some additional error checking for repeat properties, etc
//   TODO: Abstract out common code
//   TODO: Check for invalid properties

class TwoDSceneXMLParser {
 public:
     ParticleSimulation& loadExecutableSimulation(
         const std::string& file_name,
         scalar& dt, scalar& max_time); // , scalar& steps_per_sec_cap);

  // TODO: NEED AN EIGEN_ALIGNED_THING_HERE ?
 private:
     //ParticleSimulation& loadParticleSimulation(scalar& dt, rapidxml::xml_node<>* node);

  void loadLiquidInfo(rapidxml::xml_node<>* node,
                      const std::shared_ptr<TwoDScene>& twodscene);

  void loadXMLFile(const std::string& filename, std::vector<char>& xmlchars,
                   rapidxml::xml_document<>& doc);

  bool loadTextFileIntoString(const std::string& filename,
                              std::string& filecontents);

  void loadSimulationType(rapidxml::xml_node<>* node, std::string& simtype);

  void loadHairs(rapidxml::xml_node<>* node,
                 const std::shared_ptr<TwoDScene>& twodscene, const scalar& dt);

  void loadHairPose(rapidxml::xml_node<>* node,
                    const std::shared_ptr<TwoDScene>& twodscene);

  void loadClothes(rapidxml::xml_node<>* node,
                   const std::shared_ptr<TwoDScene>& twodscene);

  void loadSpringForces(rapidxml::xml_node<>* node,
                        const std::shared_ptr<TwoDScene>& twodscene);

  void loadSimpleGravityForces(rapidxml::xml_node<>* node,
                               const std::shared_ptr<TwoDScene>& twodscene);

  void loadParticles(rapidxml::xml_node<>* node,
                     const std::shared_ptr<TwoDScene>& twodscene,
                     int& maxgroup);

  void loadSceneTag(rapidxml::xml_node<>* node, std::string& scenetag);

  void loadDistanceFields(rapidxml::xml_node<>* node,
                          const std::shared_ptr<TwoDScene>& twodscene,
                          int& maxgroup);

  void loadElasticParameters(rapidxml::xml_node<>* node,
                            const std::shared_ptr<TwoDScene>& twodscene,
                            const scalar& dt);

  void loadBucketInfo(rapidxml::xml_node<>* node,
                      const std::shared_ptr<TwoDScene>& twodscene);

  void loadIntegrator(rapidxml::xml_node<>* node,
                      std::shared_ptr<SceneStepper>& scenestepper, scalar& dt);

  void loadScripts(rapidxml::xml_node<>* node,
                   const std::shared_ptr<TwoDScene>& twodscene);


  void loadMaxTime(rapidxml::xml_node<>* node, scalar& max_t);

  void loadMaxSimFrequency(rapidxml::xml_node<>* node, scalar& max_freq);

  void loadSceneDescriptionString(rapidxml::xml_node<>* node,
                                  std::string& description_string);
};

#endif
