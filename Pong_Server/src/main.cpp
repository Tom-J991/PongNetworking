#include <iostream>

#include <BCNet/IBCNetServer.h>
#include <BCNet/BCNetPacket.h>
#include <BCNet/BCNetUtil.h>

#include <raylib.h>
#include <raymath.h>

#include "shared.h"

static BCNet::IBCNetServer *g_server;

constexpr unsigned int defaultClientWidth = 800;
constexpr unsigned int defaultClientHeight = 600;

const float paddleWidth = (12.0f / defaultClientWidth); // Remap 0-1 for resolution independency
const float paddleHeight = (96.0f / defaultClientHeight);
const float paddleVSpeed = (256.0f / defaultClientHeight);

const float ballWidth = (10.0f / defaultClientWidth);
const float ballHeight = (10.0f / defaultClientHeight);
const float ballHSpeed = (256.0f / defaultClientWidth);
const float ballVSpeed = (256.0f / defaultClientHeight);

struct BallInfo
{
	float xPosition = 0.5f;
	float yPosition = 0.5f;
	float angle = 0.0f;
	int xVelocity = -1;
	int yVelocity = -1;
};

struct PlayerInfo
{
	float yPosition = 0.5f;
	bool rightSide = false;

	bool movingUp = false;
	bool movingDown = false;

	unsigned int score = 0;
};

class Game
{
public:
	void Run()
	{
		Init();
		m_gameRunning = true;
		while (m_gameRunning)
		{
			Update();
			Draw();

			m_gameRunning = g_server->IsRunning();
		}
		Shutdown();
	}

private:
	void Init()
	{

	}

	void Shutdown()
	{

	}

	void Update(double deltaTime = 0.0f)
	{

	}

	void Draw()
	{

	}

private:
	bool m_gameRunning = false;

};

void OnConnected(const BCNet::ClientInfo &clientInfo)
{

}

void PacketReceived(const BCNet::ClientInfo &clientInfo, const BCNet::Packet packet)
{
	BCNet::PacketStreamReader reader(packet);
	int packetID;
	reader >> packetID;

	switch (packetID)
	{
		default:
			break;
	}
}

int main()
{
	Game *game = new Game();

	g_server = BCNet::InitServer();

	g_server->SetConnectedCallback(OnConnected);
	g_server->SetPacketReceivedCallback(PacketReceived);

	g_server->SetMaxClients(2);

	g_server->Start();

	game->Run();

	g_server->Stop();

	if (g_server)
		delete g_server;
	g_server = nullptr;

	if (game)
		delete game;
	game = nullptr;

	return 0;
}
