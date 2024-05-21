#include <iostream>

#include <BCNet/IBCNetServer.h>
#include <BCNet/BCNetPacket.h>
#include <BCNet/BCNetUtil.h>

static BCNet::IBCNetServer *g_server;

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
	g_server = BCNet::InitServer();

	g_server->SetConnectedCallback(OnConnected);
	g_server->SetPacketReceivedCallback(PacketReceived);

	g_server->SetMaxClients(2);

	g_server->Start();
	g_server->Stop();

	if (g_server)
		delete g_server;
	g_server = nullptr;

	return 0;
}
