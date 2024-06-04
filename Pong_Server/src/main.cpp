#include <iostream>
#include <string>
#include <unordered_map>

#include <BCNet/IBCNetServer.h>
#include <BCNet/BCNetPacket.h>
#include <BCNet/BCNetUtil.h>

#include <raylib.h>
#include <raymath.h>

#include "shared.h"

#include "TextObject.h"

static BCNet::IBCNetServer *g_server;

// Game Constants.
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

// Game Objects
struct GameState
{
	int playersReady = 0;
	bool gameStarted = false;
};

struct BallInfo
{
	float xPosition = 0.5f;
	float yPosition = 0.5f;
	float xVelocity = -1.0f;
	float yVelocity = -1.0f;
	float currentHSpeed = ballHSpeed;
	float currentVSpeed = ballVSpeed;
};

struct PlayerInfo
{
	float yPosition = 0.5f;
	bool rightSide = false;

	bool movingUp = false;
	bool movingDown = false;

	unsigned int score = 0;

	bool ready = false;
};

// Utility functions.
float CalculateBallAngle()
{
	// Recycled from old code.
	float angle = (float)GetRandomValue(0, (int)(360 * DEG2RAD));
	if (angle == 0 || angle == (90 * DEG2RAD) || angle == (180 * DEG2RAD) || angle == (270 * DEG2RAD) || angle == (360 * DEG2RAD)) // Recalculates the angle if goes straight along an axis.
		return CalculateBallAngle(); // TODO: Uh oh, recursive function.
	return angle;
}

bool AABB(BallInfo &ball, PlayerInfo &paddle) // Ball-Paddle collision detection.
{
	float paddleXPos = paddleXOffset;
	if (paddle.rightSide)
		paddleXPos = 1.0f - paddleXOffset;

	bool collX = ball.xPosition - (ballWidth/2.0f) <= paddleXPos + (paddleWidth/2.0f) && // Ball intersects paddle on X axis.
		ball.xPosition + (ballWidth/2.0f) >= paddleXPos - (paddleWidth/2.0f);
	bool collY = ball.yPosition - (ballHeight/2.0f) <= paddle.yPosition + (paddleHeight/2.0f) && // Ball intersects paddle on Y axis.
		ball.yPosition + (ballHeight/2.0f) >= paddle.yPosition - (paddleHeight/2.0f);

	return collX && collY;
}

// --------------------- Main Class
class Game
{
public:
	void Run()
	{
		InitWindow(clientWidth, clientHeight, "Pong Server");
		SetTargetFPS(60);
		SetExitKey(NULL);

		Init();

		double lastTime = 1.0 / 60.0; // Delta time.

		// Game loop.
		m_gameRunning = true;
		while (m_gameRunning)
		{
			double currentTime = GetTime(); // Delta time.
			double deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			Update(deltaTime);

			BeginDrawing();
			Draw();
			EndDrawing();

			m_gameRunning = !WindowShouldClose();
		}

		Shutdown();

		CloseWindow();
	}

public:
	void OnDisconnected(const BCNet::ClientInfo &clientInfo)
	{
		// Get rid of disconnected player.
		m_players.erase(clientInfo.id);

		// Tell the client a player has disconnected.
		BCNet::Packet packet;
		packet.Allocate(1024);
		BCNet::PacketStreamWriter writer(packet);
		writer << PongPackets::PONG_PLAYER_DISCONNECTED << clientInfo.id;
		g_server->SendPacketToAllClients(writer.GetPacket(), clientInfo.id);
		packet.Release();

		// Reset the game.
		Init();
	}

	void OnConnected(const BCNet::ClientInfo &clientInfo)
	{
		// Setup player with defaults.
		m_players[clientInfo.id].movingUp = false;
		m_players[clientInfo.id].movingDown = false;
		m_players[clientInfo.id].yPosition = 0.5f;
		m_players[clientInfo.id].score = 0;
		m_players[clientInfo.id].rightSide = g_server->GetConnectedCount() > 1;
		m_players[clientInfo.id].ready = false;

		// Tell any connected clients that another player has connected.
		BCNet::Packet packet;
		packet.Allocate(1024);
		BCNet::PacketStreamWriter writer(packet);
		writer << PongPackets::PONG_PLAYER_CONNECTED << clientInfo.id << m_players[clientInfo.id].rightSide;
		g_server->SendPacketToAllClients(writer.GetPacket());
		packet.Release();
	}

