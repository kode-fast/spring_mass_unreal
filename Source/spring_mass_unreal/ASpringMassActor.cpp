// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "ASpringMassActor.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"


// sets default values
ASpringMassActor::ASpringMassActor()
{
	// THESE NEED TO BE IN THE CONSTRUCTOR
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	 
	// create procedural mesh
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ClothMesh"));
	// attach to root component (the actor)
	RootComponent = Mesh;

}
void ASpringMassActor::EnableClothCollision()
{
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetGenerateOverlapEvents(true);

	// TODO create trigger function for cloth overlap
	// set overlap trigger function 

}
void ASpringMassActor::OnConstruction(const FTransform& Transform) 
{
	Super::OnConstruction(Transform);
}

// called when the game starts or when spawned
void ASpringMassActor::BeginPlay()
{
	// calls parent actor class function
	Super::BeginPlay();


}

void ASpringMassActor::SetClothSim(int width, int hight, int panel_size, double struct_spring_kd, double struct_spring_ks,
			double shear_spring_kd, double shear_spring_ks, double flex_spring_kd, double flex_spring_ks, int pin_spacing) 
{

	// add vertices to mesh
	for (int j = 0; j < hight; j++) {


		for (int i = 0; i < width; i++) {

			Vertices.Add(FVector(FMath::FRandRange(0.0, 10.0), i * panel_size, -j * panel_size));
			UE_LOG(LogTemp, Display, TEXT("ADDING VERTEX AT %s"), *FVector(0, i * panel_size, j * panel_size).ToString());
		}

	}

	// add triangles
	// for each Vertex

	int count = 0;
	for (int i = 0; i < Vertices.Num(); i++) {
		UE_LOG(LogTemp, Display, TEXT("LOOP %d"), i);

		if (count + 2 > width) {
			count = 0;
			continue;
		}

		UE_LOG(LogTemp, Display, TEXT("MAKING TRIANGLE %s"), *FVector(i + width, i + width + 1, i).ToString());
		UE_LOG(LogTemp, Display, TEXT("MAKING TRIANGLE %s"), *FVector(i, i + width + 1, i + 1).ToString());

		Triangles.Append({ i + width, i + width + 1, i });
		Triangles.Append({ i, i + width + 1, i + 1 });
		count++;
	}


	Normals.Init(FVector(0, 0, 1), Vertices.Num());

	UVs.Init(FVector2D(0, 0), Vertices.Num());

	Tangents.Init(FProcMeshTangent(1, 0, 0), Vertices.Num());

	VertexColors.Init(FColor::White, Vertices.Num());

	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// set material 
	UMaterialInterface* TwoSidedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Game/Materials/M_TwoSided.M_TwoSided'"));
	if (TwoSidedMaterial)
	{
		Mesh->SetMaterial(0, TwoSidedMaterial);
	}

	// TODO expose mass as editble varible too
	// positions number of partiels mass - mass need to be big - units are all weird 
	particles = ParticleSystem(Vertices, Vertices.Num(), 100.0);


	// fix the top row with spacing 
	for (int i = 0; i < width; i = i + pin_spacing) {
		particles.particles[i].fixed = true;
	}

	// add springs to vertices setup

	// kd is how much it resist the existing velocity 
	// ks is how much it pushes on a particle giving it more velocity

	// kd is spring dampaning - higher number is stiffer spring (more energy loss to the spring) less occilations , lower number is spriggyer more occilations
	// ks is spring stiffness - higher is more snappy to the rest legnth, lower is more springy 
	// index , index, rest length kd, ks

	num_struct_springs = 0;
	num_shear_springs = 0;
	num_flex_springs = 0;


	// UE_LOG(LogTemp, Display, TEXT("SET STRUCT SPRINGS"));
	// add structural springs [i,j] -> [i+1, j] ; [ i,j] -> [i, j+1]
	int vertex = 0;
	for (int row = 0; row < hight; row++) {

		for (int col = 0; col < width; col++) {
			// UE_LOG(LogTemp, Display, TEXT("col, row: %s"), *FVector(col, row, 0).ToString());

			// x directio spring - if last particle in row then dont attach spring to next particle
			if (col < width - 1) {
				// connect currect particle (row*col) with next particle in row (row * col) + 1
				particles.setSpring(vertex, vertex + 1, 10.0, struct_spring_kd, struct_spring_ks);
				// DrawDebugLine(GetWorld(), Vertices[vertex], Vertices[vertex + 1],FColor::Red,true,0.0f,0,2.0f);           
				num_struct_springs++;
			}

			// y spring - if last particle in collumn dont attach spring to next row 
			if (row < hight - 1) {
				// connect current particle with particle under it (col * row) + width
				particles.setSpring(vertex, vertex + width, 10.0, struct_spring_kd, struct_spring_ks);
				// DrawDebugLine(GetWorld(),  Vertices[vertex],Vertices[vertex + width],FColor::Red, true, 0.0f, 0, 2.0f);    
				num_struct_springs++;

			}
			vertex++;
		}

	}

	// UE_LOG(LogTemp, Display, TEXT("SET SHEAR SPRINGS"));

	// for spring x is right y is up

	// add Shear springs [ i , j ] -> [i+1, j+1] ; [i, j] -> [i+1, j]
	vertex = 0;
	for (int row = 0; row < hight; row++) {

		for (int col = 0; col < width; col++) {

			// x spring - if last particle in row OR column dont attch - connecting current particle to particle 1 below and 1 over to it
			if (col < width - 1 && row < hight - 1) {
				particles.setSpring(vertex, vertex + width + 1, 10.0, shear_spring_kd, shear_spring_ks);
				// y spring
				// also attetch current partecl + 1 colunm to current particle + 1 row
				particles.setSpring(vertex + 1, vertex + width, 10.0, shear_spring_kd, shear_spring_ks);
				//DrawDebugLine(GetWorld(), Vertices[vertex], Vertices[vertex + width + 1], FColor::Yellow, true, 0.0f, 0, 2.0f);
				//DrawDebugLine(GetWorld(), Vertices[vertex + 1], Vertices[vertex + width], FColor::Yellow, true, 0.0f, 0, 2.0f);

				num_shear_springs++;
				num_shear_springs++;
			}
			vertex++;
		}

	}

	// UE_LOG(LogTemp, Display, TEXT("SET FLEX SPRINGS"));

	// add Flexion (bend) springs [i, j] -> [i+2, j]; [i,j] -> [i, j+2]
	vertex = 0;
	for (int row = 0; row < hight; row++) {

		for (int col = 0; col < width; col++) {

			// x spring - if particle is second last or last in row dont attatch to next particle in x direction
			if (col < width - 2) {
				// attatch current particle with particle 2 spots over
				particles.setSpring(vertex, vertex + 2, 10.0, flex_spring_kd, flex_spring_ks);
				// DrawDebugLine(GetWorld(), Vertices[vertex], Vertices[vertex + 2], FColor::Blue, true, 0.0f, 0, 2.0f);

				num_flex_springs++;

			}

			// y spring - if particle is second or last in collum dont attatch to next particle in y direction
			if (row < hight - 2) {
				particles.setSpring(vertex, vertex + (2 * width), 10.0, flex_spring_kd, flex_spring_ks);
				// DrawDebugLine(GetWorld(), Vertices[vertex], Vertices[vertex + (2 * width)], FColor::Blue, true, 0.0f, 0, 2.0f);

				num_flex_springs++;


			}
			vertex++;
		}

	}


	simulation = ParticleSimulator(&particles);

}


