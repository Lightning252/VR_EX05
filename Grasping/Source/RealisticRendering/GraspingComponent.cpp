// Fill out your copyright notice in the Description page of Project Settings.

#include "GraspingComponent.h"


// Sets default values for this component's properties
UGraspingComponent::UGraspingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGraspingComponent::BeginPlay()
{
	Super::BeginPlay();

	UInputComponent* Input = GetOwner()->InputComponent;
	if (!Input)
	{
		UE_LOG(LogTemp, Error, TEXT("No Input Component! Check the auto process player option!"));
		return;
	}

	Input->BindAxis("GraspRight", this, &UGraspingComponent::GraspRight);
	Input->BindAxis("GraspLeft", this, &UGraspingComponent::GraspLeft);


	for (UActorComponent* Component : GetOwner()->GetComponents()) 
	{
		//If we found a motion controller component then set it
		if (UMotionControllerComponent* MotionControllerComponent = Cast<UMotionControllerComponent>(Component))
		{
			bool bIsRight = true;
			//Looking for the left and right controller
			if (MotionControllerComponent->MotionSource == FName("Left")) bIsRight = false;


			TArray<USceneComponent*> Children;
			MotionControllerComponent->GetChildrenComponents(false, Children);

			//Get the mesh for each motion controller
			for (USceneComponent* CurrentChild : Children)
			{
				if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(CurrentChild))
				{
					if (bIsRight)
					{
						Right = MotionControllerComponent;
						RightMesh = Mesh;
					}
					else
					{
						Left = MotionControllerComponent;
						LeftMesh = Mesh;
					}
				}
			}
		}
		
	}
	//Setup the right mesh
	if (RightMesh)
	{
		RightMesh->SetCollisionProfileName("OverlapAllDynamic");
		RightMesh->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RightMesh->SetEnableGravity(false);
		RightMesh->SetGenerateOverlapEvents(true);
		RightMesh->OnComponentBeginOverlap.AddDynamic(this, &UGraspingComponent::RightOverlapBeginEvent);
		RightMesh->OnComponentEndOverlap.AddDynamic(this, &UGraspingComponent::RightOverlapEndEvent);
	}

	//Setup the left mesh
	if (LeftMesh)
	{
		LeftMesh->SetCollisionProfileName("OverlapAllDynamic");
		LeftMesh->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		LeftMesh->SetEnableGravity(false);
		LeftMesh->SetGenerateOverlapEvents(true);
		LeftMesh->OnComponentBeginOverlap.AddDynamic(this, &UGraspingComponent::LeftOverlapBeginEvent);
		LeftMesh->OnComponentEndOverlap.AddDynamic(this, &UGraspingComponent::LeftOverlapEndEvent);
	}
	// ...
	
}


// Called every frame
void UGraspingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGraspingComponent::GraspRight(float Alpha)
{
	if (Alpha > 0.5 && !CurrentRightGrasp)
	{

		AStaticMeshActor* CurrentNearestActor = CalculateGraspActor(RightMesh, RightOverlapActors);
		if (CurrentNearestActor)
		{
			//Disable physics and gravity
			CurrentRightGrasp = CurrentNearestActor;
			CurrentRightGrasp->GetStaticMeshComponent()->SetEnableGravity(false);
			CurrentRightGrasp->GetStaticMeshComponent()->SetSimulatePhysics(false);
			CurrentRightGrasp->AttachToComponent(Right, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else if (Alpha < 0.5 && CurrentRightGrasp)
	{
		CurrentRightGrasp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		//Enable physics and gravity
		CurrentRightGrasp->GetStaticMeshComponent()->SetEnableGravity(true);
		CurrentRightGrasp->GetStaticMeshComponent()->SetSimulatePhysics(true);
		CurrentRightGrasp = nullptr;
	}
}

void UGraspingComponent::GraspLeft(float Alpha)
{
	if (Alpha > 0.5 && !CurrentLeftGrasp)
	{
		AStaticMeshActor* CurrentNearestActor = CalculateGraspActor(LeftMesh, LeftOverlapActors);

		if (CurrentNearestActor)
		{
			//Disable physics and gravity
			CurrentLeftGrasp = CurrentNearestActor;
			CurrentLeftGrasp->GetStaticMeshComponent()->SetEnableGravity(false);
			CurrentLeftGrasp->GetStaticMeshComponent()->SetSimulatePhysics(false);
			CurrentLeftGrasp->AttachToComponent(Left, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else if (Alpha < 0.5 && CurrentLeftGrasp)
	{
		CurrentLeftGrasp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		//Enable physics and gravity
		CurrentLeftGrasp->GetStaticMeshComponent()->SetEnableGravity(true);
		CurrentLeftGrasp->GetStaticMeshComponent()->SetSimulatePhysics(true);
		CurrentLeftGrasp = nullptr;
	}
}

AStaticMeshActor * UGraspingComponent::CalculateGraspActor(USkeletalMeshComponent * Hand, TArray<AStaticMeshActor*> OverlapActors)
{
	float CurrentNearestDist = 99999999.0f;
	AStaticMeshActor* CurrentNearestActor = nullptr;

	//Setup ray trace
	const FName TraceTag("TraceTag");
	FCollisionQueryParams TraceParams(FName(TEXT("Trace")), true, GetOwner());
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.TraceTag = TraceTag;

	//Ray trace start position
	FVector StartTrace = Hand->GetComponentLocation();


	for (AStaticMeshActor* CurrentActor : OverlapActors)
	{
		//Calculate the distance
		float Dist = FVector::Dist(Hand->GetComponentLocation(), CurrentActor->GetActorLocation());
		if (CurrentNearestDist > Dist)
		{
			FVector EndTrace = CurrentActor->GetActorLocation();
			FHitResult HitObject;

			//Start ray trace
			GetOwner()->GetWorld()->LineTraceSingleByChannel(HitObject, StartTrace, EndTrace, ECollisionChannel::ECC_Visibility, TraceParams);
			if (HitObject.IsValidBlockingHit() && !(Cast<AStaticMeshActor>(HitObject.GetActor()) == CurrentActor)) continue;
			CurrentNearestActor = CurrentActor;
			CurrentNearestDist = Dist;
		}
	}
	return CurrentNearestActor;
}

void UGraspingComponent::LeftOverlapBeginEvent(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//Checks for a static mesh actor we only want to grasp static mesh actors
	AStaticMeshActor* CurrentActor = Cast<AStaticMeshActor>(OtherActor);
	if (CurrentActor) LeftOverlapActors.Add(CurrentActor);
}

void UGraspingComponent::LeftOverlapEndEvent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AStaticMeshActor* CurrentActor = Cast<AStaticMeshActor>(OtherActor);
	if (CurrentActor) LeftOverlapActors.Remove(CurrentActor);
}

void UGraspingComponent::RightOverlapBeginEvent(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//Checks for a static mesh actor we only want to grasp static mesh actors
	AStaticMeshActor* CurrentActor = Cast<AStaticMeshActor>(OtherActor);
	if (CurrentActor) RightOverlapActors.Add(CurrentActor);
}

void UGraspingComponent::RightOverlapEndEvent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	AStaticMeshActor* CurrentActor = Cast<AStaticMeshActor>(OtherActor);
	if (CurrentActor) RightOverlapActors.Remove(CurrentActor);
}

