//
// Created by nathan on 2/16/23.
//

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include "Server.h"
#include "log.h"
#include "Numerics.h"
#include "Message.h"
#include "ParamSplitter.h"

const int			Server::m_server_backlog = 10;
const int			Server::m_timeout = 20;

ServerInfo			Server::m_server_info;
int					Server::m_server_socket;
bool				Server::m_is_running = true;
std::time_t			Server::m_start_timestamp;
std::string			Server::m_password;

std::string			Server::m_oper_password;
std::string			Server::m_oper_username;
std::string			Server::m_oper_host;

std::vector<pollfd>	Server::m_pollfds;

size_t				Server::m_unknown_connections = 0;

Server::UserVector								Server::m_users;
Server::OldUserVector							Server::m_old_users;
Server::ChannelMap								Server::m_channels;
std::map<std::string, Server::command_function>	Server::m_commands;
std::map<std::string, std::size_t>				Server::m_command_stats;

const std::size_t	Server::m_awaylen = 50;
const std::size_t	Server::m_chan_name_len = 50;
const std::size_t	Server::m_kicklen = 120;
const std::size_t	Server::m_max_lists_entries = 20;
const std::size_t	Server::m_nicklen = 9;
const std::size_t	Server::m_userlen = 10;
const std::size_t	Server::m_topiclen = 80;

std::map<std::string, Server::command_function>	Server::m_connection_commands;

OldUserInfo::OldUserInfo(std::time_t time, const User &user)
	: m_last_time_seen(time), m_nickname(user.nickname()),
	m_username(user.username()), m_realname(user.realname()), m_host(user.ip())
{
}

bool OldUserInfo::operator==(const User &user) const
{
	return to_upper(m_nickname) == user.nickname_upper() && m_username == user.username() && m_realname == user.realname() && m_host == user.ip();
}

bool OldUserInfo::operator==(const OldUserInfo &user) const
{
	return to_upper(m_nickname) == to_upper(user.nickname()) && m_username == user.username() && m_realname == user.realname() && m_host == user.host();
}

void Server::signal_handler(int signal)
{
	(void) signal;
	m_is_running = false;
}

bool Server::initialize(uint16_t port)
{
	if (!initialize_operator_credential()) {
		CORE_ERROR("Couldn't initialize operator credential");
		return false;
	}

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
	load_old_user_list_from_file();
	m_server_info.initialize();

	m_is_running = true;
	m_start_timestamp = time(NULL);

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
	m_commands.insert(std::make_pair("NOTICE", notice));
	m_commands.insert(std::make_pair("INVITE", invite));

	// server commands
	m_commands.insert(std::make_pair("ADMIN", admin));
	m_commands.insert(std::make_pair("MODE", mode));
	m_commands.insert(std::make_pair("MOTD", motd));
	m_commands.insert(std::make_pair("VERSION", version));
	m_commands.insert(std::make_pair("TIME", time_cmd));
	m_commands.insert(std::make_pair("INFO", info_cmd));
	m_commands.insert(std::make_pair("LUSERS", lusers));
	m_commands.insert(std::make_pair("STATS", stats));
	m_commands.insert(std::make_pair("KILL", kill));

	// User queries
	m_commands.insert(std::make_pair("WHO", who));
	m_commands.insert(std::make_pair("WHOWAS", whowas));
	m_commands.insert(std::make_pair("WHOIS", whois));

	// Optional commands
	m_commands.insert(std::make_pair("AWAY", away));
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

	m_unknown_connections++;

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
			if (!user.receive_message())
				user.disconnect();
		}
		if (user.is_writable()) {
			if (!user.write_buffer().empty() && !user.send_message())
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
					Server::reply(user, ERR_PASSWDMISMATCH(user));
					user.disconnect();
					return;
				}
			}
			m_command_stats[command_name]++;
			command_it->second(user, cmd);
		}
		else if (m_commands.find(command_name) != m_commands.end())
			reply(user, ERR_NOTREGISTERED(user));
		else
			reply(user, ERR_UNKNOWNCOMMAND(user, command_name));
		return ;
	}

	command_it = m_commands.find(command_name);
	if (command_it != m_commands.end()) {
		m_command_stats[command_name]++;
		command_it->second(user, cmd);
	} else if (m_connection_commands.find(command_name) != m_connection_commands.end())
		reply(user, ERR_ALREADYREGISTERED(user));
	else
		reply(user, ERR_UNKNOWNCOMMAND(user, command_name));
}

