#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <QCoreApplication>
#include <QDebug>

#include <QCommandLineParser>

#include <random>
#include <vector>
#include <sstream>

struct ParsedArguments {
	QString nickname;
	int listen_port;
	bool initial_connection = false;
	QString server_ip;
	int server_port;
};

class cli_parser
{

public:
	cli_parser ();
	bool parse(const QStringList& arguments);

	ParsedArguments getParsed_arguments() const;

private:
	QCommandLineParser parser;
	ParsedArguments parsed_arguments;

	int generateRandomNumber();
	bool isValidIPv4(const std::string& ipAddress);
};

#endif // CLI_PARSER_H
