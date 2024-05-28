#include "Game.h"
#include "shared.h"

Game::Game()
{ 
	m_loadedSounds.resize((int)eSounds::SOUNDS_MAX);

	m_netClient = BCNet::InitClient();
}
Game::~Game()
{ 
	if (m_netClient)
		delete m_netClient;
	m_netClient = nullptr;
}

void Game::Init()
{
	m_netClient->SetConnectedCallback([this]() { OnConnected(); });
	m_netClient->SetDisconnectedCallback([this]() { OnDisconnected(); });
	m_netClient->SetPacketReceivedCallback([this](const BCNet::Packet packet) { PacketReceived(packet); });

	m_netClient->Start();

	// Load Assets.
	m_loadedSounds[(int)eSounds::BOUNCE] = LoadSound("./assets/sfx/pong/ball_bounce.ogg");
	m_loadedSounds[(int)eSounds::HIT] = LoadSound("./assets/sfx/pong/ball_hit.ogg");
	m_loadedSounds[(int)eSounds::SCORE] = LoadSound("./assets/sfx/pong/score.ogg");

	// Init Game.
	m_gameState.gameStarted = false;

	m_ball.xPosition = 0.5f;
	m_ball.yPosition = 0.5f;
	m_ball.xVelocity = -1.0f;
	m_ball.yVelocity = -1.0f;
}

void Game::Shutdown()
{
	m_netClient->Stop();

	// Unload Assets
	for (int i = 0; i < (int)eSounds::SOUNDS_MAX; i++)
		if (IsSoundReady(m_loadedSounds[i]))
			UnloadSound(m_loadedSounds[i]);
	m_loadedSounds.clear();
	m_loadedSounds.resize((int)eSounds::SOUNDS_MAX);
}

