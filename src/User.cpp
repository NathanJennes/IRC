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
		m_is_disconnected(false),
		m_is_readable(false), m_is_writable(false),
		m_is_registered(false), m_is_negociating_capabilities(false),
		m_need_password(true),
		m_is_afk(false), m_is_operator(false), m_is_invisible(true),
		m_can_receive_wallop(true), m_can_receive_notice(true),
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
	if (!has_pending_command())
		return "";

	std::size_t command_end = m_readbuf.find_first_of("\n\r");
	std::size_t crlf_end = command_end + 1;
	if (m_readbuf[crlf_end] == '\r' || m_readbuf[crlf_end] == '\n')
		crlf_end++;

	std::string command;
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
	// TODO: break PRIVMSG if realname as space. hardcoded to localhost for now
	std::string source = ":" + nickname() + "!~" + m_username + "@localhost";
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

bool User::has_pending_command()
{
	return m_readbuf.find_first_of("\r\n") != std::string::npos;
}

void User::add_channel(Channel &channel)
{
	m_channels.push_back(&channel);
}

std::string User::get_modes_as_str()	const
{
	std::string modes = "+";

	if (is_operator())
		modes += "o";
	if (is_invisible())
		modes += "i";
	if (can_get_wallop())
		modes += "w";
	if (can_get_notice())
		modes += "s";
	return modes;
}

bool User::update_mode(const std::vector<mode_param>& mode_params)
{
	CORE_TRACE("mode_params size: %d", (int)mode_params.size());
	for (size_t i = 0; i < mode_params.size(); i++)
	{
		const mode_param& mode = mode_params[i];
		switch (mode.mode) {
			case 'o':
				// TODO: implement operator mode
				break ;
			case 'i':
				m_is_invisible = mode.is_adding;
				break ;
			case 'w':
				m_can_receive_wallop = mode.is_adding;
				break ;
			case 's':
				m_can_receive_notice = mode.is_adding;
				break ;
			default:
				Server::reply(*this, ERR_UNKNOWNMODE((*this), mode.mode));
				return false;
		}
	}
	return false;
}
