// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "CameraSystemCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS(config=Game)
class ACameraSystemCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACameraSystemCharacter();

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void AddTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable)
	void RemoveTag(const FGameplayTag& Tag);

	const FGameplayTagContainer& GetGameplayTags() const;

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void EnableSprint();
	void DisableSprint();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	DECLARE_EVENT_TwoParams(ACameraSystemCharacter, FOnTagContainerChanged, const FGameplayTag& /*ChangedTag*/, bool /*bExist*/);
	FOnTagContainerChanged OnTagContainerChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	USceneComponent* SpringArmPivot;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer TagContainer;
};

