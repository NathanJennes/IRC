//
// Created by nathan on 2/16/23.
//

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include "Server.h"

const int	Server::m_server_backlog = 10;
const int	Server::m_timeout = 1000;

bool Server::initialize(uint16_t port)
{
	m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server_socket < 0) {
		std::cerr << "Error: socket: " << strerror(errno) << std::endl;
		return false;
	}

	bool is_active = true;
	if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &is_active, sizeof(bool)) == -1) {
		std::cerr << "Error: setsockopt: " << strerror(errno) << std::endl;
		return false;
	}

	struct sockaddr_in params = {};
	params.sin_family = AF_INET;
	params.sin_addr.s_addr = htonl(INADDR_ANY);
	params.sin_port = htons(port);

	if (bind(m_server_socket, (const struct sockaddr *) &params, sizeof (params)) < 0) {
		std::cerr << "Error: bind: " << strerror(errno) << std::endl;
		return false;
	}

	if (fcntl(m_server_socket, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << "Error: fcntl: " << strerror(errno) << std::endl;
		return false;
	}

	if (listen(m_server_socket, m_server_backlog)) {
		std::cerr << "Error: listen: " << strerror(errno) << std::endl;
		return false;
	}

	m_is_running = true;

	return true;
}

bool Server::update()
{
	accept_new_connections();
	poll_events();
	handle_events();
	return true;
}

void Server::accept_new_connections() const
{
	struct sockaddr_in client = {};
	socklen_t len = sizeof (client);

	int new_client_socket_fd = accept(m_server_socket, reinterpret_cast<sockaddr *>(&client), &len);
	if (new_client_socket_fd <= 0)  {
		std::cerr << "Error: accept: " << strerror(errno) << std::endl;
	}

	std::cout << "Incomming connexion from :" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << std::endl;
}

void Server::poll_events()
{
	std::vector<pollfd> pollfds(m_users.size());

	size_t i = 0;
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++, i++) {
		pollfds[i].fd = user->fd();
		pollfds[i].events = POLLIN | POLLOUT;
		pollfds[i].revents = 0;
	}

	int result = poll(pollfds.data(), pollfds.size(), m_timeout);
	if (result < 0) {
        std::cerr << "Error: poll: " << strerror(errno) << std::endl;
    }

	i = 0;
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		user->set_is_readable(pollfds[i].revents & POLLIN);
		user->set_is_writable(pollfds[i].revents & POLLOUT);
	}
}

void Server::handle_events()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		if (user->is_readable())
			;
    }
}
