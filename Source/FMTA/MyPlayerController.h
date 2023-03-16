// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ClientSocket.h"
#include "MyPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class FMTA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();
	~AMyPlayerController();

	// id
	UFUNCTION(BlueprintPure, Category = "Properties")
	int GetControllerId();

	// ���� ���

	// �ٸ� ĳ����
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> WhoToSpawn;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// id ��Ī
	AActor* FindActorBySessionId(TArray<AActor*> ActorArray, const int& SeesionId);

	// ���� ���� ����
	void RecvWorldInfo(WorldCharactersInfo ci);

private:
	// ���� ���ӿ� ����
	ClientSocket * Socket;
	// ����Ȯ��
	bool bIsConnected;
	// ĳ���� ���� ���̵�
	int ControllerId;
	// �ٸ� ĳ���� ����
	WorldCharactersInfo OtherPlayer;
	// �÷��̾� ��ġ �۽�
	bool SendPlayerInfo();
	// ���� ����ȭ
	//bool UpdatePlayer();
	// �÷��̾� ����ȭ
	void UpdatePlayerInfo(const CharacterInfo & info);
	
};