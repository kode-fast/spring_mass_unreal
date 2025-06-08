#pragma once

#include "particle_system.h"
#include "CoreMinimal.h"
#include <vector>

// ParticalSimulator will be to class that runs the whole simulation
class ParticleSimulator
{
public:
	// variables 

	// use a REFERANCE to particle system
	ParticleSystem* system;
	// ground penelty forces 
	double Gks;
	double Gkd;
	// gravity
	double g;
	// global drag coefficient
	double kdrag;

	// assume ground position is (0,0,0) 
	// ground direction vector (points strait up) ( y axis )
	FVector ground;
	FVector grav; // -gravity direction

	double Mass;
	FVector Force;
	int intigrateType;
	// ----------COUNTERS----------
	// number of particles in simulation - get from system ? 
	//int numParticles;
	//int numSprings;
	int numGroundSprings;
	double timeStep;

	// -----LIST / CONTAINERS------
	// TODO where to put spring lists ?
	// array of springs

	// TODOD switch these to TArravy<Springs>
	std::vector<Spring*> springs;
	// std::vector<Spring*> groundSprings;

	// functions
	// 
	// defult constructor (empty simulation) 
	ParticleSimulator();
	ParticleSimulator(ParticleSystem* p);

	int UpdateSpring(int spring_idx, double length, double kd, double, double ks);

	int step(double time, double deltaTIme, FVector outside_vel);

	TArray<FVector> update();

	void integrate(FVector pos, FVector vel, int index);
	void euler(FVector pos, FVector vel, int index);
	void symplectic(FVector pos, FVector vel, int index);
	void verlet(FVector pos, FVector vel, int index);

	void springsForce();

	// functions not used
	//void groundForcePenelty();
	//bool groundCollision(FVector pos);


};
