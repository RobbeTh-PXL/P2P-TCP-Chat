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

	QString getServer_ip() const;
	void setServer_ip(const QString &newServer_ip);
	int getServer_port() const;
	void setServer_port(int newServer_port);

signals:

public slots:
	void handle_new_connection();
	void broadcast_Rx();
	void handle_socket_state_changed();

private:
	// TODO getter setter
	QString server_ip;
	int server_port;
	QTcpServer *server;
	QList<QTcpSocket *> m_sockets;

	QString update_peer_list();
	void broadcast_Tx(QString msg);
};

#endif // P2P_NETWORK_H
