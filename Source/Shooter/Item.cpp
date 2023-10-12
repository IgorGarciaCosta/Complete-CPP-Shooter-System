// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"




// Sets default values
AItem::AItem():
	ItemName(FString("Base Weapon"))
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject< USkeletalMeshComponent>("ItemMesh");
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject< UBoxComponent>("CollisionBox");
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	AreaSphere = CreateDefaultSubobject< USphereComponent>("AreaSphere");
	AreaSphere->SetupAttachment(GetRootComponent());

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>("PickupWidget");
	PickupWidget->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget) {
		PickupWidget->SetVisibility(false);

	}
	SetStarsActive();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnShpereOverlapBegin);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereOverlapEnd);

	SetItemPrpoerties(ItemState);
}

void AItem::OnShpereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) {
		AShooterCharacter* Char = Cast< AShooterCharacter>(OtherActor);
		if (Char) {
			Char->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) {
		AShooterCharacter* Char = Cast< AShooterCharacter>(OtherActor);
		if (Char) {
			Char->IncrementOverlappedItemCount(-1);
			PickupWidget->SetVisibility(false);
		}
	}
}



// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);
}

void AItem::SetStarsActive()
{
	for (int i = 0;i < 5;i++) {
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
		case EItemRarity::EIR_Damage:
			ActiveStars[0] = true;
			break;
		case EItemRarity::EIR_Common:
			ActiveStars[0] = true;
			ActiveStars[1] = true;
			break;
		case EItemRarity::EIR_Uncommon:
			ActiveStars[0] = true;
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			break;
		case EItemRarity::EIR_Rare:
			ActiveStars[0] = true;
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			break;
		case EItemRarity::EIR_Legendary:
			ActiveStars[0] = true;
			ActiveStars[1] = true;
			ActiveStars[2] = true;
			ActiveStars[3] = true;
			ActiveStars[4] = true;
			break;
		case EItemRarity::EIR_MAX:
			break;
		default:
			break;
	}
}

void AItem::SetItemPrpoerties(EItemState curState)
{
	switch (curState)
	{
	case EItemState::EIS_Pickup:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Pickedup:
		break;
	case EItemState::EIS_Equipped:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetEnableGravity(false);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}
}

void AItem::FinishInterping()
{
	bInterping = false;
	if (CharRef) {
		CharRef->GetPuckupItem(this);
	}
	//set scale back to normal
	SetActorScale3D(FVector(1.f));

}


//make item interp logic
void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (CharRef && ItemZCurve) {

		//elapsed time since ItemInterpTimerHanlde started
		const float elapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimerHanlde);
		
		//get float val corresonding to elapsed time
		const float CurveValue = ItemZCurve->GetFloatValue(elapsedTime);

		//get item's inital loc, when the curve started
		FVector ItemLoc = ItemInterpStartLoc;

		//get loc in front of the camera
		const FVector CameraInterpLocation = CharRef->GetCameraInterpLoc();

		//vector from item to camera
		const FVector ItemToCamera = FVector(0.f, 0.f, (CameraInterpLocation - ItemLoc).Z);

		//scale factor to multiply with curve value
		const float DeltaZ = ItemToCamera.Size();

		const FVector curLoc = GetActorLocation();
		const float InterpXVal = FMath::FInterpTo(curLoc.X, CameraInterpLocation.X, DeltaTime, 30.0f);
		const float InterpYVal = FMath::FInterpTo(curLoc.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);

		//set x and y of itemLoc to interpolated values
		ItemLoc.X = InterpXVal;
		ItemLoc.Y = InterpYVal;

		ItemLoc.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLoc, true, nullptr, ETeleportType::TeleportPhysics);

		//cam rot
		const FRotator CameraRotation = CharRef->GetFollowCamera()->GetComponentRotation();
		//cam rot + initial yaw offset
		FRotator ItemRot{ 0.f, CameraRotation.Yaw + InterpInitilaYawOffset, 0.f };
		SetActorRotation(ItemRot, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve) {
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(elapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

void AItem::SetItemState(EItemState state)
{
	ItemState = state; 
	SetItemPrpoerties(state);
}

void AItem::StartItemCurve(AShooterCharacter* Char)
{
	CharRef = Char;

	ItemInterpStartLoc = GetActorLocation();

	bInterping = true;

	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().SetTimer(ItemInterpTimerHanlde, this, &AItem::FinishInterping, zCurveTime);


	//get initial yaw values of camera and item
	const float CameraRotationYaw = CharRef->GetFollowCamera()->GetComponentRotation().Yaw;
	const float ItemRotYaw = GetActorRotation().Yaw;

	InterpInitilaYawOffset =  ItemRotYaw - CameraRotationYaw;
}

