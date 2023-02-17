//
// Created by nathan on 2/16/23.
//

#include <unistd.h>
#include "User.h"
#include "IRC.h"

User::User(const std::string &username, const std::string &real_name, const std::string &server_name, int fd)
	: m_username(username), m_real_name(real_name), m_server_name(server_name), m_is_afk(false), m_is_disconnected(false), m_fd(fd), m_is_readable(), m_is_writable()
{
}

ssize_t User::receive_message()
{
	char buffer[MAX_MESSAGE_LENGTH + 1];
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < MAX_MESSAGE_LENGTH) {
		ssize_t bytes_read = read(m_fd, buffer + total_bytes_read, static_cast<size_t>(MAX_MESSAGE_LENGTH - total_bytes_read));

		if (bytes_read < 0)
			m_is_disconnected = true;
		if (bytes_read <= 0)
			break ;

		total_bytes_read += bytes_read;
	}
	buffer[total_bytes_read] = 0;

	m_last_message.append(buffer);

	return total_bytes_read;
}

std::string User::get_next_command_str()
{
	std::string command;
	size_t command_end = m_last_message.find_first_of("\n\r");

	if (command_end != std::string::npos) {
		command = m_last_message.substr(0, command_end);
		m_last_message.erase(0, m_last_message.find_first_not_of("\n\r"));
	}

	return command;
}