void Game::Update(double deltaTime)
{
	m_textPool.Animate(deltaTime);

	if (m_gameState.gameStarted == true) // Do gameplay state.
	{
		// Update ball.
		m_ball.xPosition += m_ball.xVelocity * (float)deltaTime;
		m_ball.yPosition += m_ball.yVelocity * (float)deltaTime;
	}
	else // Do lobby state.
	{
		if (m_player.connected)
		{
			if (IsKeyReleased(KEY_SPACE)) // Ready up.
			{
				m_player.ready = !m_player.ready;

				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_PLAYER_READY << m_player.ready;
				m_netClient->SendPacketToServer(writer.GetPacket());
				packet.Release();
			}
		}
	}

	// Get client player input.
	if (m_player.connected)
	{
		if (IsKeyPressed(KEY_UP))
		{
			m_player.movingUp = true;

			// TODO: Clean this up.
			BCNet::Packet packet;
			packet.Allocate(1024);
			BCNet::PacketStreamWriter writer(packet);
			writer << PongPackets::PONG_PLAYER_MOVING_UP << m_player.movingUp;
			m_netClient->SendPacketToServer(writer.GetPacket());
			packet.Release();
		}
		else if (IsKeyReleased(KEY_UP))
		{
			m_player.movingUp = false;

			BCNet::Packet packet;
			packet.Allocate(1024);
			BCNet::PacketStreamWriter writer(packet);
			writer << PongPackets::PONG_PLAYER_MOVING_UP << m_player.movingUp;
			m_netClient->SendPacketToServer(writer.GetPacket());
			packet.Release();
		}

		if (IsKeyPressed(KEY_DOWN))
		{
			m_player.movingDown = true;

			BCNet::Packet packet;
			packet.Allocate(1024);
			BCNet::PacketStreamWriter writer(packet);
			writer << PongPackets::PONG_PLAYER_MOVING_DOWN << m_player.movingDown;
			m_netClient->SendPacketToServer(writer.GetPacket());
			packet.Release();
		}
		else if (IsKeyReleased(KEY_DOWN))
		{
			m_player.movingDown = false;

			BCNet::Packet packet;
			packet.Allocate(1024);
			BCNet::PacketStreamWriter writer(packet);
			writer << PongPackets::PONG_PLAYER_MOVING_DOWN << m_player.movingDown;
			m_netClient->SendPacketToServer(writer.GetPacket());
			packet.Release();
		}
	}

	// Update peer player.
	if (m_peerPlayer.connected == true)
	{
		int input = (int)m_peerPlayer.movingUp - (int)m_peerPlayer.movingDown;
		float move = input * paddleVSpeed * (float)deltaTime;

		m_peerPlayer.yPosition -= move;

		// Clamp to bounds.
		if (m_peerPlayer.yPosition - (paddleHeight / 2.0f) < 0.0f)
			m_peerPlayer.yPosition = (paddleHeight / 2.0f);
		if (m_peerPlayer.yPosition + (paddleHeight / 2.0f) > 1.0f)
			m_peerPlayer.yPosition = 1.0f - (paddleHeight / 2.0f);
	}

	// Update client player.
	if (m_player.connected == true)
	{
		int input = (int)m_player.movingUp - (int)m_player.movingDown;
		float move = input * paddleVSpeed * (float)deltaTime;

		m_player.yPosition -= move;

		// Clamp to bounds.
		if (m_player.yPosition - (paddleHeight / 2.0f) < 0.0f)
			m_player.yPosition = (paddleHeight / 2.0f);
		if (m_player.yPosition + (paddleHeight / 2.0f) > 1.0f)
			m_player.yPosition = 1.0f - (paddleHeight / 2.0f);
	}

	// Do Text Input for Connection Menu.
	if (m_player.connected == false && 
		(m_ipEntered == false || 
		m_portEntered == false))
	{
		int key = GetCharPressed();
		while (key > 0)
		{
			if ((key >= 32) && (key <= 125) && (m_connectionInputCount < MAX_INPUT))
			{
				m_connectionInput[m_connectionInputCount] = (char)key;
				m_connectionInput[m_connectionInputCount + 1] = '\0';
				m_connectionInputCount++;
			}
			key = GetCharPressed();
		}

		if (IsKeyPressed(KEY_BACKSPACE))
		{
			m_connectionInputCount--;
			if (m_connectionInputCount < 0) m_connectionInputCount = 0;
			m_connectionInput[m_connectionInputCount] = '\0';
		}
	}

	m_frameCounter++;
}

