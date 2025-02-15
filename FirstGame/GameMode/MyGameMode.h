// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTGAME_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMyGameMode();

	virtual void Tick(float DeltaTime) override;

	void PlayerEliminated(class AMyCharacter* ElimmedCharacter, class AMyPlayerController* VictimController, AMyPlayerController* AttackController);
	void RequestRespawn(AMyCharacter* ElimmedCharacter, AMyPlayerController* ElimmedController);\

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountDownTime = 0.f;
};
