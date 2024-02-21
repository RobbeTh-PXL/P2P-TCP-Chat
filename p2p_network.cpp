#include "p2p_network.h"

/* ORI code problems
1. Each time a connection is made to a peer already connectes to the nework
the peer list will be send, this list is not processed. (firstConnect)

2. A dublicate socket will be created in firstConnect: during initial connection and during
the connection of the peers from the peer list as the created socket is also in there

3. No closing of spent sockets

4. No real error detection/handeling

5. m_sockets only keeps growing, no deletions
*/

p2p_network::p2p_network(QObject *parent) : QObject(parent) {}

bool p2p_network::connection_listener()
{
	qDebug() << "Starting internal server...";
	server = new QTcpServer(this);
	// If server has a new connection, handle it (asynch)
	connect(server, SIGNAL(new_connection()), this, SLOT(handle_new_connection()));
	// Listen for new connection to setup p2p socket
	server->listen(QHostAddress::Any, 24042);
	// TODO Custom Port and add error check
	return true;
}

void p2p_network::handle_new_connection()
{
	qDebug() << "New peer joined, sending current peer list...";
	// Create a socket for the new connection or wait until new connection
	QTcpSocket *socket = new QTcpSocket(server->nextPendingConnection());
	if (socket->waitForConnected())
	{
		// Add all current peers to peerlist
		QString peers = update_peer_list();
		// Send peerlist to new connection
		socket->write(peers.toUtf8());
		// Add socket to socket list
		m_sockets.append(socket);

		// Receive data when sended
		connect(socket, SIGNAL(readyRead()), this, SLOT(broadcast_Rx()));
	}

	else
	{
		qDebug() << "Could not connect to new peer";
		socket->close();
		delete socket;
	}

	// Get ready for new connection
	emit new_connection();
}

QString p2p_network::update_peer_list()
{
	// Not really needed
	QString peerList = "PEERLIST\n";

	// Add each active socket to the list
	for (QTcpSocket *socket : m_sockets) {
		if (socket->state() == QTcpSocket::ConnectedState) {
			QString address = QHostAddress(socket->peerAddress().toIPv4Address()).toString();
			peerList += address + ":24042\n";
			// TODO Custom Port
		}
	}

	return peerList;
}

void p2p_network::broadcast_Rx()
{
	// Read from all the sockets
	for (QTcpSocket *socket : m_sockets)
	{
		while (socket->bytesAvailable() > 0)
		{
			QString message = QHostAddress(socket->peerAddress().toIPv4Address()).toString() + +": " + QString::fromUtf8(socket->readAll());
			qDebug() << "Broadcast RX: " << message;
			//emit newMessageReceived(message);
		}
	}
}

void p2p_network::broadcast_Tx(QString msg)
{
	// Write to all the sockets
	for (QTcpSocket *socket : m_sockets)
	{
		socket->write(msg.toUtf8());
	}
}

bool p2p_network::join_network()
{
	//Connect to a p2p (tcp) server
	qDebug() << "Joining network...";
	QTcpSocket *socket = new QTcpSocket(this);
	socket->connectToHost(server_ip, server_port);

	if (socket->waitForConnected())
	{
		// Receive data when sended
		connect(socket, SIGNAL(readyRead()), this, SLOT(broadcast_Rx()));

		// Add socket to socket list
		m_sockets.append(socket);
		qDebug() << "Connected to: " << socket;

		// Read received peerlist
		QString data, peer_ip, peer_port;
		data = socket->readAll();

		// Check if there is data
		if (data.isEmpty())
		{
			qWarning() << "No data received, could not join network";
			//socket->close();
			//delete socket;

			return false;
		}

		// Split data
		QStringList data_lines = data.split("\n", Qt::SkipEmptyParts);
		// Remove trash
		if (!data_lines.first().contains(":"))
		{
			data_lines.removeFirst();
		}

		// Use pointer instead of copying data_lines
		qDebug() << "Adding peers...";
		for (QString &line: data_lines)
		{
			QStringList parts = line.split(":");
			if (parts.size() == 2)
			{
				// Add to correct var, remove whitespaces
				peer_ip = parts[0].trimmed();
				peer_port = parts[1].trimmed();

				if (peer_ip == server_ip)
				{
					// Run next itteration
					continue;
				}

				// Connect to listed peer
				QTcpSocket *peer_socket = new QTcpSocket(this);
				peer_socket->connectToHost(peer_ip, peer_port.toInt());
				if (peer_socket->waitForConnected())
				{
					// Receive data when sended
					connect(socket, SIGNAL(readyRead()), this, SLOT(broadcast_Rx()));

					// Add socket to socket list
					m_sockets.append(peer_socket);
				}

				else
				{
					qWarning() << "Could not connect to peer: " << line;
					peer_socket->close();
					delete peer_socket;
				}
			}

			else
			{
				qWarning() << "Invalid format of received peer adress: " << line;
			}
		}
	}

	else
	{
		qWarning() << "Could not connect to server: " << server_ip << ":" << server_port;
		return false;
	}

	// Accept new connections to the network (peer list complete, all connections made)
	emit new_connection();

	return true;
}
