//
// Created by nathan on 2/16/23.
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include "IRC.h"
#include "Server.h"
#include "Command.h"

const int	Server::m_server_backlog = 10;
const int	Server::m_timeout = 500;

bool Server::initialize(uint16_t port)
{
	m_server_name = "IRC Server";

	m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server_socket < 0) {
		std::cerr << "Error: socket: " << strerror(errno) << std::endl;
		return false;
	}

	int is_active = 0;
	if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &is_active, sizeof(int)) == -1) {
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
	m_is_readonly = false;

	if (!initialize_config_file()) {
		std::cerr << "Error: " << strerror(errno) << std::endl;
		std::cerr << "Server is in readonly mode" << std::endl;
		m_is_readonly = true;
	}

	m_users.push_back(User("Server", "Server", "Server", m_server_socket));

	return true;
}

bool Server::update()
{
	poll_events();
	handle_events();
	return true;
}

void Server::poll_events()
{
	std::vector<pollfd> pollfds(m_users.size());

	size_t i = 0;
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++, i++) {
		pollfds[i].fd = user->fd();
		if (i == 0)
			pollfds[i].events = POLLIN;
		else
			pollfds[i].events = POLLIN | POLLOUT;
		pollfds[i].revents = 0;
	}

	int poll_count = poll(pollfds.data(), (unsigned int)pollfds.size(), m_timeout);
	if (poll_count < 0) {
		std::cerr << "Error: poll: " << strerror(errno) << std::endl;
	}

	i = 0;
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++, i++) {
		user->set_is_readable(pollfds[i].revents & POLLIN);
		user->set_is_writable(pollfds[i].revents & POLLOUT);
	}
}

void Server::handle_events()
{
	for (size_t i = 0; i < m_users.size(); ++i)
	{
		if (m_users[i].is_readable())
		{
			std::cout << " is readable" << std::endl;
			if (m_users[i].fd() == m_server_socket) {
				accept_new_connections();
			}
			else {
				if (m_users[i].receive_message() <= 0)
				{
					m_users[i].disconnect();
					std::cout << RPL_LOGGEDOUT(m_users[i].username());
					m_users.erase(m_users.begin() + (long)i);
				}
			}
		}
		else if (m_users[i].is_writable())
		{
//			m_users[i].send_message();
		}
	}
}

void Server::accept_new_connections()
{
	struct sockaddr_in client = {};
	socklen_t len = sizeof(client);

	int new_client_socket_fd = accept(m_server_socket, reinterpret_cast<sockaddr *>(&client), &len);
	if (new_client_socket_fd <= 0)  {
		std::cerr << "Error: accept: " << strerror(errno) << std::endl;
	}
	std::cout << "Incomming connexion from :" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << std::endl;

	std::string username = gethostbyaddr(&client.sin_addr, client.sin_len, AF_INET)->h_name;
	std::cout << "Username: " << username << std::endl;

	std::stringstream realname;
	realname << inet_ntoa(client.sin_addr) << ":" << std::to_string(ntohs(client.sin_port));
	std::cout << "Realname: " << realname.str() << std::endl;

	User user(username, realname.str(), server_name(), new_client_socket_fd);
	m_users.push_back(user);
	std::cout << "New user added | Size: " << m_users.size() << std::endl;
}

void Server::handle_messages()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		if (!user->is_readable() || user->is_disconnected())
			continue ;

		std::string command_str = user->get_next_command_str();
		while (!command_str.empty()) {
			std::cout << "Received command [" << command_str << "] from: " << user->username() << std::endl;
			//Command command(command_str);
			//if (command.is_valid())
			//	command.execute(*user);
		}
	}
}

void Server::execute_command(User &user)
{
	(void)user;
}

bool Server::initialize_config_file()
{
	std::fstream banlist_file("banlist.txt", std::ios::in | std::ios::out | std::ios::app);
	if (!banlist_file.is_open()) {
		std::cerr << "Error: banlist.txt: " << strerror(errno) << std::endl;
		return false;
	}

	std::fstream whitelist_file("whitelist.txt", std::ios::in | std::ios::out | std::ios::app);
	if (!whitelist_file.is_open()) {
		std::cerr << "Error: whitelist.txt: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}