void Game::Draw()
{
	ClearBackground(BLACK);

	// Connection Menu.
	// Currently broken because of a bug in BCNet.
	/*if (m_player.connected == false)
	{
		constexpr static int textSize = 36;
		constexpr static int textOffset = 12;
		constexpr static int maxTextElements = 3;
		int i = 0;

		std::string ipAddress;
		int port = -1;

		#define M_nextTextPos ( (int)((textSize + textOffset) * i++) )
		#define M_textXPosition(t) ( (int)(clientWidth / 2.0f) - (MeasureText(t, textSize) / 2) )
		#define M_textYPosition ( (int)(clientHeight / 2.0f) - (int)(textSize / 2.0f) + M_nextTextPos - (int)((textSize + textOffset) * (maxTextElements - 1) / 2.0f) )

		std::string connectionText = "Connect to a host!";
		DrawText(connectionText.c_str(), M_textXPosition(connectionText.c_str()), M_textYPosition, textSize, WHITE);
		if (m_ipEntered == false)
		{
			std::string descText = "Enter the IP Address!";
			DrawText(descText.c_str(), M_textXPosition(descText.c_str()), M_textYPosition, textSize, WHITE);

			int yPos = M_textYPosition;
			DrawText(m_connectionInput, M_textXPosition(m_connectionInput), yPos, textSize, WHITE);
			if (m_connectionInputCount < MAX_INPUT)
			{
				if (((m_frameCounter / 24) % 2) == 0)
					DrawText("_", M_textXPosition("_") + ((MeasureText(m_connectionInput, textSize) + textSize) / 2), yPos, textSize, WHITE);
			}

			if (IsKeyReleased(KEY_ENTER))
			{
				ipAddress = std::string(m_connectionInput);
				m_connectionInput[0] = '\0';
				m_connectionInputCount = 0;
				m_ipEntered = true;
				std::cout << ipAddress << std::endl;
			}
		}
		else if (m_portEntered == false)
		{
			std::string descText = "Enter the port!";
			DrawText(descText.c_str(), M_textXPosition(descText.c_str()), M_textYPosition, textSize, WHITE);

			int yPos = M_textYPosition;
			DrawText(m_connectionInput, M_textXPosition(m_connectionInput), yPos, textSize, WHITE);
			if (m_connectionInputCount < MAX_INPUT)
			{
				if (((m_frameCounter / 24) % 2) == 0)
					DrawText("_", M_textXPosition("_") + ((MeasureText(m_connectionInput, textSize) + textSize) / 2), yPos, textSize, WHITE);
			}

			if (IsKeyReleased(KEY_ENTER))
			{
				port = std::stoi(m_connectionInput);
				m_connectionInput[0] = '\0';
				m_connectionInputCount = 0;
				m_portEntered = true;
				std::cout << port << std::endl;
			}
		}

		if (m_ipEntered && m_portEntered)
		{
			std::string descText = "Connecting...";
			DrawText(descText.c_str(), M_textXPosition(descText.c_str()), M_textYPosition, textSize, WHITE);

			m_netClient->ConnectToServer(ipAddress, port);
		}

		return;
	}*/

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

	// Draw Peer Player.
	if (m_peerPlayer.connected == true)
	{
		int playerXPos = m_peerPlayer.rightSide ? (int)((1.0f - paddleXOffset) * clientWidth) : (int)(paddleXOffset * clientWidth);
		int playerYPos = (int)(m_peerPlayer.yPosition * clientHeight);
		int playerWidth = (int)(paddleWidth * clientWidth);
		int playerHeight = (int)(paddleHeight * clientHeight);

		int playerCenterX = playerXPos - (int)(playerWidth / 2.0f);
		int playerCenterY = playerYPos - (int)(playerHeight / 2.0f);

		DrawRectangle(playerCenterX, playerCenterY, playerWidth, playerHeight, RED);

		// Draw their Score.
		constexpr int screenHalf = clientWidth / 2;
		constexpr int leftHalf = screenHalf / 2;
		constexpr int rightHalf = screenHalf + leftHalf;
		int scoreXPos = m_peerPlayer.rightSide ? rightHalf : leftHalf;

		std::string scoreText = std::to_string(m_peerPlayer.score);
		DrawText(scoreText.c_str(), scoreXPos, clientHeight / 5, 48, RED);

		if (m_gameState.gameStarted == false)
		{
			std::string readyText = m_peerPlayer.ready ? "Is Ready" : "Not Ready";

			int textXPos = playerXPos + 12;
			if (m_peerPlayer.rightSide)
			{
				int textWidth = MeasureText(readyText.c_str(), 24);
				textXPos = playerXPos - textWidth - 12;
			}

			int textYPos = playerYPos - playerHeight;
			if (textYPos < 0)
				textYPos = 0;
			if (textYPos > clientHeight - 24)
				textYPos = clientHeight - 24;

			DrawText(readyText.c_str(), textXPos, textYPos, 24, RED);
		}
	}

	// Draw Client Player.
	if (m_player.connected == true)
	{
		int playerXPos = m_player.rightSide ? (int)((1.0f - paddleXOffset) * clientWidth) : (int)(paddleXOffset * clientWidth);
		int playerYPos = (int)(m_player.yPosition * clientHeight);
		int playerWidth = (int)(paddleWidth * clientWidth);
		int playerHeight = (int)(paddleHeight * clientHeight);

		int playerCenterX = playerXPos - (int)(playerWidth / 2.0f);
		int playerCenterY = playerYPos - (int)(playerHeight / 2.0f);

		DrawRectangle(playerCenterX, playerCenterY, playerWidth, playerHeight, BLUE);

		// Draw their Score.
		constexpr int screenHalf = clientWidth / 2;
		constexpr int leftHalf = screenHalf / 2;
		constexpr int rightHalf = screenHalf + leftHalf;
		int scoreXPos = m_player.rightSide ? rightHalf : leftHalf;

		std::string scoreText = std::to_string(m_player.score);
		DrawText(scoreText.c_str(), scoreXPos, (int)(clientHeight / 5.0f), 48, BLUE);

		if (m_gameState.gameStarted == false)
		{
			std::string readyText = m_player.ready ? "Is Ready" : "Not Ready";

			int textXPos = playerXPos + 12;
			if (m_player.rightSide)
			{
				int textWidth = MeasureText(readyText.c_str(), 24);
				textXPos = playerXPos - textWidth - 12;
			}

			int textYPos = playerYPos - playerHeight;
			if (textYPos < 0)
				textYPos = 0;
			if (textYPos > clientHeight - 24)
				textYPos = clientHeight - 24;

			DrawText(readyText.c_str(), textXPos, textYPos, 24, BLUE);
		}

		// Draw ready message.
		if (m_gameState.gameStarted == false && m_player.ready == false)
		{
			std::string readyMessage = "Press the spacebar to ready up!";
			int textWidth = MeasureText(readyMessage.c_str(), 24);
			DrawText(readyMessage.c_str(), (int)(clientWidth / 2.0f) - (int)(textWidth / 2.0f), (int)(clientHeight / 2.0f) - 12, 24, BLUE);
		}
	}

	m_textPool.Draw();
}

