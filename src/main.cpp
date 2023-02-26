//
// Created by nathan on 2/16/23.
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <csignal>
#include "Server.h"
#include "log.h"

void initialize_signals()
{
	struct sigaction sa;
	sa.sa_handler = Server::signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
}

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

	initialize_signals();

	Server server;
	if (!server.initialize(static_cast<uint16_t>(port))) {
		std::cout << "Error: Couldn't initialize server" << std::endl;
		server.cleanup();
		return 1;
	}
	else
		CORE_DEBUG("Server is running on port %d", port);

	while (server.is_running()) {
		server.update();
	}
	server.cleanup();
	CORE_DEBUG("Server is shutting down");
}
