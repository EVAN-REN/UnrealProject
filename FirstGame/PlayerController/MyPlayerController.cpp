// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "FirstGame/HUD/MyHUD.h"
#include "FirstGame/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "FirstGame/Character/MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "FirstGame/GameMode/MyGameMode.h"

void AMyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerController, MatchState);
}

void AMyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->HealthBar &&
		MyHUD->CharacterOverlay->HealthText;
	if (bHUDValid) {

		const float HealthPercent = Health / MaxHealth;
		MyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDMaxHealth = MaxHealth;
		HUDHealth = Health;
	}
}

void AMyPlayerController::SetHUDScore(float Score)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		MyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AMyPlayerController::SetHUDDefeats(int32 Defeats)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid) {
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		MyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AMyPlayerController::SetHUDAmmo(int32 Ammo)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->AmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MyHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AMyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid) {
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		MyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AMyPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;

	bool bHUDValid = MyHUD &&
		MyHUD->CharacterOverlay &&
		MyHUD->CharacterOverlay->MatchCountDown;
	if (bHUDValid) {
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - 60 * Minutes;

		FString CountDownTimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MyHUD->CharacterOverlay->MatchCountDown->SetText(FText::FromString(CountDownTimeText));
	}
}

void AMyPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(NewPawn);
	if (MyCharacter)
	{
		SetHUDHealth(MyCharacter->GetHealth(), MyCharacter->GetMaxHealth());
	}


}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);

	PollInit();
}

float AMyPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMyPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress) {
		MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;
		if (MyHUD) {
			MyHUD->AddCharacterOverlay();
		}
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MyHUD = Cast<AMyHUD>(GetHUD());
}

void AMyPlayerController::PollInit()
{
	if (bInitializeCharacterOverlay) {
		if (CharacterOverlay == nullptr) {
			if (MyHUD && MyHUD->CharacterOverlay) {
				CharacterOverlay = MyHUD->CharacterOverlay;
				if (CharacterOverlay) {
					SetHUDHealth(HUDHealth, HUDMaxHealth);
					SetHUDDefeats(HUDDefeats);
					SetHUDScore(HUDScore);
				}
			}
		}
	}
}

void AMyPlayerController::SetHUDTime()
{
	uint32 SecondLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountDownInt != SecondLeft) {
		SetHUDMatchCountDown(MatchTime - GetServerTime());
	}
	CountDownInt = SecondLeft;
}

void AMyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AMyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress) {
		MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(GetHUD()) : MyHUD;
		if (MyHUD) {
			MyHUD->AddCharacterOverlay();
		}
	}
}

void AMyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5 * RoundTripTime);
	
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
