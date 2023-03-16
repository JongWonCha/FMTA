// Fill out your copyright notice in the Description page of Project Settings.

#include "ClientSocket.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformAffinity.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "MyPlayerController.h"

ClientSocket::ClientSocket() :StopTaskCounter(0)
{

	// ID 전부 -1로 초기화
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CharactersInfo.WorldCharacterInfo[i].SessionId = -1;
	}

}

ClientSocket::~ClientSocket()
{
	delete Thread;
	Thread = nullptr;

	closesocket(ServerSocket);
	WSACleanup();
}

bool ClientSocket::InitSocket()
{
	WSADATA wsaData;
	int retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (retval != 0) {
		return false;
	}
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ServerSocket == INVALID_SOCKET)
	{
		return false;
	}
	return true;
}

bool ClientSocket::Connect(const char * pszIP, int nPort)
{
	SOCKADDR_IN stServerAddr;

	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(nPort);
	stServerAddr.sin_addr.s_addr = inet_addr(pszIP);

	int retval = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (retval == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

// 캐릭터 등록
void ClientSocket::EnrollCharacterInfo(CharacterInfo & info)
{
	CharacterInfo loc;
	loc.packet = EEnroll;
	loc.SessionId = info.SessionId;
	loc.x = info.x;
	loc.y = info.y;
	loc.z = info.z;
	loc.yaw = info.yaw;
	loc.pitch = info.pitch;
	loc.roll = info.roll;
	loc.IsAlive = info.IsAlive;

	// 캐릭터 정보 전송
	int retval = send(ServerSocket, (CHAR*)&loc, sizeof(CharacterInfo), 0);
	if (retval == -1)
	{
		//UE_LOG(LogClass, Log, TEXT("EnrollCharacterInfo Send fail!"));
		return;
	}
}

// 내 캐릭터 정보 송신
void ClientSocket::SendMyCharacterInfo(CharacterInfo& ActorCharacterInfo)
{
	CharacterInfo loc;
	loc.packet = ESend;
	loc.SessionId = ActorCharacterInfo.SessionId;
	loc.x = ActorCharacterInfo.x;
	loc.y = ActorCharacterInfo.y;
	loc.z = ActorCharacterInfo.z;
	loc.yaw = ActorCharacterInfo.yaw;
	loc.pitch = ActorCharacterInfo.pitch;
	loc.roll = ActorCharacterInfo.roll;
	loc.IsAlive = ActorCharacterInfo.IsAlive;
	//UE_LOG(LogClass, Log, TEXT("Send!"));
	int retval = send(ServerSocket, (CHAR*)&loc, sizeof(CharacterInfo), 0);
	if (retval == -1) {
		UE_LOG(LogClass, Log, TEXT("SendMyCharacterInfo Send fail!"));
		return;
	}
}

// 캐릭터 정보 수신
WorldCharactersInfo * ClientSocket::RecvCharacterInfo(CharacterInfo& Recvp)
{
	//UE_LOG(LogClass, Log, TEXT("RecvCharacterInfo!"));

	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].SessionId = Recvp.SessionId;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].x = Recvp.x;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].y = Recvp.y;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].z = Recvp.z;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].yaw = Recvp.yaw;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].pitch = Recvp.pitch;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].roll = Recvp.roll;
	CharactersInfo.WorldCharacterInfo[Recvp.SessionId].IsAlive = Recvp.IsAlive;

	//UE_LOG(LogClass, Log, TEXT("RecvCharacterInfo Complete!"));

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (CharactersInfo.WorldCharacterInfo[i].IsAlive == true)
		{
			UE_LOG(LogClass, Log, TEXT("[%d] x : [%f], y :[%f], z : [%f]"), i, CharactersInfo.WorldCharacterInfo[i].x, CharactersInfo.WorldCharacterInfo[i].y, CharactersInfo.WorldCharacterInfo[i].z);
		}
	}

	return &CharactersInfo;
}

void ClientSocket::SetPlayerController(AMyPlayerController* pPlayerController)
{
	// 플레이어 컨트롤러 세팅
	if (pPlayerController)
	{
		PlayerController = pPlayerController;
	}
}

void ClientSocket::CloseSocket()
{
	closesocket(ServerSocket);
	WSACleanup();
}

bool ClientSocket::Init()
{
	return true;
}

uint32 ClientSocket::Run()
{
	FPlatformProcess::Sleep(0.03);
	// 서버로부터 데이터 수신
	while (StopTaskCounter.GetValue() == 0 && PlayerController != nullptr)
	{
		CharacterInfo *Recvp;
		int nRecvLen = recv(ServerSocket, (CHAR*)&recvBuffer, MAX_BUFFER, 0);
		if (nRecvLen > 0)
		{
			//UE_LOG(LogClass, Log, TEXT("Recv Success!"));
			Recvp = reinterpret_cast<CharacterInfo *>(&recvBuffer);
			//UE_LOG(LogClass, Log, TEXT(" X : [%f], Y : [%f], Z : [%f]"), Recvp->x, Recvp->y, Recvp->z);

			switch (Recvp->packet)
			{
			case ERecv:
			{
				//UE_LOG(LogClass, Log, TEXT("Recv Success!"));
				UE_LOG(LogClass, Log, TEXT("Recvp.SessionID : %d"), Recvp->SessionId);
				(*PlayerController).RecvWorldInfo(*RecvCharacterInfo(*Recvp));
			}
			break;
			default:
			{

			}
			break;
			}
		}
	}

	return 0;
}

void ClientSocket::Stop()
{
	// thread safety 변수를 조작해 while loop 가 돌지 못하게 함
	StopTaskCounter.Increment();
}

void ClientSocket::Exit()
{
}

bool ClientSocket::StartListen()
{
	// 스레드 시작
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void ClientSocket::StopListen()
{
	// 스레드 종료
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();
	delete Thread;
	Thread = nullptr;
	StopTaskCounter.Reset();
}
