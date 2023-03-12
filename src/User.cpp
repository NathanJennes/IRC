//
// Created by nathan on 2/16/23.
//

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/time.h>
#include "Server.h"
#include "User.h"
#include "log.h"
#include "Numerics.h"

User::User(int fd, const std::string& ip, uint16_t port) :
		m_nickname("*"), m_hostname("localhost"), m_ip(ip), m_port(port), m_fd(fd),
		m_is_disconnected(false),
		m_is_readable(false), m_is_writable(false),
		m_is_registered(false), m_is_negociating_capabilities(false), m_need_password(true),
		m_is_afk(false), m_is_operator(false), m_is_invisible(false), m_can_receive_wallop(false), m_can_receive_notice(false),
		m_signon_timestamp(), m_last_idle_timestamp(), m_idle(0), m_last_ping_timestamp(), m_ping(0)
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
			break;

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
			break;

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

std::string User::source() const
{
	std::string source = nickname() + "!~" + username() + "@" + ip();
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
		Server::register_user(*this);
		take_signon_timestamp();
		take_idle_timestamp();
	}
}

void User::take_signon_timestamp()
{
	struct timeval tv = {};
	gettimeofday(&tv, NULL);
	long time = tv.tv_sec;
	m_signon_timestamp = to_string(time);
}

void User::take_ping_timestamp()
{
	gettimeofday(&m_last_ping_timestamp, NULL);
}

void User::take_idle_timestamp()
{
	gettimeofday(&m_last_idle_timestamp, NULL);
}

void User::recalculate_ping()
{
	struct timeval tv = {};
	gettimeofday(&tv, NULL);
	m_ping = (tv.tv_sec * 1000 + tv.tv_usec / 1000) - (m_last_ping_timestamp.tv_sec * 1000 + m_last_ping_timestamp.tv_usec / 1000);
}

