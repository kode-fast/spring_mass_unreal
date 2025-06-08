#pragma once

#include "particle_simulator.h"

ParticleSimulator::ParticleSimulator() 
{ 
	Gks = 1; 
	Gkd = 1; 
	g = -9.8; 
	kdrag = 1; 
	ground = FVector(0, 0, 1); 
	grav = FVector(0, 0, 1); 
}

ParticleSimulator::ParticleSimulator(ParticleSystem* p)
{
	system = p;
	// ground penelty forces 
	Gks = 300;
	Gkd = 50;
	// gravity
	g = -10.0;
	// global drag coefficient
	kdrag = 1.0;
	// number of particles in simulation
	// numParticles = 0;
	// number of springs
	// numSprings = 0;
	// ground vector points sraight up (y is up)
	// N normal vector for ground
	ground = FVector(0, 0, 1);
	// grav vector points straight down (our grav acceleration includs the negative to make this point down)
	grav = FVector(0, 0, 1);

	numGroundSprings = 0;

	intigrateType = 1;

}


int ParticleSimulator::UpdateSpring(int spring_idx, double length, double kd, double, double ks) 
{
	system->springs[spring_idx].restLen = length;
	system->springs[spring_idx].kd = kd;
	system->springs[spring_idx].ks = ks;

	return 0;
}

// TODO make springs and groundsprings list part of simulator class not system ? 
int ParticleSimulator::step(double time, double deltaTime, FVector outside_vel)
{

	timeStep = deltaTime;
	// bool col;
	// FVector gravForce;
	// FVector dragForce;

	FVector tmp;

	FVector vel;
	FVector pos;

	// step 1 CALCULATE FORCE 
	// step 2 find new position and velocity
	// each step calculate spring forces

	springsForce();

	// NOTE dont need ground force
	//groundForcePenelty();

	// for each particle in the particle system 
	for (int i = 0; i < system->numParticles; i++) {

		if (system->particles[i].fixed) {
			continue;
		}
		pos = system->particles[i].pos;
		vel = system->particles[i].vel;

		Mass = system->particles[i].mass;

		// zero the force vector 
		Force = FVector(0.0, 0.0, 0.0);

		// F = -kd*vel + mass*gravity + External force (springs) / ground ect
		// find all springs with particle i and add spring force to force vector;
		for (int j = 0; j < system->numSprings; j++) {

			if (system->springs[j].indexA == i) {
				// positive
				tmp = system->springs[j].ForceAtoB;
				Force = Force + tmp;

			}

			else if (system->springs[j].indexB == i) {
				// negative
				tmp = system->springs[j].ForceAtoB;
				tmp = tmp * -1.0f;
				Force = Force + tmp;

			}

		}

		/*
		// NOTE dont need ground force 
		// tarible way to do this for optimization.
		for (int k = 0; k < numGroundSprings; k++) {

			if (groundSprings[k]->indexA == i) {
				// positive

				if (system->particles[i].groundCol) {
					// get force vector
					tmp = groundSprings[k]->ForceAtoB;
					// add it to Force vector
					Force = Force + tmp;
				}
				else {
					// TODO WHEN WE DELETE A DOWN SPRING WE NEED TO STOP ITS FORCE?
					// TODO ground springs are not relesing fast enough resulting in negitive force the step after being applied?
					// maby just need to tune Gks and Gkd?

					// getting alot more positive force after clearing the ground 
					groundSprings.erase(groundSprings.begin() + k);
					// aslo fergot to make deincremnt duhduh
					numGroundSprings -= 1;
				}

			}

		}
		*/

		// add gravety and drag to force vector 

		// drag
		tmp = vel * (float)-kdrag;
		Force = tmp + Force;
		// gravity
		tmp = grav * (float)(g * Mass);
		Force = tmp + Force + outside_vel;

		// TODO make better data structure for integate function
		integrate(pos, vel, i);

		// updated system pos directly in integrate function
		//system->particles[i].pos = pos;
		//system->particles[i].vel = vel;

		/*
		// NOTE dont need ground force

		// COLLISION DETECTION AND PENELTY SPRING SETTING
		col = groundCollision(pos);
		//printf("ground collision %d\n", col);
		//printf("particle ground collision %d\n", system->particles[i].groundCol);

		if (col && !system->particles[i].groundCol) {
			//printf("in ground spring added if statment\n");
			system->particles[i].groundCol = true;

			//	printf("particle ground collision %d\n", system->particles[i].groundCol);

				// TODO make sure to clean this up properly 
			Spring* s = new Spring();

			// setSpring(int indexA, int indexB, double ks, double kd, double restLength)
			// s.setSpring(i, -1, Gks, Gkd, 0);

			s->indexA = i;
			s->indexB = -1;
			s->kd = Gkd;
			s->ks = Gks;
			s->restLen = 0;
			s->ForceAtoB = FVector(0, 0, 0);

			groundSprings.push_back(s);
			// fergot to incremet groundsprings duh
			numGroundSprings += 1;
		}
		else if (system->particles[i].groundCol) {
			system->particles[i].groundCol = false;
		}
		*/
	}
	return 0;

}

