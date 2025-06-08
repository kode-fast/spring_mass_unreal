#pragma once

#include "CoreMinimal.h"


struct Spring 
{

    int indexA; // index if 1st sprint attachment
    int indexB; // index if 1st sprint attachment

    // TODO dont need currLen ? 
    double currLen;
    double restLen;

    // static spring constant
    double ks;
    // dynamic spring constat
    double kd;

    FVector ForceAtoB;


};

struct Particle 
{

    FVector vel;
    double mass;
    FVector pos;
    FVector prevPos;

    // origonal position 
    FVector pos0;

    bool fixed; // if the particel has fixed pos
    bool groundCol; // ground collision flag

    FVector getPos() {
        return pos;
    }
};

class ParticleSystem
{
public:

    int numParticles;
    int numSprings;
    TArray<Particle> particles;
    // use std vector for memory managment as we want to be able to add springs dynamicly 
    // TODO change to use TArray<Spring>
    std::vector<Spring> springs;
    std::vector<Spring> groundSprings;
    // defult constructor
    ParticleSystem();
    
    ParticleSystem(TArray<FVector> verts, int sizeVerts, double mass);

    void setSpring(int indexA, int indexB, double restLen, double kd, double ks);


};

  