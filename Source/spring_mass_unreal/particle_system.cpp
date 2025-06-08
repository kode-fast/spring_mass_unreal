# pragma once

#include "particle_system.h"

ParticleSystem::ParticleSystem() 
{
    numSprings = 0;
}


ParticleSystem::ParticleSystem(TArray<FVector> verts, int numVerts, double mass)
{
   
    numSprings = 0;
    particles.SetNum(numVerts);

    // constuct array of particles  
    for (int i = 0; i < numVerts; i++) {
        // use particles[i] directly to stop the varible a from getting dealocated on the stack
        particles[i].pos = verts[i];
        particles[i].prevPos = verts[i];
        particles[i].pos0 = verts[i];
        particles[i].vel = FVector(0.0, 0.0, 0.0);
        particles[i].fixed = false;
        particles[i].groundCol = false;
        particles[i].mass = mass;

    }

    numParticles = numVerts;
}

void ParticleSystem::setSpring(int indexA, int indexB, double restLen, double kd, double ks) 
{
    Spring spring;
    spring.indexA = indexA;
    spring.indexB = indexB;
    spring.restLen = restLen;
    spring.currLen = restLen;
    // ks is spring stiffness - higher is more snappy to the rest legnth, lower is more springy 
    spring.ks = ks;
    // kd is spring dampaning - higher number is stiffer spring (more energy loss to the spring) , lower number is sprigyer more occilations
    spring.kd = kd;
    spring.ForceAtoB = FVector(0, 0, 0);
    springs.push_back(spring);
    numSprings = numSprings + 1;
}