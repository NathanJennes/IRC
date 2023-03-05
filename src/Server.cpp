//
// Created by nathan on 2/16/23.
//

#include <fstream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include "log.h"
#include "IRC.h"
#include "Server.h"
#include "Message.h"
#include "Command.h"

const int			Server::m_server_backlog = 10;
const int			Server::m_timeout = 20;
const std::string	Server::m_creation_date = "2021-02-16 23:00:00";
const std::string	Server::m_user_modes = "none";
const std::string	Server::m_channel_modes = "none";
const std::string	Server::m_channel_modes_parameter = "none";

std::string			Server::m_network_name = "GigaChat";
std::string			Server::m_server_name = "localhost";
int					Server::m_server_socket;
bool				Server::m_is_running = true;
bool				Server::m_is_readonly = true;
std::string			Server::m_password;

std::vector<pollfd>	Server::m_pollfds;

std::vector<User>								Server::m_users;
std::vector<Channel>							Server::m_channels;
std::map<std::string, Server::command_function>	Server::m_commands;
std::map<std::string, Server::command_function>	Server::m_connection_commands;

void Server::signal_handler(int signal)
{
	(void) signal;
	m_is_running = false;
}

bool Server::initialize(uint16_t port)
{
	m_network_name = "IRC Server";

	m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_server_socket < 0) {
		CORE_ERROR("socket: %s", strerror(errno));
		return false;
	}

	int is_active = 1;
	if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &is_active, sizeof(int)) == -1) {
		CORE_ERROR("setsockopt: %s", strerror(errno));
		return false;
	}

	struct sockaddr_in params = {};
	bzero(&params, sizeof(params));

	params.sin_family = AF_INET;
	params.sin_addr.s_addr = INADDR_ANY;
	params.sin_port = htons(port);

	if (bind(m_server_socket, (const struct sockaddr *) &params, sizeof (params)) < 0) {
		CORE_ERROR("bind: %s", strerror(errno));
		return false;
	}

	if (listen(m_server_socket, m_server_backlog)) {
		CORE_ERROR("listen: %s", strerror(errno));
		return false;
	}

	initialize_command_functions();

	m_is_running = true;
	m_is_readonly = false;

	if (!initialize_config_file()) {
		CORE_WARN("Server is in readonly mode");
		m_is_readonly = true;
	}

	pollfd m_server_pollfd = {};
	m_server_pollfd.fd = m_server_socket;
	m_server_pollfd.events = POLLIN;
	m_server_pollfd.revents = 0;
	m_pollfds.push_back(m_server_pollfd);

	return true;
}

void Server::initialize_command_functions()
{
	// register commands
	m_connection_commands.insert(std::make_pair("AUTH", auth));
	m_connection_commands.insert(std::make_pair("CAP", cap));
	m_connection_commands.insert(std::make_pair("NICK", nick));
	m_connection_commands.insert(std::make_pair("PASS", pass));
	m_connection_commands.insert(std::make_pair("USER", user));

	// connection commands
	m_commands.insert(std::make_pair("OPER", oper));
	m_commands.insert(std::make_pair("PING", ping));
	m_commands.insert(std::make_pair("PONG", pong));
	m_commands.insert(std::make_pair("QUIT", quit));
	m_commands.insert(std::make_pair("NICK", nick));

	// channel commands
	m_commands.insert(std::make_pair("JOIN", join));
	m_commands.insert(std::make_pair("MODE", mode));
	m_commands.insert(std::make_pair("PRIVMSG", privmsg));
}

bool Server::update()
{
	poll_events();
	accept_new_connections();
	handle_events();
	handle_messages();
	disconnect_users();
	return true;
}

