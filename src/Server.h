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

class Server
{
public:
	bool initialize(uint16_t port);
	bool update();
	void cleanup();
	void send_to_client(const User& user, const std::string& message);

	static void signal_handler(int signal);

	// getters
	const std::string&	server_name()	const { return m_server_name; }
	bool 				is_running()	const { return m_is_running; }

	// setters
	void stop_server() 										{ m_is_running = false; }
	void set_server_name(const std::string& server_name)	{ m_server_name = server_name; }

private:
	// Member functions
	bool initialize_config_file();
	void accept_new_connections();
	void poll_events();
	void handle_events();
	void handle_messages();
	void execute_command(User& user, const Command& command);
	void disconnect_users();

	void initialize_command_functions();

	// Member variables
	std::string				m_server_name;
	static bool				m_is_running;
	bool 					m_is_readonly;
	int						m_server_socket;

	std::vector<User>		m_users;
	std::vector<Channel>	m_channels;

	static const int		m_server_backlog;
	static const int		m_timeout;

	typedef int (*function)(User&, const Command&);

	std::map<std::string, function>	m_commands;

	typedef std::vector<User>::iterator					UserIterator;
	typedef std::map<std::string, function>::iterator	CommandIterator;
};

#endif //SERVER_H
