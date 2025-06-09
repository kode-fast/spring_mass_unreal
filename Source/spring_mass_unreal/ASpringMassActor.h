#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "KismetProceduralMeshLibrary.h"

#include "particle_system.h"
#include "particle_simulator.h"

#include "ASpringMassActor.generated.h"

// TODOS 
// - seperate cloth and swing into seperate actor classes that inharet from SpringMassActor
// - change cloth overlap to a trigger function
// - check degen triangles on procedural cloth mesh
// - play with decressed weights for cloth sim / rework cloth sim units
// - standerdize varible names .-.
UCLASS(Blueprintable)
class SPRING_MASS_UNREAL_API ASpringMassActor : public AActor
{
	GENERATED_BODY()

public:	
	// class constructor
	ASpringMassActor();


protected:
	// called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	

	UFUNCTION(BlueprintCallable)
	void EnableClothCollision();

	UFUNCTION(BlueprintCallable)
	void SetClothSim(int width = 10, int hight = 20, int panel_size = 10, double struct_spring_kd = 10.0, double struct_spring_ks = 100.0,
		double shear_spring_kd = 10.0, double shear_spring_ks = 10.0, double flex_spring_kd = 10.0, double flex_spring_ks = 10.0, int pin_spacing = 1);
	// called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void CreateSpring(int ParticleNumA, int ParticleNumB, double restLen, double Kd, double Ks);
	UFUNCTION(BlueprintCallable)
	void EditStructSprings(double restLen, double Kd, double Ks);
	UFUNCTION(BlueprintCallable)
	void EditShearSprings(double restLen, double Kd, double Ks);
	UFUNCTION(BlueprintCallable)
	void EditFlexSprings(double restLen, double Kd, double Ks);
	UFUNCTION(BlueprintCallable)
	void ClothTick(double deltaTime, FVector outsideForce);

	UFUNCTION(BlueprintCallable)
	void OnHold(FVector charPos, FVector hitPos, double stringLen);

	UFUNCTION(BlueprintCallable)
	FVector OnRelease();
	UFUNCTION(BlueprintCallable)
	FVector SwingTick(double DeltaTime, FVector playerVel);

	UPROPERTY()
	UProceduralMeshComponent* Mesh;

	ParticleSystem particles;
	ParticleSimulator simulation;

	int num_struct_springs;
	int num_shear_springs;
	int num_flex_springs;

	UPROPERTY(BlueprintReadOnly)
	bool swing = false;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FVector> DummyTangent;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;
};