void Server::accept_new_connections()
{
	if ((m_pollfds[0].revents & POLLIN) == 0)
		return ;

	struct sockaddr_in client = {};
	socklen_t len = sizeof(client);

	int new_client_socket_fd = accept(m_server_socket, reinterpret_cast<sockaddr *>(&client), &len);
	if (new_client_socket_fd <= 0)
		CORE_ERROR("accept: %s", strerror(errno));
	CORE_INFO("Incomming connexion from : %s:%u", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	if (fcntl(new_client_socket_fd, F_SETFL, O_NONBLOCK) < 0) {
		CORE_ERROR("fcntl: %s", strerror(errno));
		close(new_client_socket_fd);
		return ;
	}

	pollfd pollfds = {};
	pollfds.fd = new_client_socket_fd;
	pollfds.events = POLLIN | POLLOUT;
	pollfds.revents = 0;

	m_pollfds.push_back(pollfds);
	m_users.push_back(User(new_client_socket_fd, inet_ntoa(client.sin_addr), ntohs(client.sin_port)));
}

void Server::poll_events()
{
	int poll_count = poll(m_pollfds.data(), (nfds_t)m_pollfds.size(), m_timeout);
	if (poll_count < 0 && errno != EINTR) {
		CORE_ERROR("poll: %s", strerror(errno));
	}

	size_t i = 1; // skip server socket
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++, i++) {
		user->set_is_readable(m_pollfds[i].revents & POLLIN);
		user->set_is_writable(m_pollfds[i].revents & POLLOUT);
	}
}

void Server::handle_events()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		if (user->is_readable()) {
			if (user->receive_message() <= 0)
				user->disconnect();
		}
		if (user->is_writable()) {
			if (!user->write_buffer().empty() && user->send_message() <= 0)
				user->disconnect();
		}
	}
}

void Server::handle_messages()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++)
	{
		if (!user->has_pending_command() || user->is_disconnected())
			continue ;

		std::string	command_str = user->get_next_command_str();
		Command		command(command_str);

		command.print();
		if (command.is_valid())
			execute_command(*user, command);
	}
}

void Server::execute_command(User &user, const Command &cmd)
{
	CommandIterator it;
	if (!user.is_registered()) {
		it = m_connection_commands.find(cmd.get_command());
		if (it != m_connection_commands.end()) {
			if (user.need_password() && cmd.get_command() == "NICK") {
				user.check_password();
				if (user.need_password()) {
					reply(user, ":Access denied, need password"); //TODO: get the real error
					user.disconnect();
					return;
				}
			}
			it->second(user, cmd);
		}
		else if (m_commands.find(cmd.get_command()) != m_commands.end())
			reply(user, ERR_NOTREGISTERED(user));
		else
			reply(user, ERR_UNKNOWNCOMMAND(cmd.get_command()));
		return ;
	}

	it = m_commands.find(cmd.get_command());
	if (it != m_commands.end())
		it->second(user, cmd);
	else if (m_connection_commands.find(cmd.get_command()) != m_connection_commands.end())
		reply(user, ERR_ALREADYREGISTERED(user));
	else
		reply(user, ERR_UNKNOWNCOMMAND(cmd.get_command()));
}

void Server::reply(User& user, const std::string &msg)
{
	CORE_TRACE("REPLYING TO %s:%d [%s]", user.ip().c_str(), user.port(), msg.c_str());
	user.update_write_buffer(msg + "\r\n");
}

void Server::broadcast(const std::string &msg)
{
	CORE_TRACE("BROADCASTING [%s]", msg.c_str());
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++)
		reply(*user, msg);
}

void Server::broadcast(User& user, const std::string &msg)
{
	CORE_TRACE("BROADCASTING [%s]", msg.c_str());
	for (UserIterator it = m_users.begin(); it != m_users.end(); it++)
	{
		if (*it != user)
			reply(*it, msg);
	}
}

void Server::broadcast_to_channel(User& user, Channel& channel, const std::string& msg)
{
	CORE_TRACE("BROADCASTING [%s] TO %s from %s", msg.c_str(), channel.name().c_str(), user.nickname().c_str());
	for (UserIterator it = m_users.begin(); it != m_users.end(); it++)
	{
		if (channel.has_user(*it) && *it != user)
			reply(*it, msg);
	}
}

