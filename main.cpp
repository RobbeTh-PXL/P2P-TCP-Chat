#include "mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>

#include "cli_parser.h"
#include "p2p_network.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// CLI Parser
	cli_parser parser;

	// Process args
	if (!parser.parse(a.arguments()))
	{
		return -1;
	}

	ParsedArguments parsedArgs = parser.getParsed_arguments();

	// Network
	p2p_network network;

	// Set vars
	network.setServer_ip(parsedArgs.server_ip);
	network.setServer_port(parsedArgs.server_port);

	if (parsedArgs.initial_connection)
	{
		network.join_network();
		network.connection_listener();
	}

	else
	{
		network.connection_listener();
	}

    MainWindow w;
    w.show();

    return a.exec();
}
