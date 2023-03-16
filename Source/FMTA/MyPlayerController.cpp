// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include <string>

AMyPlayerController::AMyPlayerController()
{
	ControllerId = FMath::RandRange(0, 100);

	Socket = ClientSocket::GetSingleton();
	Socket->InitSocket();
	bIsConnected = Socket->Connect("127.0.0.1", 8000);
	if (bIsConnected)
	{
		Socket->SetPlayerController(this);
	}
}

AMyPlayerController::~AMyPlayerController()
{

}

int AMyPlayerController::GetControllerId()
{
	return ControllerId;
}

void AMyPlayerController::Tick(float DeltaSeconds)
{
	//UE_LOG(LogClass, Log, TEXT("Tick!"));

	Super::Tick(DeltaSeconds);

	// ������ ���� ���� Ȯ��
	if (!bIsConnected)
	{
		return;
	}
	// �÷��̾� ���� �۽�
	if (!SendPlayerInfo()) { return; }
	// ���� ����ȭ
	//if (!UpdatePlayer()) { return; }

}

void AMyPlayerController::BeginPlay()
{
	auto Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	// ĳ���� �ʱ� ����
	auto MyCharacterInfo = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();

	CharacterInfo Character;
	UE_LOG(LogClass, Log, TEXT("ControllerId : %d"), ControllerId);
	Character.SessionId = ControllerId;
	Character.x = MyCharacterInfo.X;
	Character.y = MyCharacterInfo.Y;
	Character.z = MyCharacterInfo.Z;
	Character.yaw = MyRotation.Yaw;
	Character.pitch = MyRotation.Pitch;
	Character.roll = MyRotation.Roll;

	Character.IsAlive = true;

	Socket->EnrollCharacterInfo(Character);
	Socket->StartListen();

}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Socket->CloseSocket();
	Socket->StopListen();
}

// ID ã�� warining
AActor * AMyPlayerController::FindActorBySessionId(TArray<AActor*> ActorArray, const int & SessionId)
{
	for (const auto& Actor : ActorArray)
	{
		if (std::stoi(*Actor->GetName()) == SessionId)
			return Actor;
	}
	return nullptr;
}

bool AMyPlayerController::SendPlayerInfo()
{
	auto Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		//UE_LOG(LogClass, Log, TEXT("Player false!"));
		return false;
	}
	auto MyCharacterInfo = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();

	CharacterInfo Character;

	Character.SessionId = ControllerId;
	Character.x = MyCharacterInfo.X;
	Character.y = MyCharacterInfo.Y;
	Character.z = MyCharacterInfo.Z;
	Character.yaw = MyRotation.Yaw;
	Character.pitch = MyRotation.Pitch;
	Character.roll = MyRotation.Roll;
	Character.IsAlive = true;
	// IsAlive?

	//UE_LOG(LogClass, Log, TEXT(" X : [%f], Y : [%f], Z : [%f]"), Character.yaw, Character.pitch, Character.roll);
	Socket->SendMyCharacterInfo(Character);

	return true;
}

void AMyPlayerController::RecvWorldInfo(WorldCharactersInfo ci)
{
	//UE_LOG(LogClass, Log, TEXT("RecvWorldInfo!"));
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (ci.WorldCharacterInfo[i].SessionId != -1)
		{
			OtherPlayer.WorldCharacterInfo[i].SessionId = ci.WorldCharacterInfo[i].SessionId;
			OtherPlayer.WorldCharacterInfo[i].x = ci.WorldCharacterInfo[i].x;
			OtherPlayer.WorldCharacterInfo[i].y = ci.WorldCharacterInfo[i].y;
			OtherPlayer.WorldCharacterInfo[i].z = ci.WorldCharacterInfo[i].z;
			OtherPlayer.WorldCharacterInfo[i].yaw = ci.WorldCharacterInfo[i].yaw;
			OtherPlayer.WorldCharacterInfo[i].pitch = ci.WorldCharacterInfo[i].pitch;
			OtherPlayer.WorldCharacterInfo[i].roll = ci.WorldCharacterInfo[i].roll;
			OtherPlayer.WorldCharacterInfo[i].IsAlive = ci.WorldCharacterInfo[i].IsAlive;
		}
	}
}

