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

private:
	// Member functions
	bool initialize_config_file();
	void accept_new_connections() const;
	void poll_events();
	void handle_events();
	void handle_messages();
	void execute_command(User& user);

	// Member variables
	bool					m_is_running;
	int						m_server_socket;

	std::vector<User>		m_users;
	std::vector<Channel>	m_channels;

	static const int		m_server_backlog;
	static const int		m_timeout;

	typedef std::vector<User>::iterator	UserIterator;
};

#endif //SERVER_H
