#include "tcp_network.h"

tcp_network::tcp_network(QObject *parent) : QObject(parent) {}

bool tcp_network::p2p_network()
{
	server = new QTcpServer(this);
	//CONNECT
	server->listen(QHostAddress::Any, 24042);
	return true;
}