// updates the rendered vertices positions from partical simulator step
TArray<FVector> ParticleSimulator::update() 
{


	TArray<FVector> new_verts;
	TArray<Particle> particles = system->particles;
	for (int i = 0; i < system->numParticles; i++) {

		new_verts.Add(particles[i].pos);
	}

	return new_verts;
}

void ParticleSimulator::integrate(FVector pos, FVector vel, int index) 
{

	// integration time step is independent of simulation time step?

	// symplectic most stable 
	if (intigrateType == 1) {
			symplectic(pos, vel, index);
	}
	else if (intigrateType == 0) {
		euler(pos, vel, index);
	}

	// varlet
	else {
		// if its the first iteration use euler - def better way of doing this 
		FVector tmp;
		tmp = system->particles[index].pos0;
		// prev position was never updated after euler 

		// TODO add tolerance to equal ? (p1, p2, 0.00001f)
		if (tmp.Equals(system->particles[index].prevPos, 0.0001f)) {
			//animTcl::OutputMessage("euler");
			euler(pos, vel, index);

			// TODO make elment wise vector assignment function ?
			system->particles[index].prevPos = pos;
		}
		else {
			verlet(pos, vel, index);
		}
	}

}


void ParticleSimulator::euler(FVector pos, FVector vel, int index) 
{


	FVector tmp;

	// position
	tmp = vel * (float)timeStep;

	system->particles[index].pos = pos + tmp;

	// pos = pos + vel * timestep

	// velocity 
	tmp = Force * (float)(1.0 / Mass);
	tmp = tmp * (float)timeStep;
	system->particles[index].vel = vel + tmp;

	// vel = vel + timestep * Force / Mass

}

void ParticleSimulator::symplectic(FVector pos, FVector vel, int index) 
{

	FVector tmp;

	// velocity 
	tmp = Force * (float)(1.0 / Mass);
	tmp = tmp * (float)timeStep;
	
	system->particles[index].vel = vel + tmp;
	// vel = vel + timestep * Force / Mass

	// position
	tmp = system->particles[index].vel * (float)timeStep;

	system->particles[index].pos = pos + tmp;

	// pos = pos + new-vel * timestep

}

void ParticleSimulator::verlet(FVector pos, FVector vel, int index) 
{
	// acceleration = prev vel - vel * timestep


	// F =ma,  a = F/m
	FVector tmp;
	FVector accel;
	FVector newPos;
	// position = 2 * pos - prev pos + accelration->(F/m) * timestep ^2

	// 2 * pos
	tmp = pos * 2.0f;

	// accelration->(F/m) * timestep ^2
	accel = Force * (float)(1.0 / Mass);
	accel = accel * (float)pow(timeStep, 2);

	// 2 * pos - prev pos
	tmp = tmp - system->particles[index].prevPos;

	// 2 * pos - prev pos + accelration->(F/m) * timestep ^2
	newPos = tmp + accel;

	// velocity 
	// = new pos - prev pos / 2* timestep

	vel = newPos - system->particles[index].prevPos;
	system->particles[index].vel = vel * (float)(1.0 / 2 * timeStep);

	// set pos and prev pos 
	system->particles[index].prevPos = pos;
	system->particles[index].pos = newPos;

}