	void PacketReceived(const BCNet::ClientInfo &clientInfo, const BCNet::Packet packet)
	{
		BCNet::PacketStreamReader reader(packet);
		int packetID;
		reader >> packetID;

		switch (packetID)
		{
			case (int)PongPackets::PONG_PLAYER_REQUEST_PEERS:
			{
				// A client has requested info from any connected peers.
				for (auto [id, info] : m_players)
				{
					if (id == clientInfo.id) // Makes sure not to give them their own info.
						continue;
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_PLAYER_REQUEST_PEERS
						<< info.rightSide << info.score << info.ready << info.yPosition
						<< info.movingDown << info.movingUp;
					g_server->SendPacketToClient(clientInfo.id, writer.GetPacket());
					packet.Release();
				}
			} break;
			case (int)PongPackets::PONG_PLAYER_MOVING_UP:
			{
				// A client has told us that they're moving up, or not.
				bool moving;
				reader >> moving;
				m_players[clientInfo.id].movingUp = moving;

				// Tell other clients that they're moving up, or not.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_PLAYER_MOVING_UP << moving;
				g_server->SendPacketToAllClients(writer.GetPacket(), clientInfo.id);
				packet.Release();
			} break;
			case (int)PongPackets::PONG_PLAYER_MOVING_DOWN:
			{
				// A client has told us that they're moving down, or not.
				bool moving;
				reader >> moving;
				m_players[clientInfo.id].movingDown = moving;

				// Tell other clients that they're moving down, or not.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_PLAYER_MOVING_DOWN << moving;
				g_server->SendPacketToAllClients(writer.GetPacket(), clientInfo.id);
				packet.Release();
			} break;
			case (int)PongPackets::PONG_PLAYER_READY:
			{
				// A client has told us that they're ready.
				bool ready;
				reader >> ready;
				m_players[clientInfo.id].ready = ready;
				m_gameState.playersReady += ready ? 1 : -1;

				// Tell other clients that they readied up.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_PLAYER_READY << ready;
				g_server->SendPacketToAllClients(writer.GetPacket(), clientInfo.id);
				packet.Release();
			} break;
			default:
				break;
		}
	}

private:
	void Init()
	{
		// Setup game defaults.
		m_gameState.gameStarted = false;
		m_gameState.playersReady = 0;

		m_ball.xPosition = 0.5f;
		m_ball.yPosition = 0.5f;
		m_ball.xVelocity = -1.0f;
		m_ball.yVelocity = -1.0f;
		m_ball.currentHSpeed = ballHSpeed;
		m_ball.currentVSpeed = ballVSpeed;

		float angle = CalculateBallAngle();
		m_ball.xVelocity = ballHSpeed * cosf(angle);
		m_ball.yVelocity = ballVSpeed * sinf(angle);

		m_timer = 0.0f;
	}

	void Shutdown()
	{
		std::cout << "Shutting down" << std::endl;
	}