void User::recalculate_idle()
{
	struct timeval tv = {};
	gettimeofday(&tv, NULL);
	m_idle = tv.tv_sec - m_last_idle_timestamp.tv_sec;
	CORE_TRACE("SEC: %l", m_idle);
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

void User::remove_channel(const Channel &channel)
{
	ChannelIterator it = channels().begin();
	for (; it < channels().end(); ++it) {
		if (get_channel_reference(it).name() == channel.name()) {
			channels().erase(it);
			return;
		}
	}
}

std::string User::get_modes_as_str() const
{
	std::string modes = "+";

	if (is_away())
		modes += "a";
	if (is_operator())
		modes += "o";
	if (is_invisible())
		modes += "i";
	if (can_get_wallop())
		modes += "w";
	if (can_get_notice()) // obsolete
		modes += "s";
	// TODO: add r mode (restricted user connection)
	return modes;
}

bool User::update_mode(const std::vector<ModeParam>& mode_params)
{
	std::string plus_modes_update = "+";
	std::string minus_modes_update = "-";
	bool updated = false;

	for (size_t i = 0; i < mode_params.size(); i++)
	{
		const ModeParam& mode = mode_params[i];

		switch (mode.mode) {
			case 'o':
				if (is_operator() && !mode.is_adding) {
					m_is_operator = mode.is_adding;
					minus_modes_update += mode.mode;
					updated = true;
				}
				break;
			case 'i':
				if (is_invisible() != mode.is_adding) {
					m_is_invisible = mode.is_adding;
					updated = true;
				}
				break;
			case 'w':
				if (can_get_wallop() != mode.is_adding) {
					m_can_receive_wallop = mode.is_adding;
					updated = true;
				}
				break;
			case 's':
				if (can_get_notice() != mode.is_adding) {
					m_can_receive_notice = mode.is_adding;
					updated = true;
				}
				break;
			default:
				Server::reply(*this, ERR_UMODEUNKNOWNFLAG((*this), mode.mode));
				break;
		}

		if (!updated)
			continue;

		if (mode.is_adding)
			plus_modes_update += mode.mode;
		else
			minus_modes_update += mode.mode;
		updated = false;
	}

	if (plus_modes_update.size() == 1) plus_modes_update = "";
	if (minus_modes_update.size() == 1) minus_modes_update = "";

	if (plus_modes_update.empty() && minus_modes_update.empty())
		return false;

	Server::reply(*this, RPL_MODE(*this, plus_modes_update + minus_modes_update));
	return false;
}

bool User::is_nickname_valid(const std::string &nickname)
{
	if (nickname.empty())
		return false;

	if (nickname.find(' ') != std::string::npos ||
		nickname.find(',') != std::string::npos ||
		nickname.find('*') != std::string::npos ||
		nickname.find('?') != std::string::npos ||
		nickname.find('!') != std::string::npos ||
		nickname.find('@') != std::string::npos ||
		nickname.find('.') != std::string::npos)
		return false;

	if (nickname[0] == '$' || nickname[0] == ':')
		return false;

	if (nickname[0] == '#' || nickname[0] == '&')
		return false;

	return true;
}

bool User::is_username_valid(const std::string &username)
{
	if (username.empty())
		return false;

	if (username[0] == ':')
		return false;

	if (username.find(' ') != std::string::npos)
		return false;

	return true;
}

bool User::is_host_valid(const std::string &username)
{
	if (username.empty())
		return false;

	if (username[0] == ':')
		return false;

	if (username.find(' ') != std::string::npos)
		return false;

	return true;
}

bool User::has_channel_in_common(const User& other_user) const
{
	if (channels().size() > other_user.channels().size())
		return other_user.has_channel_in_common(*this);

	ConstChannelIterator chan_u1 = channels().begin();
	ConstChannelIterator chan_u2 = other_user.channels().begin();

	for (; chan_u1 != channels().end(); ++chan_u1) {
		for (; chan_u2 < other_user.channels().end(); ++chan_u2) {
			if (get_channel_reference(chan_u1).name() == get_channel_reference(chan_u2).name()) {
				CORE_DEBUG("common channel found %s", (*chan_u1)->name().c_str());
				return true;
			}
		}
	}
	return false;
}

std::string User::get_user_flags() const
{
	std::string flags = "H";

	if (is_away())
		flags = "G";
	if (is_operator())
		flags += "*";
	return flags;
}

// TODO: to fix
bool User::has_mask(std::vector<Mask> masks) const
{
	std::vector<Mask>::iterator it = masks.begin();
	size_t pos = 0;
	for (; it != masks.end();)
	{
		pos = nickname().find(it->str, pos);
		if (pos == std::string::npos)
			break;
		if (!it->before && pos != 0)
			break ;
		pos += it->str.size();
		if (!it->after && pos < nickname().size())
			break ;
		++it;
		if (it == masks.end())
			return true;
	}

	it = masks.begin();
	pos = 0;
	for (size_t i = 0; i < masks.size(); ++i) {
		pos = username().find(it->str, pos);
		if (pos == std::string::npos)
			break;
		if (!it->before && pos != 0)
			break ;
		pos += it->str.size();
		if (!it->after && pos < username().size())
			break ;
		i++;
		if (i == masks.size())
			return true;
	}

	it = masks.begin();
	pos = 0;
	for (size_t i = 0; i < masks.size(); ++i) {
		pos = hostname().find(it->str, pos);
		if (pos == std::string::npos)
			break;
		if (!it->before && pos != 0)
			break ;
		pos += it->str.size();
		if (!it->after && pos < hostname().size())
			break ;
		i++;
		if (i == masks.size())
			return true;
	}

	it = masks.begin();
	pos = 0;
	for (size_t i = 0; i < masks.size(); ++i) {
		pos = realname().find(it->str, pos);
		if (pos == std::string::npos)
			break;
		if (!it->before && pos != 0)
			break ;
		pos += it->str.size();
		if (!it->after && pos < realname().size())
			break ;
		i++;
		if (i == masks.size())
			return true;
	}
	return false;
}

bool User::is_visible_to_user(User &user) const
{
	if (!is_invisible())
		return true;
	if (is_invisible() && has_channel_in_common(user))
		return true;
	return false;
}

void User::reply_list_of_channel_to_user(User &user)
{
	std::string str;
	ChannelIterator it = channels().begin();
	for(size_t i = 0; it != channels().end(); ++it, ++i) {
		Channel channel = get_channel_reference(it);
		str += channel.get_user_prefix(*this);
		str += channel.name() + " ";
		if (i == 10) {
			Server::reply(user, SERVER_SOURCE("319", (user)) + " " + nickname() + " " + str);
			str.clear();
			i = 0;
		}
	}
	if (!str.empty())
		Server::reply(user, SERVER_SOURCE("319", (user)) + " " + nickname() + " " + str);
}
