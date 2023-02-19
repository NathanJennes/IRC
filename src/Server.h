//
// Created by nathan on 2/16/23.
//

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <netdb.h>
#include "User.h"
#include "Channel.h"

class Server
{
public:
	bool initialize(uint16_t port);
	bool update();

	bool is_running()	const { return m_is_running; }

	// getters
	const std::string&	server_name()	const { return m_server_name; }

	// setters
	void set_server_name(const std::string& server_name) { m_server_name = server_name; }

private:
	// Member functions
	bool initialize_config_file();
	void accept_new_connections();
	void poll_events();
	void handle_events();
	void handle_messages();
	void execute_command(User& user);

	// Member variables
	std::string				m_server_name;
	bool					m_is_running;
	bool 					m_is_readonly;
	int						m_server_socket;

	std::vector<User>		m_users;
	std::vector<Channel>	m_channels;

	static const int		m_server_backlog;
	static const int		m_timeout;

	typedef std::vector<User>::iterator	UserIterator;
};

#endif //SERVER_H