	void Update(double deltaTime = 0.0f)
	{
		//std::cout << "delta: " << deltaTime << std::endl;

		m_textPool.Animate(deltaTime); // Update text object pool.

		// Do gameplay state.
		if (m_gameState.gameStarted == true)
		{
			// Check ball collision.
			for (auto &[id, info] : m_players)
			{
				bool collided = false;
				if (AABB(m_ball, info)) // TODO: Better collision detection.
					collided = true;

				// Paddle hit ball.
				if (collided)
				{
					// Calculate bounce angle
					float intersectY = info.yPosition - m_ball.yPosition; // How far from the middle of the paddle did the ball hit?
					float normalizedIntersect = intersectY / (paddleHeight / 2.0f); // -1 <-> 1
					float bounceAngle = normalizedIntersect * (45 * DEG2RAD);
					m_ball.currentHSpeed = ballHSpeed + abs(normalizedIntersect) * ballHSpeed;
					m_ball.currentVSpeed = ballVSpeed + abs(normalizedIntersect) * ballVSpeed;

					// Set balls new velocity based on new angle and which direction they've been hit from.
					if (info.rightSide == false)
					{
						m_ball.xVelocity = m_ball.currentHSpeed * cosf(bounceAngle);
						m_ball.yVelocity = m_ball.currentVSpeed * -sinf(bounceAngle);
					}
					else
					{
						m_ball.xVelocity = m_ball.currentHSpeed * -cosf(bounceAngle);
						m_ball.yVelocity = m_ball.currentVSpeed * sinf(bounceAngle);
					}

					// Update clients on the ball's new velocity.
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_BALL_VELOCITY << m_ball.xVelocity << m_ball.yVelocity;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();

					// Tell the clients that a player's paddle has hit the ball.
					packet.Allocate(1024);
					writer = BCNet::PacketStreamWriter(packet);
					writer << PongPackets::PONG_PLAYER_HIT;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();
				}

				// Check if ball goes out of bounds, i.e. goal.
				if (m_ball.xPosition > 1.0f && info.rightSide == false) // Update score for the left player.
				{
					info.score += 1;

					// Tell the clients that a player's score has been updated.
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_PLAYER_SCORE << info.rightSide << info.score;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();
				}
				else if (m_ball.xPosition < 0.0f && info.rightSide == true) // Update score for the right player.
				{
					info.score += 1;

					// Tell the clients that a player's score has been updated.
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_PLAYER_SCORE << info.rightSide << info.score;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();
				}
			}

			// Check if ball hits vertical bounds.
			if (m_ball.yPosition < 0.0f || m_ball.yPosition > 1.0f) // Flip y direction if hit ceiling or floor.
			{
				m_ball.yVelocity *= -1; // Bounce.

				// Update clients on the ball's new velocity.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_BALL_VELOCITY << m_ball.xVelocity << m_ball.yVelocity;
				g_server->SendPacketToAllClients(writer.GetPacket());
				packet.Release();

				// Tell the clients that the ball has bounced.
				packet.Allocate(1024);
				writer = BCNet::PacketStreamWriter(packet);
				writer << PongPackets::PONG_BALL_BOUNCE;
				g_server->SendPacketToAllClients(writer.GetPacket());
				packet.Release();
			}

			// Reset the ball when out of bounds.
			if (m_ball.xPosition > 1.0f || m_ball.xPosition < 0.0f)
			{
				m_ball.xPosition = 0.5f;
				m_ball.yPosition = 0.5f;
				m_ball.xVelocity = -1.0f;
				m_ball.yVelocity = -1.0f;
				m_ball.currentHSpeed = ballHSpeed;
				m_ball.currentVSpeed = ballVSpeed;

				float angle = CalculateBallAngle();
				m_ball.xVelocity = ballHSpeed * cosf(angle);
				m_ball.yVelocity = ballVSpeed * sinf(angle);

				// Tell the clients that the ball has been reset.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_BALL_RESET;
				g_server->SendPacketToAllClients(writer.GetPacket());
				packet.Release();

				// Update the clients on the ball's new velocity.
				packet.Allocate(1024);
				writer = BCNet::PacketStreamWriter(packet);
				writer << PongPackets::PONG_BALL_VELOCITY << m_ball.xVelocity << m_ball.yVelocity;
				g_server->SendPacketToAllClients(writer.GetPacket());
				packet.Release();
			}

			// Update ball.
			m_ball.xPosition += m_ball.xVelocity * (float)deltaTime;
			m_ball.yPosition += m_ball.yVelocity * (float)deltaTime;
		}
		else // Do lobby state.
		{
			if (m_gameState.playersReady >= 2) // Both players are ready.
			{
				// Start count down.
				m_timer += (float)deltaTime;
				static unsigned int lastCountDown = 4;
				unsigned int countDown = (unsigned int)(std::ceilf(3.0f - m_timer));

				if (countDown != lastCountDown) // So the stuff inside isn't called each frame,
				{
					lastCountDown = countDown; // only when the countdown has changed.

					// Count down!
					std::string countDownText = std::to_string(countDown);
					if (countDown <= 0)
						countDownText = "GO!";

					int textWidth = MeasureText(countDownText.c_str(), 48);
					float textXPosition = (clientWidth / 2.0f) - (textWidth / 2.0f);
					float textYPosition = (clientHeight / 2.0f) - 24.0f;
					m_textPool.Init(countDownText, textXPosition, textYPosition, 1.0f, 48, BLUE);

					// Alert the clients on the count down.
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_PLAYER_COUNTDOWN << countDownText;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();

					g_server->Log(countDownText);
				}

				if (m_timer >= 3.0f) // The count down has ended!
				{
					// Start game!
					m_gameState.gameStarted = true;

					// Tell the clients that the game has commenced!
					BCNet::Packet packet;
					packet.Allocate(1024);
					BCNet::PacketStreamWriter writer(packet);
					writer << PongPackets::PONG_GAME_STARTED;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();

					// Update the clients on the ball's new velocity.
					packet.Allocate(1024);
					writer = BCNet::PacketStreamWriter(packet);
					writer << PongPackets::PONG_BALL_VELOCITY << m_ball.xVelocity << m_ball.yVelocity;
					g_server->SendPacketToAllClients(writer.GetPacket());
					packet.Release();
				}
			}
			else
			{
				m_timer = 0.0f; // Reset timer.
			}
		}

		// Update players.
		for (auto &[id, info] : m_players)
		{
			int input = (int)info.movingUp - (int)info.movingDown;
			float move = input * paddleVSpeed * (float)deltaTime;

			info.yPosition -= move; // Move the player.

			// Clamp to bounds.
			if (info.yPosition - (paddleHeight / 2.0f) < 0.0f)
				info.yPosition = (paddleHeight / 2.0f);
			if (info.yPosition + (paddleHeight / 2.0f) > 1.0f)
				info.yPosition = 1.0f - (paddleHeight / 2.0f);
		}
	}

