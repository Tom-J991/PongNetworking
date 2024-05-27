#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include <raylib.h>
#include <raymath.h>

#include <BCNet/IBCNetClient.h>
#include <BCNet/BCNetPacket.h>

constexpr unsigned int defaultClientWidth = 800; // Basis resolution.
constexpr unsigned int defaultClientHeight = 600;

constexpr float paddleWidth = (12.0f / defaultClientWidth); // Remap 0-1 for resolution independency
constexpr float paddleHeight = (96.0f / defaultClientHeight);
constexpr float paddleXOffset = (64.0f / defaultClientWidth);
constexpr float paddleVSpeed = (256.0f / defaultClientHeight);

constexpr float ballWidth = (10.0f / defaultClientWidth);
constexpr float ballHeight = (10.0f / defaultClientHeight);
constexpr float ballHSpeed = (256.0f / defaultClientWidth);
constexpr float ballVSpeed = (256.0f / defaultClientHeight);

constexpr unsigned int clientWidth = defaultClientWidth; // Actual resolution.
constexpr unsigned int clientHeight = defaultClientHeight;

struct GameState
{
	bool gameStarted = false;
};

struct BallInfo
{
	float xPosition = 0.5f;
	float yPosition = 0.5f;
	float xVelocity = -1.0f;
	float yVelocity = -1.0f;
};

struct PlayerInfo
{
	bool connected = false;

	float yPosition = 0.5f;
	bool rightSide = false;

	bool movingUp = false;
	bool movingDown = false;

	unsigned int score = 0;

	bool ready = false;
};

enum class eSounds
{
	BOUNDS = 0,
	BOUNCE,
	SCORE,
	SOUNDS_MAX
};

class Game
{
public:
	Game();
	~Game();

	void Run();

private:
	void Init();
	void Shutdown();

	void Update(double deltaTime = 0.0f);
	void Draw();

	void PlaySFX(eSounds sound);

	void OnConnected();
	void OnDisconnected();
	void PacketReceived(const BCNet::Packet packet);

private:
	bool m_running = false;

	BCNet::IBCNetClient *m_netClient;

	std::vector<Sound> m_loadedSounds;

	GameState m_gameState;

	PlayerInfo m_player; // Me.
	PlayerInfo m_peerPlayer; // Other client. Only two max players so only need just one reference to a peer.
	unsigned int m_playerCount = 0; // Not the count of how many is connected to the server, just how many the client knows about. Peer clients should always be >1.

	BallInfo m_ball;

};
