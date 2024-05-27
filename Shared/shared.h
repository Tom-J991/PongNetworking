#pragma once

#include <BCNet/BCNetPacket.h>

enum class PongPackets : int
{
	PONG_PLAYER_MOVING_UP = DEFAULT_PACKETS_COUNT + 1, // Player is moving up.
	PONG_PLAYER_MOVING_DOWN, // Player is moving down.

	PONG_PLAYER_HIT, // Player's paddle has hit the ball.
	PONG_PLAYER_SCORE, // Ball goes out of bounds, i.e. player scores a point.
	PONG_PLAYER_LOSE, // Tell the player they've lost.
	PONG_PLAYER_WIN, // Tell the player they've won.

	PONG_PLAYER_READY, // Player has readied up.
	PONG_PLAYER_CONNECTED, // Player has connected.
	PONG_PLAYER_DISCONNECTED, // Player has disconnected.
	PONG_PLAYER_REQUEST_PEERS, // New player requests peer info.

	PONG_BALL_RESET, // Resets the ball to default.
	PONG_BALL_VELOCITY, // Return ball's current velocity.
	PONG_BALL_BOUNCE, // Paddle bounce off of bounds

	PONG_GAME_STARTED, // Game has commenced.
	PONG_GAME_ENDED, // Game has finished.

	PONG_PACKET_COUNT // MAX
};
