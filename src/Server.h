//
// Created by nathan on 2/16/23.
//

#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <netdb.h>
#include "User.h"

class Server
{
public:
	bool initialize(uint16_t port);
	bool update();

	void accept_new_connections() const;
	void poll_events();
	void handle_events();

	bool is_running()	const { return m_is_running; }

private:
	// Member variables
	bool				m_is_running;
	int					m_server_socket;

	std::vector<User>	m_users;

	static const int	m_server_backlog;
	static const int	m_timeout;

	typedef std::vector<User>::iterator	UserIterator;
};

#endif //SERVER_H
