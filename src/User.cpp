//
// Created by nathan on 2/16/23.
//

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/time.h>
#include "Server.h"
#include "User.h"
#include "Message.h"
#include "log.h"
#include "IRC.h"

User::User(int fd, const std::string& ip, uint16_t port) :
		m_nickname("*"), m_ip(ip), m_port(port), m_fd(fd),
		m_is_afk(false), m_is_disconnected(false),
		m_is_readable(false), m_is_writable(false),
		m_is_registered(false), m_is_negociating_capabilities(false),
		m_need_password(true),
		m_last_ping_timestamp(), m_ping(0)
{
}

ssize_t User::receive_message()
{
	char buffer[MAX_MESSAGE_LENGTH + 1];
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_read = read(m_fd, buffer + total_bytes_read, static_cast<size_t>(MAX_MESSAGE_LENGTH - total_bytes_read));

		if (bytes_read < 0 && errno != EAGAIN) {
			CORE_ERROR(std::strerror(errno));
		}
		if (bytes_read <= 0)
			break ;

		total_bytes_read += bytes_read;
	}
	buffer[total_bytes_read] = 0;
	m_readbuf.append(buffer);

	return total_bytes_read;
}

ssize_t User::send_message()
{
	ssize_t total_bytes_write = 0;

	while (total_bytes_write < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_write = write(fd(), m_writebuf.c_str() + total_bytes_write, m_writebuf.size() - (size_t)total_bytes_write);

		if (bytes_write < 0 && errno != EAGAIN) {
			CORE_ERROR(std::strerror(errno));
		}
		if (bytes_write <= 0)
			break ;

		total_bytes_write += bytes_write;
	}
	m_writebuf.clear();

	return total_bytes_write;
}

std::string User::get_next_command_str()
{
	std::string command;

	std::size_t command_end = m_readbuf.find_first_of("\n\r");
	if (command_end == std::string::npos)
		return "";

	std::size_t crlf_end = command_end + 1;
	if (m_readbuf[crlf_end] == '\r' || m_readbuf[crlf_end] == '\n')
		crlf_end++;

	if (crlf_end != std::string::npos) {
		command = m_readbuf.substr(0, crlf_end);
		m_readbuf.erase(0, command.size());
	}

	CORE_TRACE("INCOMING FROM %s[%s]", debug_name(), command.c_str());
	CORE_DEBUG("User %s command buffer left: %s", debug_name(), m_readbuf.c_str());
	return command;
}

std::string User::source()
{
	std::string source = ":" + nickname() + "!" + m_username + "@" + m_realname;
	return source;
}

bool User::check_password()
{
	if (!Server::password().empty() && m_password != Server::password())
	{
		Server::reply(*this, ERR_PASSWDMISMATCH((*this)));
		disconnect();
		return false;
	}
	m_need_password = false;
	CORE_DEBUG("User %s succesfully gave password", debug_name());
	return true;
}

void User::try_finish_registration()
{
	CORE_DEBUG("trying to finalize registration: %d | %s | %s | %s | %d", (int)m_is_negociating_capabilities, m_nickname.c_str(), m_username.c_str(), m_realname.c_str(), (int)m_need_password);
	if (m_is_registered)
		return ;

	if (!m_is_negociating_capabilities && !m_nickname.empty()
		&& !m_username.empty() && !m_realname.empty() && !m_need_password)
	{
		m_is_registered = true;
		CORE_TRACE("User %s registered", nickname().c_str());
		Server::reply_welcome_user(*this);
	}
}

void User::take_ping_timestamp()
{
	gettimeofday(&m_last_ping_timestamp, NULL);
}

void User::recalculate_ping()
{
	struct timeval tv = {};
	gettimeofday(&tv, NULL);
	m_ping = (tv.tv_sec * 1000 + tv.tv_usec / 1000) - (m_last_ping_timestamp.tv_sec * 1000 + m_last_ping_timestamp.tv_usec / 1000);
}

const char *User::debug_name()
{
	static std::string debug_name;
	debug_name = m_nickname + "@" + m_ip + ":" + to_string(m_port);
	return debug_name.c_str();
}
