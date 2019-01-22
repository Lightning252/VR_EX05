// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionControllerComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "GraspingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REALISTICRENDERING_API UGraspingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGraspingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	UMotionControllerComponent* Left;
	UMotionControllerComponent* Right;

	USkeletalMeshComponent* RightMesh;
	USkeletalMeshComponent* LeftMesh;

	AStaticMeshActor* CurrentRightGrasp;
	AStaticMeshActor* CurrentLeftGrasp;

	TArray<AStaticMeshActor*> LeftOverlapActors;
	TArray<AStaticMeshActor*> RightOverlapActors;

	AStaticMeshActor* CalculateGraspActor(USkeletalMeshComponent* Hand, TArray<AStaticMeshActor*> OverlapActors);

	UFUNCTION()
	void GraspRight(float Alpha);
	UFUNCTION()
	void GraspLeft(float Alpha);

	UFUNCTION()
	void LeftOverlapBeginEvent(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void LeftOverlapEndEvent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void RightOverlapBeginEvent(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void RightOverlapEndEvent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex);

		
	
};
