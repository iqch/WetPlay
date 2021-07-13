//
// This file was part of the libWetCloth open source project, 
// but heavily modified later by iqcook@gmail.com
//
// but Copyright 2018 Yun (Raymond) Fei, Christopher Batty, Eitan Grinspun, and
// Changxi Zheng
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// CRT
#include <string>
#include <iostream>
#include <thread>

// LIBS
#include <Eigen/StdVector>

// LIB WET
#include <Core/Force.h>
#include <Core/MathDefs.h>
#include <Core/StringUtilities.h>
#include <Core/TimingUtilities.h>
#include <Core/TwoDScene.h>

#include "ParticleSimulation.h"
#include "TwoDSceneSerializer.h"
#include "TwoDSceneXMLParser.h"

#include "TwoD2USD.h"

int main(int argc, char** argv) 
{
    Eigen::initParallel();
    Eigen::setNbThreads(std::thread::hardware_concurrency());

    srand(0x0108170F);

    std::string scene_file = argv[1];

    std::string output_file = scene_file + ".usd";

    if (argc > 2) output_file = argv[2];

    TwoDSceneXMLParser xml_scene_parser;
    scalar dt, max_time;

    ParticleSimulation& simulation =
        xml_scene_parser.loadExecutableSimulation(
            scene_file,
            dt, max_time);

    int num_steps = ceil(max_time / dt);
    int current_step = 1;

    TwoD2USD saviour(output_file,simulation);

    assert(saviour.valid());

    while (true)
    {
        while (current_step <= num_steps)
        {
            std::cout << "****** STEP " << current_step << " OF " << num_steps << std::endl;

            simulation.stepSystem(dt);
            saviour.save(simulation,current_step);
            current_step++;
       }

        std::cout << "Complete Simulation! Enter time to continue (exit with 0): "  << std::endl;

        int new_time = 0;
        std::cin >> new_time;
        if (new_time < 1) break;

        num_steps += new_time;
    }

    return 0;
}


