// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraModeComponent.h"
#include "CameraMode.h"
#include "CameraSystemCharacter.h"
#include "Algo/Find.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

UCameraModeComponent::UCameraModeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraModeComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = CastChecked<ACameraSystemCharacter>(GetOwner());

	if (Character->IsLocallyControlled())
	{
		Character->OnTagContainerChanged.AddDynamic(this, &UCameraModeComponent::OnAbilityTagChanged);
	}

	PlayerCameraManager = CastChecked<APlayerController>(Character->GetController())->PlayerCameraManager;

	TryUpdateCameraMode();
}

void UCameraModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCameraMode(DeltaTime);
}

void UCameraModeComponent::OnAbilityTagChanged(const FGameplayTag& Tag, bool TagExists)
{
	TryUpdateCameraMode();
}

void UCameraModeComponent::TryUpdateCameraMode()
{
	SetCameraMode(DetermineCameraMode(Character->GetGameplayTags()));
}

void UCameraModeComponent::SetCameraMode(UCameraMode* NewMode)
{
	if (CurrentCameraMode == NewMode)
	{
		return;
	}

	PreviousInterpSpeed = CurrentCameraMode == nullptr ? NewMode->InterpolationSpeed : CurrentCameraMode->InterpolationSpeed;

	CurrentCameraMode = NewMode;
	
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = CurrentCameraMode->bUseControllerDesiredRotation;
	Character->GetCharacterMovement()->bOrientRotationToMovement = !CurrentCameraMode->bUseControllerDesiredRotation;
	TimeSecondsAfterSetNewMode = GetWorld()->GetTimeSeconds();

	OnCameraModeChangedDelegate.Broadcast(NewMode);
}

UCameraMode* UCameraModeComponent::DetermineCameraMode(const FGameplayTagContainer& Tags) const
{
	if (auto foundMode = Algo::FindByPredicate(CameraModes, [&](const auto modeData) {return modeData.CanActivateMode(Tags); }))
	{
		return foundMode->GetCameraMode();
	}

	return nullptr;
}

float UCameraModeComponent::GetInterpSpeed() const
{
	auto timeAfterModeWasChanged = GetWorld()->GetTimeSeconds() - TimeSecondsAfterSetNewMode;
	auto lerpDuration = CurrentCameraMode->InterpolationLerpDuration;
	auto lerpAlpha = FMath::IsNearlyZero(lerpDuration) ? 1.f : FMath::Min(1.f, timeAfterModeWasChanged / lerpDuration);
	return FMath::Lerp(PreviousInterpSpeed, CurrentCameraMode->InterpolationSpeed, lerpAlpha);
}

void UCameraModeComponent::UpdateCameraMode(float DeltaTime)
{
	if (CurrentCameraMode != nullptr)
	{
		UpdateSpringArmLength(DeltaTime);
		UpdateCameraLocation(DeltaTime);
		UpdateFOV(DeltaTime);
	}
}

void UCameraModeComponent::UpdateSpringArmLength(float DeltaTime)
{
	const auto currentLength = Character->GetCameraBoom()->TargetArmLength;

	const auto targetLength = CurrentCameraMode->ArmLength;

	const auto newArmLength = FMath::FInterpTo(currentLength, targetLength, DeltaTime, GetInterpSpeed());

	Character->GetCameraBoom()->TargetArmLength = newArmLength;
}

void UCameraModeComponent::UpdateCameraLocation(float DeltaTime)
{
	const auto currentLocation = Character->GetCameraBoom()->SocketOffset;
	const auto targetLocation = CurrentCameraMode->CameraOffset;
	FVector newLocation = FMath::VInterpTo(currentLocation, targetLocation, DeltaTime, GetInterpSpeed());

	Character->GetCameraBoom()->SocketOffset = newLocation;
}

void UCameraModeComponent::UpdateFOV(float DeltaTime)
{
	const auto currentFov = PlayerCameraManager->GetFOVAngle();
	const auto targetFov = CurrentCameraMode->Fov;
	auto newFov = FMath::FInterpTo(currentFov, targetFov, DeltaTime, GetInterpSpeed());
	
	PlayerCameraManager->SetFOV(newFov);
}

#if WITH_EDITOR
void UCameraModeComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty != nullptr && PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UCameraModeComponent, CameraModes))
	{
		for (auto& mode : CameraModes)
		{
			mode.DebugName = mode.GetCameraMode() != nullptr ? mode.GetCameraMode()->GetClass()->GetName() : TEXT("None");
		}
	}
}
#endif