void AMyPlayerController::UpdatePlayerInfo(const CharacterInfo & info)
{
	auto Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}
	UWorld* const world = GetWorld();
	if (!world)
	{
		return;
	}
	// �÷��̾� ��� �� ���� �ı�
	if (!info.IsAlive)
	{
		//UE_LOG(LogClass, Log, TEXT("Player Die"));
		Player->Destroy();
	}
}
/*
// �ٸ� �÷��̾�
bool AMyPlayerController::UpdatePlayer()
{
	//UE_LOG(LogClass, Log, TEXT("UpdatePlayer()"));
	UWorld* const world = GetWorld();
	if (world == nullptr)
	{
		return false;
	}

	// ���� �� Character ���� ����
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyCharacter::StaticClass(), SpawnedCharacters);


	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		int CharacterID = OtherPlayer.WorldCharacterInfo[i].SessionId;

		// ID�� '��'�� ��� ����
		if (CharacterID == ControllerId)
		{
			UpdatePlayerInfo(OtherPlayer.WorldCharacterInfo[i]);
			continue;
		}

		if (CharacterID != -1)
		{
			auto Actor = FindActorBySessionId(SpawnedCharacters, CharacterID);
			if (Actor == nullptr &&OtherPlayer.WorldCharacterInfo[i].IsAlive == true)
			{
				FVector SpawnCharacterInfo;
				SpawnCharacterInfo.X = OtherPlayer.WorldCharacterInfo[i].x;
				SpawnCharacterInfo.Y = OtherPlayer.WorldCharacterInfo[i].y;
				SpawnCharacterInfo.Z = OtherPlayer.WorldCharacterInfo[i].z;

				FRotator SpawnRotation;
				SpawnRotation.Yaw = OtherPlayer.WorldCharacterInfo[i].yaw;
				SpawnRotation.Pitch = OtherPlayer.WorldCharacterInfo[i].pitch;
				SpawnRotation.Roll = OtherPlayer.WorldCharacterInfo[i].roll;

				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = Instigator;
				SpawnParams.Name = FName(*FString(to_string(OtherPlayer.WorldCharacterInfo[i].SessionId).c_str()));

				AMyCharacter* const SpawnCharacter = world->SpawnActor<AMyCharacter>(WhoToSpawn, SpawnCharacterInfo, SpawnRotation, SpawnParams);
			}
			else if(Actor != nullptr && OtherPlayer.WorldCharacterInfo[i].IsAlive == true)
			{
				AMyCharacter* OtherCharacter = Cast<AMyCharacter>(Actor);
				if (OtherCharacter)
				{
					FVector CharacterCharacterInfo;
					CharacterCharacterInfo.X = OtherPlayer.WorldCharacterInfo[CharacterID].x;
					CharacterCharacterInfo.Y = OtherPlayer.WorldCharacterInfo[CharacterID].y;
					CharacterCharacterInfo.Z = OtherPlayer.WorldCharacterInfo[CharacterID].z;

					FRotator CharacterRotation;
					CharacterRotation.Yaw = OtherPlayer.WorldCharacterInfo[CharacterID].yaw;
					CharacterRotation.Pitch = OtherPlayer.WorldCharacterInfo[CharacterID].pitch;
					CharacterRotation.Roll = OtherPlayer.WorldCharacterInfo[CharacterID].roll;

					OtherCharacter->SetActorLocation(CharacterCharacterInfo);
					OtherCharacter->SetActorRotation(CharacterRotation);
				}

			}
			else if (Actor != nullptr && OtherPlayer.WorldCharacterInfo[i].IsAlive == false)
			{
				//UE_LOG(LogClass, Log, TEXT("Destroy Actor"));
				Actor->Destroy();
			}
		}

	}

	return true;
}
*/

