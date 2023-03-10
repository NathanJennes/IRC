//
// Created by nathan on 2/16/23.
//

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include "Server.h"
#include "log.h"
#include "Numerics.h"
#include "Message.h"

const int			Server::m_server_backlog = 10;
const int			Server::m_timeout = 20;

ServerInfo			Server::m_server_info;
int					Server::m_server_socket;
bool				Server::m_is_running = true;
std::string			Server::m_password;

std::vector<pollfd>	Server::m_pollfds;

Server::UserVector								Server::m_users;
Server::ChannelMap								Server::m_channels;
std::map<std::string, Server::command_function>	Server::m_commands;
std::map<std::string, Server::command_function>	Server::m_connection_commands;

void Server::signal_handler(int signal)
{
	(void) signal;
	m_is_running = false;
}

bool Server::initialize(uint16_t port)
{
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

	// setup server pollfd
	pollfd m_server_pollfd = {};
	m_server_pollfd.fd = m_server_socket;
	m_server_pollfd.events = POLLIN;
	m_server_pollfd.revents = 0;
	m_pollfds.push_back(m_server_pollfd);

	initialize_command_functions();
	m_server_info.initialize();

	m_is_running = true;

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
	m_commands.insert(std::make_pair("PART", part));
	m_commands.insert(std::make_pair("TOPIC", topic));
	m_commands.insert(std::make_pair("NAMES", names));
	m_commands.insert(std::make_pair("LIST", list));
	m_commands.insert(std::make_pair("KICK", kick));
	m_commands.insert(std::make_pair("MODE", mode));
	m_commands.insert(std::make_pair("PRIVMSG", privmsg));
	m_commands.insert(std::make_pair("INVITE", invite));

	// server commands
	m_commands.insert(std::make_pair("ADMIN", admin));
	m_commands.insert(std::make_pair("MODE", mode));
	m_commands.insert(std::make_pair("MOTD", motd));
	m_commands.insert(std::make_pair("VERSION", version));

	// user commands
	m_commands.insert(std::make_pair("WHO", who));
}

bool Server::update()
{
	poll_events();
	accept_new_connections();
	handle_events();
	handle_messages();
	check_for_closed_connexions();
	check_for_empty_channels();
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
	create_new_user(new_client_socket_fd, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
}

void Server::poll_events()
{
	int poll_count = poll(m_pollfds.data(), (nfds_t)m_pollfds.size(), m_timeout);
	if (poll_count < 0 && errno != EINTR) {
		CORE_ERROR("poll: %s", strerror(errno));
	}

	size_t i = 1; // skip server socket
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++, i++) {
		User& user = get_user_reference(user_it);
		user.set_is_readable(m_pollfds[i].revents & POLLIN);
		user.set_is_writable(m_pollfds[i].revents & POLLOUT);
	}
}

void Server::handle_events()
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user.is_readable()) {
			if (user.receive_message() <= 0)
				user.disconnect();
		}
		if (user.is_writable()) {
			if (!user.write_buffer().empty() && user.send_message() <= 0)
				user.disconnect();
		}
	}
}

void Server::handle_messages()
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);

		if (!user.has_pending_command() || user.is_disconnected())
			continue ;

		std::string	command_str = user.get_next_command_str();
		Command		command(command_str);

		command.print();
		if (command.is_valid())
			execute_command(user, command);
	}
}

void Server::execute_command(User &user, const Command &cmd)
{
	CommandIterator command_it;
	std::string command_name = to_upper(cmd.get_command());

	if (!user.is_registered()) {
		command_it = m_connection_commands.find(command_name);
		if (command_it != m_connection_commands.end()) {
			if (user.need_password() && cmd.get_command() == "NICK") {
				user.check_password();
				if (user.need_password()) {
					reply(user, ":Access denied, need password"); //TODO: get the real error
					user.disconnect();
					return;
				}
			}
			command_it->second(user, cmd);
		}
		else if (m_commands.find(command_name) != m_commands.end())
			reply(user, ERR_NOTREGISTERED(user));
		else
			reply(user, ERR_UNKNOWNCOMMAND(user, command_name));
		return ;
	}

	command_it = m_commands.find(command_name);
	if (command_it != m_commands.end())
		command_it->second(user, cmd);
	else if (m_connection_commands.find(command_name) != m_connection_commands.end())
		reply(user, ERR_ALREADYREGISTERED(user));
	else
		reply(user, ERR_UNKNOWNCOMMAND(user, command_name));
}

void Server::reply(User& user, const std::string &msg)
{
	CORE_TRACE("REPLYING TO %s:%d [%s]", user.ip().c_str(), user.port(), msg.c_str());
	user.update_write_buffer(msg + "\r\n");
}

void Server::broadcast(const std::string &msg)
{
	CORE_TRACE("BROADCASTING [%s]", msg.c_str());
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++)
		reply(get_user_reference(user_it), msg);
}

