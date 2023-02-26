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
#include <unistd.h>
#include <string>
#include "log.h"
#include "IRC.h"
#include "Server.h"
#include "Server_commands.h"
#include "Command.h"

const int	Server::m_server_backlog = 10;
const int	Server::m_timeout = 10;
bool		Server::m_is_running = true;

void Server::signal_handler(int signal)
{
	(void) signal;
	m_is_running = false;
}

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

	initialize_command_functions();

	m_is_running = true;
	m_is_readonly = false;

	if (!initialize_config_file()) {
		std::cerr << "Error: " << strerror(errno) << std::endl;
		std::cerr << "Server is in readonly mode" << std::endl;
		m_is_readonly = true;
	}

	return true;
}

bool Server::update()
{
	accept_new_connections();
	poll_events();
	handle_events();
	handle_messages();
	disconnect_users();
	return true;
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

	int poll_count = poll(pollfds.data(), (unsigned int)pollfds.size(), m_timeout);
	if (poll_count < 0 && errno != EINTR) {
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
	size_t i;
	for (i = 0; i < m_users.size(); ++i)
	{
		if (m_users[i].is_readable()) {
			if (m_users[i].receive_message() <= 0)
				m_users[i].disconnect();
		}
		if (m_users[i].is_writable()) {
			if (!m_users[i].write_buffer().empty() && m_users[i].send_message() <= 0)
				m_users[i].disconnect();
		}
		// TODO: handle errors
	}
}

void Server::accept_new_connections()
{
	pollfd pollfd = {};

	pollfd.fd = m_server_socket;
	pollfd.events = POLLIN;
	pollfd.revents = 0;

	int poll_count = poll(&pollfd, 1, m_timeout);
	if (poll_count < 0 && errno != EINTR)
		std::cerr << "Error: poll: " << strerror(errno) << std::endl;

	if ((pollfd.revents & POLLIN) == 0)
		return ;

	struct sockaddr_in client = {};
	socklen_t len = sizeof(client);

	int new_client_socket_fd = accept(m_server_socket, reinterpret_cast<sockaddr *>(&client), &len);
	if (new_client_socket_fd <= 0)
		std::cerr << "Error: accept: " << strerror(errno) << std::endl;

	std::cout << "Incomming connexion from :" << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << std::endl;

	m_users.push_back(User(new_client_socket_fd));
}

void Server::handle_messages()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++)
	{
		if (user->read_buffer().empty() || user->is_disconnected())
			continue ;

		std::string	command_str = user->get_next_command_str();
		CORE_DEBUG("Server command str: [%s]", command_str.c_str());
		Command		command(command_str);

		command.print();
		if (command.is_valid())
			execute_command(*user, command);
	}
}

void Server::execute_command(User &user, const Command &cmd)
{
	CommandIterator it = m_commands.find(cmd.get_command());
	if (it != m_commands.end())
		it->second(*this, user, cmd);
	else
		user.reply(ERR_UNKNOWNCOMMAND(cmd.get_command()));
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

void Server::disconnect_users()
{
	for (size_t i = 0; i < m_users.size(); ++i)
	{
		if (m_users[i].is_disconnected()) {
			CORE_DEBUG("%s disconnected\n\r", m_users[i].source().c_str());
			close(m_users[i].fd());
			// TODO: save user data
			m_users.erase(m_users.begin() + (long)i);
		}
	}
}

void Server::cleanup()
{
	std::cout << "Cleaning up" << std::endl;
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++)
		close(user->fd());

	close(m_server_socket);
}

void Server::send_to_client(const User& user, const std::string &message)
{
	ssize_t total_bytes_write = 0;

	while (total_bytes_write < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_write = write(user.fd(), message.c_str() + total_bytes_write, static_cast<size_t>(MAX_MESSAGE_LENGTH - total_bytes_write));

		if (bytes_write < 0 && errno != EAGAIN)
			std::cerr << "Error: " << std::strerror(errno) << std::endl;

		if (bytes_write <= 0)
			break ;

		total_bytes_write += bytes_write;
	}
	if (total_bytes_write < (ssize_t)message.size())
		std::cerr << "Error: write: message truncated" << std::endl;
}

void Server::initialize_command_functions()
{
	m_commands.insert(std::make_pair("AUTH", auth));
	m_commands.insert(std::make_pair("CAP", cap));
	m_commands.insert(std::make_pair("ERROR", error));
	m_commands.insert(std::make_pair("NICK", nick));
	m_commands.insert(std::make_pair("OPER", oper));
	m_commands.insert(std::make_pair("PASS", pass));
	m_commands.insert(std::make_pair("PING", ping));
	m_commands.insert(std::make_pair("PONG", pong));
	m_commands.insert(std::make_pair("USER", user));
	m_commands.insert(std::make_pair("QUIT", quit));

	m_commands.insert(std::make_pair("JOIN", join));

}
