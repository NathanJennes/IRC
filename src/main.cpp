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
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		CORE_ERROR("Error: wrong number of arguments.\nUsage: ./irc_server <port> (<password>)");
		return 1;
	}

	int port = std::atoi(argv[1]);
	if (std::strlen(argv[1]) == 0 || port > std::numeric_limits<uint16_t>::max() || port <= 0) {
		CORE_ERROR("Error: wrong port. Port need to be between 0 and 65535");
		return 1;
	}

	if (argc >= 3 && std::strlen(argv[2]) == 0) {
		CORE_ERROR("Error: Password can't be empty.\nUsage: ./irc_server <port> (<password>)");
		return 1;
	}

	initialize_signals();

	if (!Server::initialize(static_cast<uint16_t>(port))) {
		CORE_ERROR("Couldn't initialize server");
		Server::shutdown();
		return 1;
	}

	if (argc >= 3)
		Server::set_password(argv[2]);

	CORE_INFO("Server is running on port %d", port);

	while (Server::is_running()) {
		Server::update();
	}
	Server::shutdown();
}
