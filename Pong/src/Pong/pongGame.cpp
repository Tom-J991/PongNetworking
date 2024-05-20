#include "pongGame.h"

#include <string>

PongState::PongState()
{ 
	m_ball = std::make_unique<Ball>();
	m_firstPlayer = std::make_unique<Paddle>(1);
	m_secondPlayer = std::make_unique<Paddle>(2);

	m_loadedSounds.resize((int)eSounds::SOUNDS_MAX);

	m_netClient = BCNet::InitClient();
}
PongState::~PongState()
{ 
}

void PongState::Init()
{
	m_netClient->SetPacketReceivedCallback([this](const BCNet::Packet packet) { PacketReceived(packet); });

	m_netClient->Start();

	// Load Assets
	m_loadedSounds[(int)eSounds::BOUNCE] = LoadSound("./assets/sfx/pong/ball_bounce.ogg");
	m_loadedSounds[(int)eSounds::BOUNDS] = LoadSound("./assets/sfx/pong/ball_bounds.ogg");
	m_loadedSounds[(int)eSounds::SCORE] = LoadSound("./assets/sfx/pong/score.ogg");

	// Initialize
	m_ball->Init();
	m_firstPlayer->Init();
	m_secondPlayer->Init();
}
void PongState::Shutdown()
{
	m_netClient->Stop();

	// Unload Assets
	for (int i = 0; i < (int)eSounds::SOUNDS_MAX; i++)
		if (IsSoundReady(m_loadedSounds[i]))
			UnloadSound(m_loadedSounds[i]);
	m_loadedSounds.clear();
	m_loadedSounds.resize((int)eSounds::SOUNDS_MAX);
}

bool PongState::Update(float deltaTime)
{
	// Move.
	if (m_ball->CheckCollision(*m_firstPlayer.get(), deltaTime) || m_ball->CheckCollision(*m_secondPlayer.get(), deltaTime))
		PlaySFX(eSounds::BOUNCE);
	if (m_ball->CheckBounds(deltaTime))
		PlaySFX(eSounds::BOUNDS);

	m_ball->Move(deltaTime);
	m_firstPlayer->Move(*m_ball.get(), deltaTime);
	m_secondPlayer->Move(*m_ball.get(), deltaTime);

	// Score.
	if (m_ball->Position().x > GetScreenWidth())
		m_firstPlayer->SetScore(m_firstPlayer->Score() + 1);
	else if (m_ball->Position().x < 0)
		m_secondPlayer->SetScore(m_secondPlayer->Score() + 1);

	// Reset ball when out of bounds.
	if (m_ball->Position().x > GetScreenWidth() || m_ball->Position().x < 0)
	{
		m_ball->Init();

		PlaySFX(eSounds::SCORE);
	}

	return true;
}
void PongState::Draw()
{
	BeginDrawing();
	ClearBackground(BLACK);
	{
		// Draw Centre-line.
		const Vector2 centreLineSize = { 8, 12 };
		for (int i = 0; i < GetScreenHeight(); i += (int)centreLineSize.y * 2)
		{
			DrawRectangle(GetScreenWidth() / 2 - ((int)centreLineSize.x / 2), i - ((int)centreLineSize.x / 2), (int)centreLineSize.x, (int)centreLineSize.y, WHITE);
		}

		// Draw Game Objects.
		m_firstPlayer->Draw();
		m_secondPlayer->Draw();
		m_ball->Draw();

		// Scores.
		const int screenHalf = GetScreenWidth() / 2;
		const int leftHalf = screenHalf / 2;
		const int rightHalf = screenHalf + leftHalf;
		DrawText(std::to_string((int)m_firstPlayer->Score()).c_str(), leftHalf, GetScreenHeight() / 5, 48, WHITE);
		DrawText(std::to_string((int)m_secondPlayer->Score()).c_str(), rightHalf, GetScreenHeight() / 5, 48, WHITE);

	}
	EndDrawing();
}

void PongState::PlaySFX(eSounds sound)
{
	if (IsSoundReady(m_loadedSounds[(int)sound]))
		PlaySound(m_loadedSounds[(int)sound]);
}

void PongState::PacketReceived(const BCNet::Packet packet)
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
	}
}
