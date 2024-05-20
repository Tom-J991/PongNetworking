#pragma once

#include <iostream>
#include <memory>

#include <raylib.h>

#include "Pong/pongGame.h"

class Game
{
public:
	Game();
	~Game();

	void Update(float deltaTime);
	void Draw();

	void Quit() { m_quit = true; }
	bool IsQuitting() const { return m_quit; }

	// Singleton. Get Instance
	static Game &Get() {
		if (m_instance == nullptr) // Don't recreate itself if it already exists.
			m_instance = new Game(); 
		return *m_instance; 
	};

	// Destroy Instance
	static void Destroy()
	{
		if (m_instance != nullptr) // Don't try to delete if it doesn't exist.
			delete m_instance;
		m_instance = nullptr;
	}

private:
	bool m_quit;

	std::unique_ptr<PongState> m_pong;

private:
	static Game *m_instance;

};
