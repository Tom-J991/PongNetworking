#include <iostream>

#include <BCNet/IBCNetServer.h>
#include <BCNet/BCNetPacket.h>
#include <BCNet/BCNetUtil.h>

#define MAX_PLAYERS 2

static BCNet::IBCNetServer *g_server;

void OnConnected(const BCNet::ClientInfo &clientInfo)
{
	if (g_server->GetConnectedCount() > MAX_PLAYERS) // Ensure max players.
	{
		// TODO: Tell client that the server is full.
		std::cout << "Kicked Client " << clientInfo.nickName << ": Server is FULL" << std::endl;
		g_server->KickClient(clientInfo.id);
	}
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
	g_server = BCNet::InitServer();

	g_server->SetConnectedCallback(OnConnected);
	g_server->SetPacketReceivedCallback(PacketReceived);

	g_server->Start();
	g_server->Stop();

	if (g_server)
		delete g_server;
	g_server = nullptr;

	return 0;
}