void ASpringMassActor::CreateSpring(int ParticleNumA, int ParticleNumB, double restLen, double Kd, double Ks) {
	particles.setSpring(ParticleNumA, ParticleNumB, restLen, Kd, Kd);
		
}


void ASpringMassActor::EditStructSprings(double restLen, double kd, double ks) 
{

	for (int i = 0; i < num_struct_springs;i++) {
		simulation.springs[i]->restLen = restLen;
		simulation.springs[i]->kd = kd;
		simulation.springs[i]->restLen = ks;

	}
	
}

void ASpringMassActor::EditShearSprings(double restLen, double kd, double ks) 
{
	UE_LOG(LogTemp, Display, TEXT("NUM SPRINGS %d"), sizeof(simulation.springs) / sizeof(Spring));
	for (int i = num_struct_springs-1; i < num_struct_springs+num_shear_springs; i++) {

		simulation.springs[i]->restLen = restLen;
		simulation.springs[i]->kd = kd;
		simulation.springs[i]->restLen = ks;

	}

}
void ASpringMassActor::EditFlexSprings(double restLen, double kd, double ks) 
{
	for (int i = num_struct_springs + num_shear_springs - 1; i < num_struct_springs + num_shear_springs + num_flex_springs; i++) {
		simulation.springs[i]->restLen = restLen;
		simulation.springs[i]->kd = kd;
		simulation.springs[i]->restLen = ks;

	}
}
// called every frame
void ASpringMassActor::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);
	

}

void ASpringMassActor::ClothTick(double DeltaTime, FVector outsideForce) 
{


	// update particle locations

	TArray<AActor*> OverlappingActors;
	Mesh->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
	// ABSALUTLY ABISMAL WAY TO DO THIS - SEEING IF WORKS

	//UE_LOG(LogTemp, Display, TEXT("IN TICK"));

	// IN FUTERUE USE A HIT CALL FUNCTION - CAN FLAG WITH TRUE OR FALSE 
	// (maby use on component hit?) 
	if (OverlappingActors.Num() > 0) 
	{
		//UE_LOG(LogTemp, Display, TEXT("CHAR VEL %s"), *OverlappingActors[0]->GetVelocity().ToString());

		simulation.step(0.0, DeltaTime, 50*OverlappingActors[0]->GetVelocity());
	} 
	else 
	{
		simulation.step(0.0, DeltaTime, outsideForce);

	}


	// update vertex locations 
	// TODO if working on in future gut out all the parts of simulation and replace with unreal data structures 
	TArray<FVector> new_vert_pos = simulation.update();


	Mesh->UpdateMeshSection(0, new_vert_pos, Normals, UVs, VertexColors, Tangents);
}


// spring swing logic 
/*
void ASpringMassActor::SwingSetup() {

	

}
*/

void ASpringMassActor::OnHold(FVector charPos, FVector hitPos, double stringLen) 
{

	swing = true;
	// get hit vector and char vector
	TArray<FVector> particle = { charPos, hitPos };
	particles = ParticleSystem(particle,2,5.0);
	particles.particles[1].fixed = true;
	particles.setSpring(0, 1, 1.0, 5.0, 50.0);
	simulation = ParticleSimulator(&particles);

}

void ASpringMassActor::OnRelease() {

	swing = false;
	
}

FVector ASpringMassActor::SwingTick(double DeltaTime, FVector playerVel) 
{

	// TODO this if statment is obasalte now with the blueprint setup

	// add player velocity 
	if (swing) {

		simulation.step(0.0, DeltaTime, playerVel);
		return particles.particles[0].pos;

	}

	return FVector(0.0, 0.0, 0.0);

}