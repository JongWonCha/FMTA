// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include "Runtime/Core/Public/HAL/Runnable.h"

#define	MAX_BUFFER		2048
#define SERVER_PORT		8000
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100

using namespace std;

class AMyPlayerController;

// 패킷 종류
enum EPacket
{
	EEnroll,
	ESend,
	ERecv,
	EGameOver,
	EWon
};

// 캐릭터 정보
struct CharacterInfo
{
	EPacket packet;
	int SessionId;
	float x;
	float y;
	float z;
	float yaw;
	float pitch;
	float roll;
	bool IsAlive;
};

// 전체 캐릭터 구조체
struct WorldCharactersInfo {
	CharacterInfo WorldCharacterInfo[MAX_CLIENTS];
};


/**
 *
 */
class FMTA_API ClientSocket : public FRunnable
{
public:
	ClientSocket();
	~ClientSocket();

	bool InitSocket();
	bool Connect(const char * pszIP, int nPort);

	void EnrollCharacterInfo(CharacterInfo& info);
	void SendMyCharacterInfo(CharacterInfo& ActorCharacterInfo);

	void SetPlayerController(AMyPlayerController* pPlayerController);
	void CloseSocket();

	// FRunnable Thread members	
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	// FRunnable override 함수
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// 스레드 시작 및 종료
	bool StartListen();
	void StopListen();

	static ClientSocket* GetSingleton()
	{
		static ClientSocket ins;
		return &ins;
	}
private:
	SOCKET ServerSocket;
	char 	recvBuffer[MAX_BUFFER];
	WorldCharactersInfo CharactersInfo;
	AMyPlayerController* PlayerController;
	WorldCharactersInfo * RecvCharacterInfo(CharacterInfo& Recvp);
};
