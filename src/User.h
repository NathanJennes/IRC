//
// Created by nathan on 2/16/23.
//

#ifndef USER_H
#define USER_H

#include <string>
#include <netinet/in.h>
#include <sys/poll.h>

class User
{
public:
	User(const std::string& username, const std::string& real_name, const std::string& server_name, int fd);

	ssize_t receive_message();

	const std::string&	username()			const { return m_username; }
	const std::string&	real_name()			const { return m_real_name; }
	const std::string&	server_name()		const { return m_server_name; }
	const int&			is_afk()			const { return m_is_afk; }
	const int&			is_disconnected()	const { return m_is_disconnected; }
	const int&			fd()				const { return m_fd; }
	bool				is_writable()		const { return m_is_writable; }
	bool				is_readable()		const { return m_is_readable; }

	void	set_is_readable(bool is_readable) { m_is_readable = is_readable; }
	void	set_is_writable(bool is_writable) { m_is_writable = is_writable; }

private:
	std::string m_username;
	std::string m_real_name;
	std::string m_server_name;

	int			m_is_afk;
	int			m_is_disconnected;

	int			m_fd;
	sockaddr_in	m_address;

	bool		m_is_readable;
	bool		m_is_writable;
};

#endif //USER_H
