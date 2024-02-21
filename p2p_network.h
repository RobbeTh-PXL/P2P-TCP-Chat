#ifndef P2P_NETWORK_H
#define P2P_NETWORK_H

#include <QCoreApplication>
#include <QObject>
#include <QDebug>

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>

class p2p_network : public QObject
{
	Q_OBJECT

public:
	explicit p2p_network(QObject *parent = nullptr);
	bool connection_listener();
	bool join_network();

signals:
	//Signal of QTcpServer needs te be triggered outside of class
	void new_connection();

public slots:
	void handle_new_connection();
	void broadcast_Rx();

private:
	// TODO getter setter
	QString server_ip = "127.0.0.1";
	int server_port = 43101;
	QTcpServer *server;
	QList<QTcpSocket *> m_sockets;

	QString update_peer_list();
	void broadcast_Tx(QString msg);
};

#endif // P2P_NETWORK_H