bool Server::initialize_config_file()
{
	std::fstream banlist_file("banlist.txt", std::ios::in | std::ios::out | std::ios::app);
	if (!banlist_file.is_open()) {
		CORE_ERROR("banlist.txt: %s", strerror(errno));
		return false;
	}

	std::fstream whitelist_file("whitelist.txt", std::ios::in | std::ios::out | std::ios::app);
	if (!whitelist_file.is_open()) {
		CORE_ERROR("whitelist.txt: %s", strerror(errno));
		return false;
	}

	return true;
}

void Server::disconnect_users()
{
	for (size_t i = 0; i < m_users.size(); ++i)
	{
		if (m_users[i].is_disconnected()) {
			CORE_INFO("%s disconnected", m_users[i].nickname().c_str());
			m_pollfds.erase(m_pollfds.begin() + (long)i);
			close(m_users[i].fd());
			m_users.erase(m_users.begin() + (long)i);
		}
	}
}

void Server::shutdown()
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++)
		close(user->fd());
	close(m_server_socket);
	CORE_INFO("Server shutdown");
}

bool Server::is_nickname_taken(const std::string &nickname)
{
	UserIterator user = find_user(nickname);
	if (user_exists(user)) {
		CORE_DEBUG("Nickname %s already exists", nickname.c_str());
		return true;
	}
	return false;
}

void Server::reply_welcome_user(User &user)
{
	reply(user, RPL_WELCOME(user));
	reply(user, RPL_YOURHOST(user));
	reply(user, RPL_CREATED(user));
	reply(user, RPL_MYINFO(user));
}

void Server::reply_list_channel_members_to_user(User &user, const Channel& channel)
{
	for (Channel::ConstUserIterator channel_user = channel.users().begin(); channel_user != channel.users().end(); channel_user++)
		reply(user, RPL_NAMREPLY(user, channel, (*channel_user)));
	reply(user, RPL_ENDOFNAMES(user, channel));
}

bool Server::user_exists(const Server::UserIterator &user)
{
	return user != m_users.end();
}

bool Server::user_exists(const std::string &user_nickname)
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		if (user->nickname() == user_nickname)
			return true;
	}
	return false;
}

Server::UserIterator Server::find_user(const std::string &user_nickname)
{
	for (UserIterator user = m_users.begin(); user != m_users.end(); user++) {
		if (user->nickname() == user_nickname)
			return user;
	}
	return m_users.end();
}

bool Server::channel_exists(const Server::ChannelIterator &channel)
{
	return channel != m_channels.end();
}

bool Server::channel_exists(const std::string &channel_name)
{
	for (ChannelIterator channel = m_channels.begin(); channel != m_channels.end(); channel++) {
		if (channel->name() == channel_name)
			return true;
	}
	return false;
}

Server::ChannelIterator Server::find_channel(const std::string &channel_name)
{
	for (ChannelIterator channel = m_channels.begin(); channel != m_channels.end(); channel++) {
		if (channel->name() == channel_name)
			return channel;
	}
	return m_channels.end();
}

void Server::disconnect_user_from_channel(User &user, const std::string &channel_name)
{
	ChannelIterator channel = find_channel(channel_name);
	if (channel != m_channels.end())
		disconnect_user_from_channel(user, *channel);
}

void Server::disconnect_user_from_channel(User &user, Channel &channel)
{
	// Remove the user from the channel
	channel.remove_user(user);
	reply(user, SOURCE("PART", user) + " " + channel.name());

	// Notify other channel users
	for (Channel::ConstUserIterator channel_user = channel.users().begin(); channel_user != channel.users().end(); channel_user++) {
		UserIterator server_user = find_user(channel_user->nickname());
		if (server_user != m_users.end())
			reply(*server_user, SOURCE("PART", user) + " " + channel.name());
	}
}
