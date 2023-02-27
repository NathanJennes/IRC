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

#define SERVER_VERSION "0.1"

class Server
{
public:
	static bool initialize(uint16_t port);
	static bool update();
	static void shutdown();

	static void signal_handler(int signal);
	static void reply(User& user, const std::string& msg);
	static void welcome(User& user);

	// getters
	static const std::string&	network_name()			{ return m_network_name; }
	static const std::string&	server_name()			{ return m_server_name; }
	static bool 				is_running()			{ return m_is_running; }
	static const std::string&	creation_date()			{ return m_creation_date; }
	static const std::string&	user_modes()			{ return m_user_modes; }
	static const std::string&	channel_modes()			{ return m_channel_modes; }
	static const std::string&	channel_modes_param()	{ return m_channel_modes_parameter; }

	// setters
	static void	stop_server()									{ m_is_running = false; }
	static void	set_server_name(const std::string& server_name)	{ m_server_name = server_name; }

private:
	// Member functions
	static bool	initialize_config_file();
	static void	accept_new_connections();
	static void	poll_events();
	static void	handle_events();
	static void	handle_messages();
	static void	execute_command(User& user, const Command& command);
	static void	disconnect_users();

	static void	initialize_command_functions();

	// Member variables
	static std::string			m_network_name;
	static std::string			m_server_name;
	static int					m_server_socket;

	static std::vector<User>	m_users;
	static std::vector<Channel>	m_channels;

	static bool					m_is_running;
	static bool					m_is_readonly;

	static const int			m_server_backlog;
	static const int			m_timeout;
	static const std::string	m_creation_date;
	static const std::string	m_user_modes;
	static const std::string	m_channel_modes;
	static const std::string	m_channel_modes_parameter;

	typedef int (*command_function)(User&, const Command&);
	typedef std::vector<User>::iterator							UserIterator;
	typedef std::map<std::string, command_function>::iterator	CommandIterator;

	static std::map<std::string, command_function>	m_commands;
};

#endif //SERVER_H
