//
// Created by nathan on 2/16/23.
//

#include <iostream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <limits>
#include "Server.h"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Error: wrong number of arguments." << std::endl;
		std::cout << "Usage: ./irc_server <port> <password>" << std::endl;
		return 1;
	}

	int port = std::atoi(argv[1]);
	if (std::strlen(argv[1]) == 0 || port > std::numeric_limits<uint16_t>::max() || port <= 0) {
		std::cout << "Error: wrong port." << std::endl;
		std::cout << "Port need to be between 0 and 65535" << std::endl;
		return 1;
	}

	if (std::strlen(argv[2]) == 0) {
		std::cout << "Error: wrong password." << std::endl;
		std::cout << "Usage: ./irc_server <port> <password>" << std::endl;
		return 1;
	}

	Server server;
	if (!server.initialize(static_cast<uint16_t>(port))) {
		std::cout << "Error: Couldn't initialize server" << std::endl;
	}

	std::cout << "Server is running on port " << port << std::endl;

	while (server.is_running()) {
		server.update();
	}
}