void Server::broadcast(User& user_to_avoid, const std::string &msg)
{
	CORE_TRACE("BROADCASTING [%s]", msg.c_str());
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user != user_to_avoid)
			reply(user, msg);
	}
}

void Server::broadcast_to_channel(Channel& channel, const std::string& msg)
{
	CORE_TRACE("BROADCASTING [%s] TO %s from %s", msg.c_str(), channel.name().c_str());
	for (Channel::UserIterator user_it = channel.users().begin(); user_it != channel.users().end(); user_it++)
		reply(get_user_reference(user_it), msg);
}

void Server::broadcast_to_channel(User& user_to_avoid, Channel& channel, const std::string& msg)
{
	CORE_TRACE("BROADCASTING [%s] TO %s from %s", msg.c_str(), channel.name().c_str(), user_to_avoid.nickname().c_str());
	for (Channel::UserIterator user_it = channel.users().begin(); user_it != channel.users().end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user != user_to_avoid)
			reply(user, msg);
	}
}

void Server::check_for_closed_connexions()
{
	for (size_t i = 0; i < m_users.size(); i++) {
		User& user = get_user_reference(m_users[i]);
		if (user.is_disconnected()) {
			CORE_INFO("%s disconnected", user.nickname().c_str());
			m_pollfds.erase(m_pollfds.begin() + static_cast<long>(i + 1));
			close(user.fd());
			remove_user(user);
		}
	}
}

void Server::check_for_empty_channels()
{
	for (ChannelIterator channel_it = m_channels.begin(), next_it = channel_it; channel_it != m_channels.end(); channel_it = next_it) {
		next_it++;
		if (get_channel_reference(channel_it).user_count() == 0)
			m_channels.erase(channel_it);
	}
}

void Server::shutdown()
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		close(user.fd());
		delete &user;
	}

	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end(); channel_it++)
		delete channel_it->second;

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
	reply(user, RPL_ISUPPORT(user));
	motd(user, Command(""));
}

void Server::try_reply_list_channel_members_to_user(User &user, const std::string &channel_name)
{
	ChannelIterator channel_it = find_channel(channel_name);
	if (!channel_exists(channel_it)) {
		reply(user, RPL_ENDOFNAMES(user, channel_name));
		return ;
	}

	Channel& channel = get_channel_reference(channel_it);
	if (channel.is_secret() && !channel.has_user(user)) {
		reply(user, RPL_ENDOFNAMES(user, channel_name));
		return ;
	}

	reply_list_channel_members_to_user(user, channel);
}

void Server::reply_list_channel_members_to_user(User &user, const Channel& channel)
{
	for (Channel::ConstUserIterator channel_user_it = channel.users().begin(); channel_user_it != channel.users().end(); channel_user_it++) {
		reply(user, RPL_NAMREPLY(user, channel, (*channel_user_it->first), channel_user_it->second));
	}
	reply(user, RPL_ENDOFNAMES(user, channel.name()));
}

bool Server::user_exists(const Server::UserIterator &user)
{
	return user != m_users.end();
}

bool Server::user_exists(const std::string &user_nickname)
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user.nickname() == user_nickname)
			return true;
	}
	return false;
}

Server::UserIterator Server::find_user(const std::string &user_nickname)
{
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user.nickname() == user_nickname)
			return user_it;
	}
	return m_users.end();
}

bool Server::channel_exists(const Server::ChannelIterator &channel)
{
	return channel != m_channels.end();
}

bool Server::channel_exists(const std::string &channel_name)
{
	ChannelIterator channel_it = m_channels.find(channel_name);
	if (channel_exists(channel_it))
		return true;
	return false;
}

Server::ChannelIterator Server::find_channel(const std::string &channel_name)
{
	return m_channels.find(channel_name);
}

void Server::try_reply_part_user_from_channel(User &user, const std::string &channel_name, const std::string& reason)
{
	ChannelIterator channel_it = find_channel(channel_name);
	if (!channel_exists(channel_it)) {
		reply(user, ERR_NOSUCHCHANNEL(user, channel_name));
		return ;
	}

	Channel& channel = get_channel_reference(channel_it);
	if (!channel.has_user(user)) {
		reply(user, ERR_NOTONCHANNEL(user, channel));
		return ;
	}

	reply_part_user_from_channel(user, channel, reason);
}

void Server::reply_part_user_from_channel(User &user, Channel &channel, const std::string& reason)
{
	// Remove the user from the channel
	channel.remove_user(user);
	user.remove_channel(channel);

	std::string separator;
	if (!reason.empty())
		separator += ' ';

	reply(user, USER_SOURCE("PART", user) + " " + channel.name() + separator + reason);

	// Notify other channel users
	for (Channel::UserIterator channel_user_it = channel.users().begin(); channel_user_it != channel.users().end(); channel_user_it++)
		reply(get_user_reference(channel_user_it), USER_SOURCE("PART", user) + " " + channel.name() + separator + reason);
}

