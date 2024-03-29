#include "p2p_network.h"

/* ORI code problems
1. Each time a connection is made to a peer already connectes to the nework
the peer list will be send, this list is not processed. (firstConnect)

2. A dublicate socket will be created in firstConnect: during initial connection and during
the connection of the peers from the peer list as the created socket is also in there

3. No closing of spent sockets

4. No real error detection/handeling

5. m_sockets only keeps growing, no deletions

6. All sockets are looped through rather than the using socket that emitted the signal
*/

p2p_network::p2p_network(QObject *parent) : QObject(parent) {}

p2p_network::~p2p_network()
{
	// Clean up resources
	delete server; // Release the QTcpServer object
	for (QTcpSocket *socket : m_sockets) {
		socket->close();
		delete socket; // Release each QTcpSocket object
	}
	m_sockets.clear(); // Clear the list of sockets
}

bool p2p_network::connection_listener()
{
	qDebug() << "Starting internal server...";
	server = new QTcpServer(this);
	// If server has a new connection, handle it (asynch)
	connect(server, SIGNAL(newConnection()), this, SLOT(handle_new_connection()));
	// Listen for new connection to setup p2p socket
	server->listen(QHostAddress::Any, 24042);
	// TODO Custom Port and add error check
	return true;
}

void p2p_network::handle_new_connection()
{
	while (server->hasPendingConnections())
	{
		qDebug() << "New peer joined, sending current peer list...";
		// Create a socket for the new connection or wait until new connection
		QTcpSocket *socket = server->nextPendingConnection();
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
			// Remove connection on remote disconnect
			connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handle_socket_state_changed()));
		}

		else
		{

			qDebug() << "Could not connect to new peer";
			qDebug() << "Socket error:" << socket->errorString();
			// Check if socket still exist before yeeting it
			if (socket)
			{
				socket->close();
				delete socket;
			}
		}
	}
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
	// Find the socket that emitted the signal
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
		return; // If sender is not a QTcpSocket, return

	while (socket->bytesAvailable() > 0)
	{
		QString message = QHostAddress(socket->peerAddress().toIPv4Address()).toString() + +": " + QString::fromUtf8(socket->readAll());
		qDebug() << "Broadcast RX: " << message;
		//emit newMessageReceived(message);
	}

}

void p2p_network::broadcast_Tx(QString msg)
{
	// Write to all the sockets
	for (QTcpSocket *socket : m_sockets)
	{
		int state = socket->write(msg.toUtf8());

		// Check if message is send
		if (state == -1)
		{
			// Not send, remove from list
			qWarning() << "Peer " << socket << " did not receive the message, and is removed";
			qDebug() << "Socket error:" << socket->errorString();
			// Check if socket still exist before yeeting it
			if (socket)
			{
				socket->close();
				m_sockets.removeOne(socket);
				delete socket;
			}
		}
	}
}

bool p2p_network::join_network()
{
	//Connect to a p2p (tcp) server
	qDebug() << "Target server: " << server_ip << ":" << server_port;
	qDebug() << "Joining network...";
	QTcpSocket *socket = new QTcpSocket(this);
	socket->connectToHost(server_ip, server_port);

	if (socket->waitForConnected())
	{
		qDebug() << "Connected to: " << server_ip;

		// Read received peerlist
		QString data, peer_ip, peer_port;
		socket->waitForReadyRead();
		data = socket->readAll();

		// Receive new data when sended
		connect(socket, SIGNAL(readyRead()), this, SLOT(broadcast_Rx()));
		// Remove connection on remote disconnect
		connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handle_socket_state_changed()));

		// Add socket to socket list
		m_sockets.append(socket);

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
					connect(peer_socket, SIGNAL(readyRead()), this, SLOT(broadcast_Rx()));
					// Remove connection on remote disconnect
					connect(peer_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(handle_socket_state_changed()));

					// Add socket to socket list
					m_sockets.append(peer_socket);
				}

				else
				{
					qWarning() << "Could not connect to peer: " << line;
					qDebug() << "Socket error:" << peer_socket->errorString();

					// Check if socket still exist before yeeting it
					if (socket)
					{
						peer_socket->close();
						delete peer_socket;
					}
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
		qDebug() << "Socket error:" << socket->errorString();

		// Check if socket still exist before yeeting it
		if (socket)
		{
			socket->close();
			delete socket;
			return false;
		}
	}

	qDebug() << "All peers added";
	return true;
}

void p2p_network::handle_socket_state_changed()
{
	// Find the socket that emitted the signal
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
		return; // If sender is not a QTcpSocket, return

	if (socket->state() == QAbstractSocket::UnconnectedState)
	{
		qDebug() << "Peer " << socket->peerAddress().toIPv4Address() << " lost connection";
		// Check if socket still exist before yeeting it
		if (socket)
		{
			socket->close();
			m_sockets.removeOne(socket);
			delete socket;
		}
	}
}


QString p2p_network::getServer_ip() const
{
	return server_ip;
}

void p2p_network::setServer_ip(const QString &newServer_ip)
{
	server_ip = newServer_ip;
}

int p2p_network::getServer_port() const
{
	return server_port;
}

void p2p_network::setServer_port(int newServer_port)
{
	server_port = newServer_port;
}
