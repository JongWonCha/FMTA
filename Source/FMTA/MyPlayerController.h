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

	// 스폰 대상

	// 다른 캐릭터
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> WhoToSpawn;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// id 매칭
	AActor* FindActorBySessionId(TArray<AActor*> ActorArray, const int& SeesionId);

	// 월드 정보 수신
	void RecvWorldInfo(WorldCharactersInfo ci);

private:
	// 서버 접속용 소켓
	ClientSocket * Socket;
	// 연결확인
	bool bIsConnected;
	// 캐릭터 세션 아이디
	int ControllerId;
	// 다른 캐릭터 정보
	WorldCharactersInfo OtherPlayer;
	// 플레이어 위치 송신
	bool SendPlayerInfo();
	// 월드 동기화
	//bool UpdatePlayer();
	// 플레이어 동기화
	void UpdatePlayerInfo(const CharacterInfo & info);
	
};