	void Draw()
	{
		ClearBackground(BLACK);

		// Draw Centre-line.
		for (int i = 0; i < clientHeight; i += 24)
		{
			DrawRectangle((int)((clientWidth / 2.0f) - 4), i - 4, 8, 12, WHITE);
		}

		// Draw Ball.
		int ballXPos = (int)(m_ball.xPosition * clientWidth);
		int ballYPos = (int)(m_ball.yPosition * clientHeight);
		int ballW = (int)(ballWidth * clientWidth);
		int ballH = (int)(ballHeight * clientHeight);
		
		int ballCenterX = ballXPos - (int)(ballW / 2.0f);
		int ballCenterY = ballYPos - (int)(ballH / 2.0f);

		DrawRectangle(ballCenterX, ballCenterY, ballW, ballH, WHITE);

		// Draw Players.
		for (auto &[id, info] : m_players)
		{
			int playerXPos = info.rightSide ? (int)((1.0f - paddleXOffset) * clientWidth) : (int)(paddleXOffset * clientWidth);
			int playerYPos = (int)(info.yPosition * clientHeight);
			int playerWidth = (int)(paddleWidth * clientWidth);
			int playerHeight = (int)(paddleHeight * clientHeight);

			int playerCenterX = playerXPos - (int)(playerWidth / 2.0f);
			int playerCenterY = playerYPos - (int)(playerHeight / 2.0f);

			Color color = info.rightSide ? RED : BLUE;

			DrawRectangle(playerCenterX, playerCenterY, playerWidth, playerHeight, color);

			// Draw Score.
			constexpr int screenHalf = clientWidth / 2;
			constexpr int leftHalf = screenHalf / 2;
			constexpr int rightHalf = screenHalf + leftHalf;
			int scoreXPos = info.rightSide ? rightHalf : leftHalf;

			std::string scoreText = std::to_string(info.score);
			DrawText(scoreText.c_str(), scoreXPos, (int)(clientHeight / 5.0f), 48, color);

			// Draw whether they're ready or not.
			if (m_gameState.gameStarted == false)
			{
				std::string readyText = info.ready ? "Is Ready" : "Not Ready";

				int textXPos = playerXPos + 12;
				if (info.rightSide)
				{
					int textWidth = MeasureText(readyText.c_str(), 24);
					textXPos = playerXPos - textWidth - 12;
				}

				int textYPos = playerYPos - playerHeight;
				if (textYPos < 0)
					textYPos = 0;
				if (textYPos > clientHeight - 24)
					textYPos = clientHeight - 24;

				DrawText(readyText.c_str(), textXPos, textYPos, 24, color);
			}
		}

		m_textPool.Draw(); // Draw the text object pool.
	}

private:
	bool m_gameRunning = false;

	GameState m_gameState;

	std::unordered_map<uint32, PlayerInfo> m_players;
	BallInfo m_ball;

	float m_timer = 0.0f;
	TextObjectPool m_textPool;

public:
	Game() { }
	~Game() { }

	static Game *Instance() { return s_instance; }

private:
	Game(const Game &game) = delete;

	static Game *s_instance; // Singleton.

};
Game *Game::s_instance = new Game(); // Make sure the singleton actually exists.

// ------------------------- Entry point.
int main()
{
	Game *game = Game::Instance();

	g_server = BCNet::InitServer(); // Get networking server's interface.

	// Setup callbacks.
	g_server->SetConnectedCallback([&](const BCNet::ClientInfo &clientInfo) { game->OnConnected(clientInfo); });
	g_server->SetDisconnectedCallback([&](const BCNet::ClientInfo &clientInfo) { game->OnDisconnected(clientInfo); });
	g_server->SetPacketReceivedCallback([&](const BCNet::ClientInfo &clientInfo, const BCNet::Packet packet) { game->PacketReceived(clientInfo, packet); });

	g_server->SetMaxClients(2);

	g_server->Start(); // Start the server.

	game->Run(); // Start the game.

	g_server->Stop(); // This doesn't get called immediately after because both the game class and the server have loops.

	// Clean up.
	if (g_server)
		delete g_server;
	g_server = nullptr;

	if (game)
		delete game;
	game = nullptr;

	return 0;
}