void Server::reply(User& user, const std::string &msg)
{
	CORE_TRACE("REPLYING TO %s:%d [%s]", user.ip().c_str(), user.port(), msg.c_str());
	user.queue_command_for_sending(msg + "\r\n");
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
	CORE_TRACE("BROADCASTING [%s] TO %s", msg.c_str(), channel.name().c_str());
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
	store_user_list_to_file();

	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		close(user.fd());
		delete &user;
	}
	close(m_server_socket);

	for (ChannelIterator channel_it = m_channels.begin(); channel_it != m_channels.end(); channel_it++)
		delete channel_it->second;

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
	RPL_ISUPPORT(user);
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
	std::string user_nickname_to_upper = to_upper(user_nickname);
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user.nickname_upper() == user_nickname_to_upper)
			return true;
	}
	return false;
}

Server::UserIterator Server::find_user(const std::string &user_nickname)
{
	std::string user_nickname_to_upper = to_upper(user_nickname);
	for (UserIterator user_it = m_users.begin(); user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		if (user.nickname_upper() == user_nickname_to_upper)
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
	ChannelIterator channel_it = m_channels.find(to_upper(channel_name));
	if (channel_exists(channel_it))
		return true;
	return false;
}

Server::ChannelIterator Server::find_channel(const std::string &channel_name)
{
	return m_channels.find(to_upper(channel_name));
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

	for (User::ChannelIterator channel_it = user.channels().begin(); channel_it != user.channels().end(); channel_it++) {
		Channel& channel = get_channel_reference(channel_it);
		channel.remove_user(user);
	}

	add_to_old_users_list(user);

	m_users.erase(user_it);
	delete &user;
}

Channel& Server::create_new_channel(User &first_user, const std::string &channel_name)
{
	Channel *new_channel = new Channel(first_user, channel_name);
	m_channels[to_upper(channel_name)] = new_channel;
	return *new_channel;
}

void Server::remove_channel(Channel &channel)
{
	ChannelIterator channel_it = m_channels.find(channel.name_to_upper());
	if (channel_it == m_channels.end()) {
		CORE_WARN("Trying to remove a channel that doesn't belong to the server's channel list");
		return ;
	}

	for (Channel::UserIterator user_it = channel.users().begin(); user_it != channel.users().end(); user_it++) {
		User& user = get_user_reference(user_it);
		user.remove_channel(channel);
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

	tokens += "AWAYLEN=" + to_string(m_awaylen) + " ";
	tokens += "CASEMAPPING=ascii ";
	tokens += "CHANLIMIT=&#: "; // Unlimited if no value.
	tokens += "CHANMODES=beI,,kl,mnst ";
	tokens += "CHANNELLEN=" + to_string(m_chan_name_len) + " ";
	tokens += "CHANTYPES=#& ";
	tokens += "ELIST=UCT ";
	tokens += "EXCEPTS=e ";
//	tokens += "EXTBAN= "; // Optional
	tokens += "INVEX=I ";
	tokens += "KICKLEN=" + to_string(m_kicklen) + " ";
	tokens += "MAXLIST=beI:" + to_string(m_max_lists_entries) + " "; //TODO: check for maxlist addding users to the lists

	Server::reply(user, RPL_MESSAGE(user, "005", tokens + ":are supported by this server"));

	// =========================

	tokens.clear();

//	tokens += "MAXTARGETS= "; // Optional
//	tokens += "MODES=9 "; // optional
	tokens += "NETWORK=GigaChat ";
	tokens += "NICKLEN=" + to_string(m_nicklen) + " "; //TODO: check for nicknames bigger than 30 characters
	tokens += "PREFIX=(ov)@%+ ";
//	tokens += "SAFELIST= "; // Maybe ??
//	tokens += "SILENCE= "; // Optional - SILENCE command not implemented
	tokens += "STATUSMSG= ";
//	tokens += "TARGMAX= "; // optional
	tokens += "TOPICLEN=" + to_string(m_topiclen) + " "; //TODO: check for topic length when receiving TOPIC command
	tokens += "USERLEN=" + to_string(m_userlen) + " "; //TODO: check for usernames lenght when receiving USER command

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

void Server::add_to_old_users_list(User &user)
{
	// If the user disconnecting was already known by the server,
	//  delete its old entry (i.e. just update it)
	OldUserIterator entry = std::find(m_old_users.begin(), m_old_users.end(), user);
	if (entry != m_old_users.end())
		m_old_users.erase(entry);

	m_old_users.push_back(OldUserInfo(time(NULL), user));
	if (m_old_users.size() > 5000)
		m_old_users.erase(m_old_users.begin());
}

Server::OldUserIterator Server::find_old_user(const std::string &user_nickname, OldUserIterator start)
{
	if (m_old_users.empty())
		return m_old_users.end();

	if (start == m_old_users.begin())
		return m_old_users.end();

	std::string user_nickname_to_upper = to_upper(user_nickname);

	for (OldUserIterator user_it = start - 1; user_it != m_old_users.begin(); user_it--) {
		if (to_upper(user_it->nickname()) == user_nickname_to_upper)
			return user_it;
	}

	if (to_upper(m_old_users.begin()->nickname()) == user_nickname_to_upper)
		return m_old_users.begin();

	return m_old_users.end();
}

void Server::store_user_list_to_file()
{
	std::ofstream file("config/user_list.csv", std::ios::trunc);

	for (OldUserIterator user_it = m_old_users.begin(); file.good() && user_it != m_old_users.end(); user_it++) {
		file << user_it->nickname() << ";" << user_it->username() << ";" << user_it->realname();
		file << ";" << user_it->host() << ";" << user_it->time_last_seen() << std::endl;
	}

	std::time_t now = time(NULL);
	for (UserIterator user_it = m_users.begin(); file.good() && user_it != m_users.end(); user_it++) {
		User& user = get_user_reference(user_it);
		file << user.nickname() << ";" << user.username() << ";" << user.realname();
		file << ";" << user.ip() << ";" << now << std::endl;
	}
}

void Server::load_old_user_list_from_file()
{
	std::ifstream file("config/user_list.csv", std::ios::in);

	if (file.bad())
		return ;

	std::string line;
	std::size_t line_number = 0;
	while (std::getline(file, line)) {
		ParamSplitter<';'> splitter(line);
		if (splitter.reached_end() || splitter.peek_next_param().empty())
			continue;

		OldUserInfo new_user;
		new_user.set_nickname(splitter.next_param());
		new_user.set_username(splitter.next_param());
		new_user.set_realname(splitter.next_param());
		new_user.set_host(splitter.next_param());
		new_user.set_time_last_seen(std::atol(splitter.next_param().c_str()));

		if (!User::is_nickname_valid(new_user.nickname()) ||
			!User::is_username_valid(new_user.username()) ||
			!User::is_host_valid(new_user.host()) ||
			new_user.time_last_seen() <= 0 ||
			std::find(m_old_users.begin(), m_old_users.end(), new_user) != m_old_users.end()) {
			CORE_WARN("Ignoring line %llu in config/user_list.svg: line corrupted", line_number);
		} else
			m_old_users.push_back(new_user);

		line_number++;
	}

	CORE_INFO("Loaded %llu users from config file", m_old_users.size());
}

void Server::register_user(User &user)
{
	reply_welcome_user(user);

	// If the user connecting was already known by the server,
	//  delete its old entry (i.e. just update it)
	OldUserIterator entry = std::find(m_old_users.begin(), m_old_users.end(), user);
	if (entry != m_old_users.end())
		m_old_users.erase(entry);

	if (users().size() > info().max_users())
		info().set_max_users(users().size());
}

void Server::change_user_nickname(User &user, const std::string &new_nickname)
{
	add_to_old_users_list(user);
	user.set_nickname(new_nickname);
}

bool Server::initialize_operator_credential()
{
	std::ifstream file("config/IRCd.config", std::ios::in);

	if (file.bad())
		return false;

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#')
			continue;

		ParamSplitter<'='> splitter(line);
		std::string param = splitter.next_param();
		if (param == "authorised_host") {
			m_oper_host = splitter.next_param();
			CORE_DEBUG("m_oper_host: %s", m_oper_host.c_str());
		} else if (param == "oper_name") {
			m_oper_username = splitter.next_param();
			CORE_DEBUG("m_oper_username: %s", m_oper_username.c_str());
		} else if (param == "oper_pass") {
			m_oper_password = splitter.next_param();
			CORE_DEBUG("m_oper_password: %s", m_oper_password.c_str());
		}
	}

	return true;
}