void Server::reply_part_user_from_channels(User &user, const std::string& reason)
{
	for (User::ChannelIterator channel_it = user.channels().begin(); channel_it != user.channels().end(); channel_it++)
		reply_part_user_from_channel(user, get_channel_reference(channel_it), reason);
}

User& Server::create_new_user(int fd, const std::string &ip, uint16_t port)
{
	User *new_user = new User(fd, ip, port);
	m_users.push_back(new_user);
	return *new_user;
}

void Server::remove_user(User &user)
{
	UserIterator user_it = std::find(m_users.begin(), m_users.end(), &user);
	if (user_it == m_users.end()) {
		CORE_WARN("Trying to remove a user that doesn't belong to the server's users list");
		return ;
	}

	m_users.erase(user_it);
	delete &user;
}

Channel& Server::create_new_channel(User &first_user, const std::string &channel_name)
{
	Channel *new_channel = new Channel(first_user, channel_name);
	m_channels[channel_name] = new_channel;
	return *new_channel;
}

void Server::remove_channel(Channel &channel)
{
	ChannelIterator channel_it = m_channels.find(channel.name());
	if (channel_it == m_channels.end()) {
		CORE_WARN("Trying to remove a channel that doesn't belong to the server's channel list");
		return ;
	}

	m_channels.erase(channel_it);
	delete &channel;
}

void Server::reply_channel_list_to_user(User &user)
{
	Server::reply(user, RPL_LISTSTART(user));
	for (ConstChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end(); channel_it++) {
		const Channel& channel = get_channel_reference(channel_it);
		if (!channel.is_secret() || channel.has_user(user))
			Server::reply(user, RPL_LIST(user, channel));
	}
	Server::reply(user, RPL_LISTEND(user));
}

std::string Server::supported_tokens(User& user)
{
	std::string tokens;

	tokens += "AWAYLEN= ";
	tokens += "CASEMAPPING= ";
	tokens += "CHANLIMIT= ";
	tokens += "CHANMODES=beI,,kl,mnst ";
	tokens += "CHANNELLEN= ";
	tokens += "CHANTYPES=#& ";
	tokens += "ELIST=MNUCT ";
	tokens += "EXCEPTS ";
	tokens += "EXTBAN=,";
	tokens += "HOSTLEN= ";
	tokens += "INVEX ";
	tokens += "KICKLEN= ";
	tokens += "MAXLIST=beI ";

	Server::reply(user, RPL_MESSAGE(user, "005", tokens + ":are supported by this server"));

	// =========================

	tokens.clear();

	tokens += "MAXTARGETS= ";
	tokens += "MODES=9 ";
	tokens += "NETWORK=FT_IRC";
	tokens += "NICKLEN= ";
	tokens += "PREFIX=~&@%+ ";
//	tokens += "SAFELIST= ";
	tokens += "SILENCE= ";
	tokens += "STATUSMSG=@+ ";
	tokens += "TARGMAX= ";
	tokens += "TOPICLEN= ";
	tokens += "USERLEN= ";

	Server::reply(user, RPL_MESSAGE(user, "005", tokens + ":are supported by this server"));

	return tokens;
}

void Server::reply_channel_ban_list_to_user(User &user, const Channel &channel)
{
	CORE_TRACE("Channel ban list size %d", channel.ban_list().size());
	for (size_t i = 0; i < channel.ban_list().size(); i++)
		Server::reply(user, RPL_BANLIST(user, channel, channel.ban_list()[i]));
	Server::reply(user, RPL_ENDOFBANLIST(user, channel));
}

void Server::reply_channel_ban_exempt_list_to_user(User &user, const Channel &channel)
{
	for (size_t i = 0; i < channel.ban_exemptions().size(); i++)
		Server::reply(user, RPL_EXCEPTLIST(user, channel, channel.ban_exemptions()[i]));
	Server::reply(user, RPL_ENDOFEXCEPTLIST(user, channel));
}

void Server::reply_channel_invite_exempt_list_to_user(User &user, const Channel &channel)
{
	CORE_DEBUG("Channel invite exempt list size %d", channel.invite_exemptions().size());
	for (size_t i = 0; i < channel.invite_exemptions().size(); i++)
		Server::reply(user, RPL_INVEXLIST(user, channel, channel.invite_exemptions()[i]));
	Server::reply(user, RPL_ENDOFINVEXLIST(user, channel));
}

void Server::reply_list_of_channel_invite_to_user(User &user)
{
	Server::ConstChannelIterator channel_it = m_channels.begin();
	for (; channel_it != m_channels.end(); channel_it++)
	{
		const Channel& channel = get_channel_reference(channel_it);
		for (size_t i = 0; i < channel.invite_list().size(); ++i) {
			if (channel.invite_list().at(i) == user.nickname()) {
				Server::reply(user, RPL_INVITELIST(user, channel.name()));
				break ;
			}
		}
	}
	Server::reply(user, RPL_ENDOFINVITELIST(user, channel_it->first));
}
