// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "FirstGame/Character/MyCharacter.h"
#include "FirstGame/PlayerController/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "FirstGame/PlayerState/MyPlayerState.h"

AMyGameMode::AMyGameMode()
{
	bDelayedStart = true;
}

void AMyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) {
		CountDownTime = WarmupTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if (CountDownTime <= 0.f) {
			StartMatch();
		}
	}
}

void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It);
		if (PlayerController) {
			PlayerController->OnMatchStateSet(MatchState);
		}

	}
}

void AMyGameMode::PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* VictimController, AMyPlayerController* AttackController)
{
	AMyPlayerState* AttackPlayerState = AttackController ? Cast<AMyPlayerState>(AttackController->PlayerState) : nullptr;
	AMyPlayerState* VictimPlayerState = VictimController ? Cast<AMyPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackPlayerState && AttackPlayerState != VictimPlayerState) {
		AttackPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter) {
		ElimmedCharacter->Elim();
	}
}

void AMyGameMode::RequestRespawn(AMyCharacter* ElimmedCharacter, AMyPlayerController* ElimmedController)
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController) {
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}

}


