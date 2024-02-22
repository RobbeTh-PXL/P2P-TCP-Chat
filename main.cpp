#include <QDebug>
#include <QApplication>
#include <QCommandLineParser>

#include "cli_parser.h"
#include "p2p_network.h"
//#include "mainwindow.h"

#include <QString>
#include <QTextStream>

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

	// CLI

	QTextStream input_stream(stdin);
	QString input;
	qDebug() << "Enter messages to send (type 'exit' to quit):";

	// Loop until the user enters the exit command
	while (true) {
		// Read input from stdin
		input = input_stream.readLine();

		// Check if the user entered the exit command
		if (input == "exit") {
			qDebug() << "Exiting...";
			break;  // Break out of the loop
		}

		// Send the input message to the network
		if (!input.isEmpty())
		{
			qDebug() << "cli read: " << input;
			network.broadcast_Tx(input);
		}
	}


	//GUI
	//MainWindow w;
	//w.show();

    return a.exec();
}
