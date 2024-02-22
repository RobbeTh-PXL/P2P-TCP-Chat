#include "cli_parser.h"

cli_parser::cli_parser()
{
	// Initialize the parser in the constructor
	parser.setApplicationDescription("P2P Chat");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({{"n", "nickname"}, "Specify an username.", "nickname"});
	parser.addOption({{"p", "listen_port"}, "Specify the port for incoming connections.", "listen_port", "24042"});
	parser.addOption({{"c", "connect_to_network"}, "Make initial connection. [ip:port]", "connect_to_network"});
}

cli_parser::~cli_parser() {}

bool cli_parser::parse(const QStringList& arguments)
{
	// Process the command line arguments
	parser.process(arguments);

	// Check if help option is set
	if (parser.isSet("help")) {
		parser.showHelp();
		return false;
	}

	// Check if version option is set
	if (parser.isSet("version")) {
		parser.showVersion();
		return false;
	}

	// Retrieve the values of options
	parsed_arguments.nickname = parser.value("nickname");
	parsed_arguments.listen_port = parser.value("listen_port").toInt();
	QString connect_to_network = parser.value("connect_to_network");

	// Validate and process the connect_to_network option
	if (!connect_to_network.isEmpty()) {
		qDebug() << "Initial Connection:" << connect_to_network;
		QStringList connectionParams = connect_to_network.split(':');
		if (connectionParams.size() == 2) {
			QString server_ip = connectionParams[0];
			int server_port = connectionParams[1].toInt();

			// Validate IPv4 address
			if (!isValidIPv4(server_ip.toStdString())) {
				qWarning() << "Invalid IPv4 address";
				return false;
			}

			// Set the validated values
			parsed_arguments.initial_connection = true;
			parsed_arguments.server_ip = server_ip;
			parsed_arguments.server_port = server_port;
			qDebug() << "Connection IP:" << parsed_arguments.server_ip;
			qDebug() << "Connection Port:" << parsed_arguments.server_port;
		} else {
			qWarning() << "Invalid format for initial connection.";
			return false;
		}
	}

	// Check for unrecognized options
	const QStringList args = parser.positionalArguments();
	if (!args.isEmpty()) {
		for (const QString& arg : args) {
			qWarning() << "Unrecognized option:" << arg;
		}
		return false;
	}

	return true;
}


int cli_parser::generateRandomNumber() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1000, 9999); // Adjust the range as needed
	return dis(gen);
}

bool cli_parser::isValidIPv4(const std::string& ipAddress)
{
	// Split the IP address by '.' into octets
	std::stringstream ss(ipAddress);
	std::string octet;
	std::vector<int> octets;
	while (std::getline(ss, octet, '.')) {
		try {
			int value = std::stoi(octet);
			if (value < 0 || value > 255)
				return false; // Octet out of range
			octets.push_back(value);
		} catch (std::invalid_argument&) {
			return false; // Not a valid integer
		} catch (std::out_of_range&) {
			return false; // Integer out of range
		}
	}

	// Check if there are exactly 4 octets
	if (octets.size() != 4)
		return false;

	return true;
}

ParsedArguments cli_parser::getParsed_arguments() const
{
	return parsed_arguments;
}
