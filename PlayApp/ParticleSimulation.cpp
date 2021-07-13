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


// CRT
#include <iostream>
#include <string>

// WET
#include "Core/MemUtilities.h"
#include "Core/TimingUtilities.h"


#include "ParticleSimulation.h"


const static char* g_timing_labels[] = {
    "Sample, Merge and Split Particles",
    "Build Sparse MAC Grid",
    "Compute Weight, Solid Stress, and Distance Field",
    "APIC Particle/Vertex-to-Grid Transfer",
    "Force Integration and Velocity Prediction",
    "Solve Poisson Equation",
    "Solve Solid Velocity",
    "Solve Liquid Velocity",
    "Particle Correction",
    "APIC Grid-to-Particle/Vertex Transfer",
    "Particle/Vertex Advection",
    "Liquid Capturing",
    "Liquid Dripping",
    "Solve Quasi-Static Equation",
    "Update Deformation Gradient and Plasticity"};

const static int num_timing_labels =
    sizeof(g_timing_labels) / sizeof(const char*);

ParticleSimulation::ParticleSimulation(
    const std::shared_ptr<TwoDScene>& scene,
    const std::shared_ptr<SceneStepper>& scene_stepper)
    : m_core(std::make_shared<WetClothCore>(scene, scene_stepper))
{}

//ParticleSimulation::~ParticleSimulation() {}

void ParticleSimulation::stepSystem(const scalar& dt)
{
    m_core->stepSystem(dt);

    const std::vector<scalar>& timing_buffer = m_core->getTimingStatistics();

    scalar total_time = 0.0;
    for (scalar t : timing_buffer) total_time += t;

    std::cout << "---------------------------------" << std::endl;
    for (int i = 0; i < num_timing_labels; ++i)
    {
        scalar avg_time = (timing_buffer[i] / (scalar)(m_core->getCurrentTime() + 1));
        scalar prop = timing_buffer[i] / total_time * 100.0;
        std::cout << g_timing_labels[i] << ", " << avg_time << ", " << prop << "%" << std::endl;
    }

    const scalar divisor = (scalar)(m_core->getCurrentTime() + 1);

    std::cout << "---------------------------------" << std::endl;
    std::cout << "Total Time (per Frame), " << total_time << ", " << (total_time / divisor) << std::endl;
    scalar part_fluid_vol = m_core->getScene()->totalFluidVolumeParticles();
    scalar vert_fluid_vol = m_core->getScene()->totalFluidVolumeSoftElasto();
    std::cout << "Liquid Vol, " << part_fluid_vol << ", " << vert_fluid_vol
        << ", " << (part_fluid_vol + vert_fluid_vol) << std::endl;

    int peak_idx = 0;
    int cur_idx = 0;

    const char* mem_units[] = {"B", "KB", "MB", "GB", "TB", "PB"};

    size_t peak_usage = memutils::getPeakRSS();
    scalar peak_mem = (scalar)peak_usage;
    while (peak_mem > 1024.0 && peak_idx < (int)(sizeof(mem_units) / sizeof(char*))) 
    {
        peak_mem /= 1024.0;
        peak_idx++;
    }

    const WetClothCore::Info& info = m_core->getInfo();

    scalar avg_mem = info.m_mem_usage_accu / divisor;
    while (avg_mem > 1024.0 && cur_idx < (int)(sizeof(mem_units) / sizeof(char*))) 
    {
        avg_mem /= 1024.0;
        cur_idx++;
    }

    std::cout << "Particles (Avg.), " << m_core->getScene()->getNumParticles()
            << ", " << (info.m_num_particles_accu / divisor)
            << ", Fluid (Avg.), " << m_core->getScene()->getNumFluidParticles()
            << ", " << (info.m_num_fluid_particles_accu / divisor)
            << ", Elements, " << m_core->getScene()->getNumGausses() << ", "
            << (info.m_num_elements_accu / divisor) << std::endl;
    std::cout << "Peak Mem Usage, " << peak_mem << mem_units[peak_idx]
            << ", Avg Mem Usage, " << avg_mem << mem_units[cur_idx]
            << std::endl;

    std::cout << "---------------------------------" << std::endl;
}

//void ParticleSimulation::readPos(const std::string& fn_pos) {
//  std::ifstream ifs(fn_pos, std::ios::binary);
//  //m_scene_serializer.loadPosOnly(*m_core->getScene(), ifs);
//  ifs.close();
//}

//void ParticleSimulation::serializePositionOnly(const std::string& fn_pos) {
//  //m_scene_serializer.serializePositionOnly(*m_core->getScene(), fn_pos);
//}

//void ParticleSimulation::serializeScene(
//    const std::string& fn_clothes, const std::string& fn_hairs,
//    const std::string& fn_fluid, const std::string& fn_internal_boundaries,
//    const std::string& fn_external_boundaries, const std::string& fn_spring) {
//  m_scene_serializer.serializeScene(*m_core->getScene(), fn_clothes, fn_hairs,
//                                    fn_fluid, fn_internal_boundaries,
//                                    fn_external_boundaries, fn_spring);
//}
//
//std::string ParticleSimulation::getSolverName() {
//  return m_core->getSceneStepper()->getName();
//}

//void ParticleSimulation::printDDA() { m_core->getScene()->computeDDA(); }

//void ParticleSimulation::finalInit() {
//  m_scene_serializer.initializeFaceLoops(*m_core->getScene());
//}

//const LiquidInfo& ParticleSimulation::getLiquidInfo() {
//  return m_core->getScene()->getLiquidInfo();
//}
