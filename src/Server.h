//
// Created by nathan on 2/16/23.
//

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <netdb.h>
#include <map>
#include <string>
#include "User.h"
#include "Channel.h"
#include "Command.h"

#define SERVER_VERSION "ft_irc v0.5"

class Server
{
public:
	/// I/O typedefs
	typedef int (*command_function)(User&, const Command&);
	typedef std::map<std::string, command_function>::iterator	CommandIterator;

	typedef std::vector<User*>				UserVector;
	typedef UserVector::iterator			UserIterator;
	typedef std::map<std::string, Channel*>	ChannelMap;
	typedef ChannelMap::iterator			ChannelIterator;
	typedef ChannelMap::const_iterator		ConstChannelIterator;

	/// Server management
	static bool initialize(uint16_t port);
	static bool update();
	static void shutdown();
	static void signal_handler(int signal);

	/// Replies
	static void broadcast(const std::string& msg);
	static void broadcast(User& user_to_avoid, const std::string &msg);
	static void broadcast_to_channel(Channel& channel, const std::string& msg);
	static void broadcast_to_channel(User& user_to_avoid, Channel& channel, const std::string& msg);
	static void reply(User& user, const std::string& msg);
	static void reply_welcome_user(User& user);
	static void try_reply_list_channel_members_to_user(User& user, const std::string& channel_name);
	static void reply_list_channel_members_to_user(User &user, const Channel& channel);
	static void reply_ban_list_to_user(User& user, const Channel& channel);
	static void reply_channel_list_to_user(User& user);

	/// User management
	static void				try_disconnect_user_from_channel(User& user, const std::string& channel_name, const std::string& reason = "");
	static void				disconnect_user_from_channel(User& user, Channel& channel, const std::string& reason = "");
	static void				disconnect_user_from_channels(User& user, const std::string& reason = "");
	static bool				user_exists(const std::string& user_nickname);
	static bool				user_exists(const UserIterator& user);
	static UserIterator		find_user(const std::string& user_nickname);
	static bool				is_nickname_taken(const std::string& user_nickname);

	/// Channel management
	static Channel&			create_new_channel(User& first_user, const std::string& channel_name);
	static void				remove_channel(Channel& channel);
	static bool				channel_exists(const std::string& channel_name);
	static bool				channel_exists(const ChannelIterator& channel);
	static ChannelIterator	find_channel(const std::string& channel_name);

	/// Server information
	static const std::string&	network_name()			{ return m_network_name; }
	static const std::string&	server_name()			{ return m_server_name; }
	static       bool			is_running()			{ return m_is_running; }
	static const std::string&	creation_date()			{ return m_creation_date; }
	static const std::string&	password()				{ return m_password; }
	static       std::string	supported_tokens(User& user);

	/// User & Channels
	static       ChannelMap&	channels()				{ return m_channels; }
	static       UserVector&	users()					{ return m_users; }
	static const std::string&	user_modes()			{ return m_user_modes; }
	static const std::string&	channel_modes()			{ return m_channel_modes; }
	static const std::string&	channel_modes_param()	{ return m_channel_modes_parameter; }
	static const std::string&	motd_string()			{ return m_motd; }

	/// Setters
	static void	set_is_running(bool new_state)					{ m_is_running = new_state; }
	static void	set_server_name(const std::string& server_name)	{ m_server_name = server_name; }
	static void set_password(const std::string& password)		{ m_password = password; }

private:
	// Member functions
	static void	initialize_command_functions();
	static bool	get_server_motd(const std::string& path);
	static bool	initialize_config_file();
	static void	accept_new_connections();
	static void	poll_events();
	static void	handle_events();
	static void	handle_messages();
	static void	execute_command(User& user, const Command& command);
	static void	check_for_closed_connexions();

	/// Users
	static User&				create_new_user(int fd, const std::string& ip, uint16_t port);
	static void					remove_user(User& user);

	// Member variables
	static std::string			m_network_name;
	static std::string			m_server_name;
	static int					m_server_socket;
	static std::string			m_password;

	static std::vector<pollfd>	m_pollfds;
	static UserVector			m_users;
	static ChannelMap			m_channels;

	static bool					m_is_running;
	static bool					m_is_readonly;

	static const int			m_server_backlog;
	static const int			m_timeout;
	static const std::string	m_creation_date;
	static const std::string	m_user_modes;
	static const std::string	m_channel_modes;
	static const std::string	m_channel_modes_parameter;

	// Message function prototype
	static std::map<std::string, command_function>	m_commands;
	static std::map<std::string, command_function>	m_connection_commands;

	static std::string			m_motd;
};

inline Channel&			get_channel_reference(const Server::ChannelIterator& channel_it)		{ return *(channel_it->second); }
inline const Channel&	get_channel_reference(const Server::ConstChannelIterator& channel_it)	{ return *(channel_it->second); }
inline User&			get_user_reference(const Server::UserIterator& user_it)					{ return *(*user_it); }
inline User&			get_user_reference(User *user_ptr)										{ return *user_ptr; }

#endif //SERVER_H
