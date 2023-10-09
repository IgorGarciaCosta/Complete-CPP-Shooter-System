#pragma once


UENUM(BlueprintType)
enum class EAmmoType :uint8 {
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AssountRifle UMETA(DisplayName = "AssountRifle"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};