// calculating spring force sepretly at the start 
// to avoid paricles moving while calculating 
void ParticleSimulator::springsForce() 
{

	FVector force;
	FVector SpringForce;
	FVector DampForce;

	// position
	FVector xi;
	FVector xj;
	// velocity
	FVector vi;
	FVector vj;

	FVector forceDirc;
	FVector tmp;
	FVector disVec;

	// GLM use float persision so must cast to float 
	// TODO change everything to float
	float length;
	float lenWeight;

	for (int i = 0; i < system->numSprings; i++) {
		// SpringForce = -spring.ks() * ( spring.length() - length(xi - xj) ) *  ( xi - xj / length (xi - xj) )		

		// ------- SPRING FORCE---------
		// get positions of xi and xj
		xi = system->particles[system->springs[i].indexA].pos;
		xj = system->particles[system->springs[i].indexB].pos;

		// more stable version is (rest lenght * (xi - xj) / length )- ks
		// 
		// calculate force direction = ( xi - xj / abs( xi - xj ))
		// = xi - xj
		disVec = xi - xj;
		// = length (xi - xj)
		length = disVec.Size();
		// forceDirc = ( xi - xj )/ length(xi,xj)

		forceDirc = disVec * (1.0f / length);

		tmp = forceDirc * (float)system->springs[i].restLen;
		tmp = tmp - disVec;

		SpringForce = tmp * (float)system->springs[i].ks;

		// -------- DAMPER FORCE ---------
		// DampForce = -spring.kd() *  ( ( vi - vj ) dotproduct ( xi - xj / length xi - xj ) ) ( xi - xj / length xi - xj)

		vi = system->particles[system->springs[i].indexA].vel;
		vj = system->particles[system->springs[i].indexB].vel;

		// vector between velocity vectors 
		tmp = vi - vj;

		lenWeight = tmp.Dot(forceDirc);


		DampForce = forceDirc * (lenWeight * ((float)-system->springs[i].kd));

		// -------- ADD FORCES -----------------

		force = SpringForce + DampForce;

		// set spring force
		system->springs[i].ForceAtoB = force;

	}

}

// THESE FUNCTIONS NOT USED IN THIS
/*
void ParticleSimulator::groundForcePenelty() {

	FVector forceNorm;
	FVector SpringForce;
	FVector DampForce;

	// arbatrary point on the ground plain
	FVector point = { 0,0,0 };

	// position
	FVector pos;
	// velocity
	FVector vel;

	// FVector forceDirc;
	// FVector tmp;

	// GLM uses float precission
	// float length;
	// float lenWeight;

	for (int i = 0; i < numGroundSprings; i++) {

		// ------- GROUND SPRING FORCE---------
		// get positions of xi 
		if (groundSprings.empty()) {
			printf("Error: groundSprings is empty!\n");
		}

		//printf("system: %p, particles: %p\n", system, system->particles);
		//printf("groundSprings[%d].indexA: %d\n", i, groundSprings[i]->indexA);

		pos = system->particles[groundSprings[i]->indexA].pos;

		// this is equal to pos
		//VecSubtract(forceDirc, pos, point);

		// -ks * ( pos - point on plane ) dotprod ground normal
		SpringForce = ground * (float)(-groundSprings[i]->ks * pos.Dot(ground));

		// ------- GROUND Dampen FORCE---------
		vel = system->particles[groundSprings[i]->indexA].vel;

		// -kd * (vel dot ground norm) * ground norm
		DampForce = ground * (float)(-groundSprings[i]->kd * vel.Dot(ground));

		// add both terms
		forceNorm = SpringForce * DampForce;

		// TODO use referance for this assignment the varible forceNorm dosnt get out of scope 
		groundSprings[i]->ForceAtoB = forceNorm;
		printf("spring %d force: (%f, %f, %f)\n", i, forceNorm[0], forceNorm[1], forceNorm[2]);

	}



}

bool ParticleSimulator::groundCollision(FVector pos) {

	// FVector v;
	FVector zero = FVector(0, 0, 0);
	// v is vector from ground orgine to particle position 
	// NOTE we can just set position as vector v as pos - 0 = pos
	// VecSubtract(v, pos, zero);

	float dot = pos.Dot(ground);

	// if dot product is negative pos is below the ground 
	if (dot < 0) {
		return true;
	}
	else {
		return false;
	}

}
*/
