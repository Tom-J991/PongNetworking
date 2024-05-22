#pragma once

#include <BCNet/BCNetPacket.h>

enum class PongPackets : int
{
	PONG_PLAYER_MOVING_UP = DEFAULT_PACKETS_COUNT + 1,
	PONG_PLAYER_MOVING_DOWN,
	PONG_BALL_POSITION,
	PONG_PACKET_COUNT
};
