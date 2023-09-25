// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ShooterCharacter.h"




// Sets default values
AItem::AItem()
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
	PickupWidget->SetVisibility(false);

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnShpereOverlapBegin);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereOverlapEnd);
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
		}
	}
}



// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

