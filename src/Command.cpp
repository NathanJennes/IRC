//
// Created by Cyril Battistolo on 17/02/2023.
//

#include "Command.h"
#include "IRC.h"
#include <iostream>

Command::Command(const std::string &command_str)
	:m_ill_formed(false), m_str(command_str), m_index(0)
{
}

bool Command::is_valid()
{
	return true;
}

void Command::execute(User& user)
{
	(void)user;
	std::cout << "Executing command: " << m_str << std::endl;
	if (m_str == "CAP LS 302") {
		user.update_write_buffer("CAP * LS :\r\n");
	}
	else if (m_str == ("CAP REQ")) {
		user.update_write_buffer("CAP * ACK :\r\n");
	}
	else if (m_str == "CAP END")
		user.update_write_buffer(RPL_WELCOME(user.nickname(), user.name_on_host(), user.nickname()));
	else if (m_str == ("JOIN")) {
		user.update_write_buffer("JOINNED\r\n");
	}
}

// Message
std::string	Command::parse_message()
{
	return "";
}

bool	Command::consume_spaces()
{
	if (!consume_char(0x20)) // ' '
		return false;

	while (consume_char(0x20)) // ' '
		;
	return true;
}

bool	Command::consume_crlf()
{
	if (!consume_char(0x0D)) // '\r'
		return false;
	if (!consume_char(0x0A)) // '\n'
		return false;
	return true;
}


// Tags
void Command::parse_tags()
{
	if (!consume_char(0x40)) // '@'
		return ;

	Tag tag = parse_tag();
	while (!tag.key.empty()) {
		m_tags.push_back(tag);
		if (!consume_char(0x3B)) // ';'
			return ;
		tag = parse_tag();
	}
}

Tag Command::parse_tag()
{
	Tag tag;
	tag.key = parse_key();
	if (!consume_char(0x3D)) // '='
		return tag;
	tag.value = parse_escaped_value();
	return tag;
}

std::string Command::parse_key()
{
	std::string client_prefix = parse_client_prefix();
	std::string vendor = parse_vendor();

	size_t end_of_key = m_str.find_first_not_of(letters() + digits() + "-", m_index);
	if (end_of_key == std::string::npos) {
		m_ill_formed = true;
		return "";
	}
	std::string key = m_str.substr(m_index, end_of_key);
	m_index = end_of_key;

	return client_prefix + vendor + key;
}

std::string Command::parse_client_prefix()
{
	std::string client_prefix;
	if (consume_char(0x2B)) // '+'
		client_prefix += '+';
	return client_prefix;
}

std::string Command::parse_escaped_value()
{
	size_t end_of_escaped_value = m_str.find_first_of("\0\r\n; ", m_index);
	if (end_of_escaped_value == std::string::npos) {
		m_ill_formed = true;
		return "";
	}
	std::string escaped_value = m_str.substr(m_index, end_of_escaped_value);
	m_index = end_of_escaped_value;

	return escaped_value;
}

std::string Command::parse_vendor()
{
	return parse_host();
}


// Source
std::string Command::parse_source()
{
	return "";
}


// Command
std::string Command::parse_command()
{
	return "";
}


// Parameters
std::string Command::parse_parameter()
{
	return "";
}

std::string Command::parse_nospcrlfcl()
{
	return "";
}

std::string Command::parse_middle()
{
	return "";
}

std::string Command::parse_trailing()
{
	return "";
}


// Wildcard
std::string Command::parse_mask()
{
	return "";
}

std::string Command::parse_wildone()
{
	return "";
}

std::string Command::parse_wildmany()
{
	return "";
}

std::string Command::parse_nowild()
{
	return "";
}

std::string Command::parse_noesc()
{
	return "";
}

std::string Command::parse_matchone()
{
	return "";
}

std::string Command::parse_matchmany()
{
	return "";
}

// Hostname
std::string	Command::parse_host()
{
	std::string hostname = parse_hostname();
	if (!hostname.empty())
		return hostname;

	std::string ipv4_address = parse_ipv4_address();
	return ipv4_address;
}

std::string	Command::parse_hostname()
{
	std::size_t old_index = m_index;
	std::string hostname;
	std::string domain_label = parse_domain_label();

	while (!domain_label.empty()) {
		hostname += domain_label;
		if (!consume_char(0x2E)) // '.'
			break ;
		domain_label = parse_domain_label();
	}

	if (contains_top_label(hostname)) {
		if (consume_char(0x2E)) // '.'
			hostname += '.';
		return hostname;
	}

	// If we are here, it means that we need to parse an ipv4
	m_index = old_index;
	hostname = parse_ipv4_address();

	return hostname;
}

std::string	Command::parse_ipv4_address()
{
	std::string address;

	for (int i = 0; i < 3; i++)
	{
		std::string numbers = consume_integer();
		if (numbers.empty())
			return "";
		address += numbers;
		if (!consume_char(0x2E)) // '.'
			return "";
		address += '.';
	}

	std::string numbers = consume_integer();
	if (numbers.empty())
		return "";
	address += numbers;

	return address;
}

std::string	Command::parse_domain_label()
{
	std::string domain_label;
	if (isalnum(current_char())) {
		domain_label += current_char();
		consume_char(current_char());

	}

	return "";
}

bool Command::contains_top_label(const std::string& hostname)
{
	(void)hostname;
	return true;
}

// Helper
bool Command::consume_char(char c)
{
	if (!characters_left() || current_char() != c)
		return false;

	m_index++;
	return true;
}

std::string Command::consume_integer()
{
	return "";
}

std::string Command::consume_string()
{
	return "";
}

char Command::current_char()
{
	return m_str[m_index];
}

std::size_t Command::characters_left()
{
	if (m_index > m_str.size())
		return 0;
	return m_str.size() - m_index;
}

const std::string& Command::letters()
{
	static std::string letters = "abcdefghijklmnopqrstuvwxyz"
								 "ABECDEFIHJKLMNOPQRSTUVWXYZ";
	return letters;
}

const std::string& Command::digits()
{
	static std::string digits = "0123456789";
	return digits;
}
