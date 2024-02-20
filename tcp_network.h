#ifndef TCP_NETWORK_H
#define TCP_NETWORK_H

#include <QCoreApplication>
#include <QObject>
#include <QDebug>

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

class tcp_network : public QObject
{
	Q_OBJECT

public:
	explicit tcp_network(QObject *parent = nullptr);
	bool p2p_network();

private:
	QTcpServer *server;
};

#endif // TCP_NETWORK_H