void Game::PlaySFX(eSounds sound)
{
	if (IsSoundReady(m_loadedSounds[(int)sound])) { PlaySound(m_loadedSounds[(int)sound]); }
}

void Game::OnConnected()
{ }

void Game::OnDisconnected()
{ 
	m_ipEntered = false;
	m_portEntered = false;

	m_player.connected = false;
	m_player.ready = false;
	m_playerCount--;
}

void Game::PacketReceived(const BCNet::Packet packet)
{
	BCNet::PacketStreamReader reader(packet);
	int packetID;
	reader >> packetID;

	switch (packetID)
	{
		case (int)BCNet::DefaultPacketID::PACKET_SERVER: // Need this to see messages from server.
		{
			std::string message;
			reader >> message;
			std::cout << message << std::endl;
		} break;
		case (int)PongPackets::PONG_PLAYER_CONNECTED:
		{
			uint32 id; // TODO: Implement function to get connection id in IBCNetClient.
			reader >> id;

			bool rightSide;
			reader >> rightSide;

			if (m_playerCount < 1) // Client has connected.
			{
				m_player.connected = true;
				m_player.movingUp = false;
				m_player.movingDown = false;
				m_player.yPosition = 0.5f;
				m_player.score = 0;
				m_player.rightSide = rightSide;
				m_player.ready = false;

				// Request peer info if any.
				BCNet::Packet packet;
				packet.Allocate(1024);
				BCNet::PacketStreamWriter writer(packet);
				writer << PongPackets::PONG_PLAYER_REQUEST_PEERS;
				m_netClient->SendPacketToServer(writer.GetPacket());
				packet.Release();
			}
			else // Someone else has connected.
			{
				m_peerPlayer.connected = true;
				m_peerPlayer.movingUp = false;
				m_peerPlayer.movingDown = false;
				m_peerPlayer.yPosition = 0.5f;
				m_peerPlayer.score = 0;
				m_peerPlayer.rightSide = rightSide;
				m_peerPlayer.ready = false;
			}

			m_playerCount++;
		} break;
		case (int)PongPackets::PONG_PLAYER_DISCONNECTED:
		{
			uint32 id;
			reader >> id;

			m_peerPlayer.connected = false;
			m_peerPlayer.ready = false;

			m_playerCount--;

			m_gameState.gameStarted = false;
		} break;
		case (int)PongPackets::PONG_PLAYER_REQUEST_PEERS:
		{
			m_peerPlayer.connected = true;

			reader >> m_peerPlayer.rightSide;
			reader >> m_peerPlayer.score;
			reader >> m_peerPlayer.ready;
			reader >> m_peerPlayer.yPosition;
			reader >> m_peerPlayer.movingDown;
			reader >> m_peerPlayer.movingUp;

			m_playerCount++;
		} break;
		case (int)PongPackets::PONG_PLAYER_MOVING_UP:
		{
			bool moving;
			reader >> moving;
			m_peerPlayer.movingUp = moving;
		} break;
		case (int)PongPackets::PONG_PLAYER_MOVING_DOWN:
		{
			bool moving;
			reader >> moving;
			m_peerPlayer.movingDown = moving;
		} break;
		case (int)PongPackets::PONG_PLAYER_READY:
		{
			bool ready;
			reader >> ready;
			m_peerPlayer.ready = ready;
		} break;
		case (int)PongPackets::PONG_GAME_STARTED:
		{
			m_gameState.gameStarted = true;
		} break;
		case (int)PongPackets::PONG_GAME_ENDED:
		{
			m_gameState.gameStarted = false;
			m_player.ready = false;
			m_peerPlayer.ready = false;
		} break;
		case (int)PongPackets::PONG_PLAYER_COUNTDOWN:
		{
			std::string countDownText;
			reader >> countDownText;

			m_netClient->Log(countDownText);

			int textWidth = MeasureText(countDownText.c_str(), 48);
			float textXPosition = (clientWidth / 2.0f) - (textWidth / 2.0f);
			float textYPosition = (clientHeight / 2.0f) - 24.0f;
			m_textPool.Init(countDownText, textXPosition, textYPosition, 1.0f, 48, BLUE);
		} break;
		case (int)PongPackets::PONG_BALL_RESET:
		{
			m_ball.xPosition = 0.5f;
			m_ball.yPosition = 0.5f;
			m_ball.xVelocity = -1.0f;
			m_ball.yVelocity = -1.0f;
		} break;
		case (int)PongPackets::PONG_BALL_VELOCITY:
		{
			float xVelocity, yVelocity;
			reader >> xVelocity;
			reader >> yVelocity;

			m_ball.xVelocity = xVelocity;
			m_ball.yVelocity = yVelocity;
		} break;
		case (int)PongPackets::PONG_PLAYER_SCORE:
		{
			int score;
			bool rightSide;
			reader >> rightSide;
			reader >> score;

			if (m_player.rightSide == rightSide)
				m_player.score = score;
			if (m_peerPlayer.rightSide == rightSide)
				m_peerPlayer.score = score;

			PlaySFX(eSounds::SCORE);
		} break;
		case (int)PongPackets::PONG_BALL_BOUNCE:
		{
			PlaySFX(eSounds::BOUNCE);
		} break;
		case (int)PongPackets::PONG_PLAYER_HIT:
		{
			PlaySFX(eSounds::HIT);
		} break;
		default:
		{
		} break;
	}
}

void Game::Run()
{
	InitAudioDevice();

	InitWindow(clientWidth, clientHeight, "Pong Client");
	SetTargetFPS(60);
	SetExitKey(NULL);

	Init();

	double lastTime = 1.0 / 60.0;

	m_running = true;
	while (m_running)
	{
		double currentTime = GetTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		Update(deltaTime);

		BeginDrawing();
		Draw();
		EndDrawing();

		m_running = !WindowShouldClose() || m_netClient->IsRunning();
	}

	Shutdown();

	CloseWindow();
	CloseAudioDevice();
}
