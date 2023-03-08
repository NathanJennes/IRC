//
// Created by Cyril Battistolo on 17/02/2023.
//

#include "Command.h"
#include "log.h"
#include <iostream>

Command::Command(const std::string &command_str)
	:m_ill_formed(false), m_str(command_str), m_index(0)
{
	parse_message();
}

bool Command::is_valid()
{
	return !m_ill_formed;
}

// Message
void	Command::parse_message()
{
	if (characters_left() == 0)
		return ;

	// If the first character is an '@'
	if (consume_char(0x40)) {
		// Try parsing tags
		std::vector<Tag> tags = try_parse_tags();
		if (tags.empty()) {
			m_ill_formed = true;
			CORE_WARN("Couldn't parse the tags after the @");
			return ;
		}

		// If successful, parse the required SPACE characters
		if (!consume_spaces()) {
			m_ill_formed = true;
			CORE_WARN("Couldn't parse spaces after the tags");
			return ;
		}

		// Save the parsed tags
		m_tags = tags;
	}

	// Now, if the current character is an ':'
	if (consume_char(0x3A)) {
		// Parse the source
		Source source = try_parse_source();
		if (source.source_name.empty()) {
			m_ill_formed = true;
			CORE_WARN("Couldn't parse source name after the :");
			return ;
		}

		// If successful, parse the required SPACE characters
		if (!consume_spaces()) {
			m_ill_formed = true;
			CORE_WARN("Couldn't parse spaces after the source");
			return ;
		}

		// Save the parsed source
		m_source = source;
	}

	// Now, parse the command
	std::string command = try_parse_command();
	if (command.empty()) {
		m_ill_formed = true;
		CORE_WARN("Couldn't parse command string");
		return ;
	} else {
		// Save the parsed command
		m_command = command;
	}

	// Now, parse the parameters
	m_parameters = try_parse_parameter();

	// Finally, we should end up at the cr-lf field
	if (!consume_crlf()) {
		m_ill_formed = true;
		CORE_WARN("Couldn't parse crlf");
	}
}

bool	Command::consume_spaces()
{
	if (characters_left() == 0)
		return false;

	if (!consume_char(0x20)) // ' '
		return false;

	while (consume_char(0x20)) // ' '
		;
	return true;
}

bool	Command::consume_crlf()
{
	if (characters_left() == 0)
		return false;

	std::size_t old_index = m_index;

	// Consume a cr '\r' or lf '\n'
	if (consume_char(0x0D) || consume_char(0x0A)) // '\r' '\n'
		return true;

	m_index = old_index;
	return false;
}

// Tags
std::vector<Command::Tag> Command::try_parse_tags()
{
	if (characters_left() == 0)
		return std::vector<Command::Tag>();

	std::vector<Tag> tags;
	Tag tag = try_parse_tag();

	// If a tag has an empty key, it is ill formed
	// At least one tag must be present
	if (tag.key.key_str.empty())
		return tags;

	// Parse all the following tags
	while (!tag.key.key_str.empty()) {
		tags.push_back(tag);

		// If no ';' is found, the last tag parsed was the last
		if (!consume_char(0x3B)) // ';'
			return tags;
		tag = try_parse_tag();
	}

	// The tags list must finish with a valid tag
	tags.clear();
	return tags;
}

Command::Tag Command::try_parse_tag()
{
	if (characters_left() == 0)
		return Tag();

	Tag tag;

	// A tag must contain a key
	tag.key = try_parse_key();
	if (tag.key.key_str.empty())
		return tag;

	// If no '=' is found, the tag ends here without a value
	if (!consume_char(0x3D)) // '='
		return tag;

	// Parse the value
	tag.value = try_parse_escaped_value();
	return tag;
}

Command::TagKey Command::try_parse_key()
{
	if (characters_left() == 0)
		return TagKey();

	std::size_t old_index = m_index;

	// Parse optional client prefix and vendor
	std::string client_prefix = try_parse_client_prefix();
	std::string vendor = try_parse_vendor();

	// Find the end of the key string
	std::size_t end_of_key = m_str.find_first_not_of(letters() + digits() + "-", m_index);
	if (end_of_key == std::string::npos) {
		m_index = old_index;
		return TagKey();
	}

	// Extract the key string
	std::string key_str = m_str.substr(m_index, end_of_key - m_index);
	m_index = end_of_key;

	// Populate the TagKey structure
	TagKey key;
	key.client_prefix = client_prefix;
	key.vendor = vendor;
	key.key_str = key_str;

	return key;
}

std::string Command::try_parse_client_prefix()
{
	if (characters_left() == 0)
		return "";

	std::string client_prefix;
	if (consume_char(0x2B)) // '+'
		client_prefix += '+';
	return client_prefix;
}

std::string Command::try_parse_escaped_value()
{
	if (characters_left() == 0)
		return "";

	// Find the end of the escaped value
	std::size_t end_of_escaped_value = m_str.find_first_of("\r\n; ", m_index);
	if (end_of_escaped_value == std::string::npos)
		return "";

	// Extract the string
	std::string escaped_value = m_str.substr(m_index, end_of_escaped_value - m_index);
	m_index = end_of_escaped_value;

	return escaped_value;
}

std::string Command::try_parse_vendor()
{
	if (characters_left() == 0)
		return "";

	std::size_t old_index = m_index;

	// Try parsing a host
	std::string host = try_parse_host();
	if (host.empty())
		return return_empty_string_and_restore_index(old_index);

	// If successful, parse an '/'
	if (!consume_char(0x2F)) { // '/'
		return return_empty_string_and_restore_index(old_index);
	}

	return host;
}

// Source
Command::Source Command::try_parse_source()
{
	if (characters_left() == 0)
		return Source();

	// A source must end with a space
	std::size_t space_position = m_str.find_first_of(' ', m_index);
	if (space_position == std::string::npos)
		return Source();

	Source source;

	// Find the end of the source, either a '!', a '@' or a SPACE
	std::size_t end_of_source = m_str.find_first_of("!@ ", m_index);
	if (end_of_source == std::string::npos)
		return Source();

	// Extract the source
	source.source_name = m_str.substr(m_index, end_of_source - m_index);
	m_index = end_of_source;

	// If the current character is an '!'
	if (consume_char(0x21)) { // '!'
		// Parse a user field
		std::size_t end_of_user = m_str.find_first_of("@ ", m_index);
		source.user = m_str.substr(m_index, end_of_user - m_index);
		m_index = end_of_user;
	}

	// If the current character is an '@'
	if (consume_char(0x40)) { // '@'
		// Parse a host field
		source.host = m_str.substr(m_index, space_position - m_index);
		m_index = space_position;
	}

	return source;
}

// Command
std::string Command::try_parse_command()
{
	if (characters_left() == 0)
		return "";

	std::string command;

	// Try parsing letters
	while (characters_left() && isalpha(current_char())) {
		command += current_char();
		m_index++;
	}

	// If no letters were found, try parsing 3 digits
	if (command.empty() && characters_left() >= 3) {
		if (isdigit(m_str[m_index]) && isdigit(m_str[m_index + 1]) && isdigit(m_str[m_index + 2])) {
			command += m_str.substr(m_index, 3);
			m_index += 3;
		}
	}

	return command;
}


// Parameters
std::vector<std::string> Command::try_parse_parameter()
{
	if (characters_left() == 0)
		return std::vector<std::string>();

	std::vector<std::string> parameters;
	std::size_t old_index = m_index;
	std::size_t index_before_space = m_index;

	// First, parse the "middle" fields
	while (consume_spaces()) {
		std::string middle = try_parse_middle();

		// Middle fields must contain at least one character
		if (middle.empty())
			break ;
		parameters.push_back(middle);
		index_before_space = m_index;
	}

	// Go back before the spaces consumed
	m_index = index_before_space;

	// Finally, parse the optional trailing field
	if (consume_spaces()) {
		// There must be a ':' before the actual trailing field
		if (!consume_char(0x3A)) {
			m_index = old_index;
			return std::vector<std::string>();
		}

		std::string trailing = try_parse_trailing();
		parameters.push_back(trailing);
	}

	return parameters;
}

std::string Command::try_parse_nospcrlfcl()
{
	if (characters_left() == 0)
		return "";

	// Find the end of the string
	std::size_t end_of_string = m_str.find_first_of("\r\n: ", m_index);
	if (end_of_string == std::string::npos)
		return "";

	// Extract the string
	std::string string = m_str.substr(m_index, end_of_string - m_index);
	m_index = end_of_string;
	return string;
}

std::string Command::try_parse_middle()
{
	if (characters_left() == 0)
		return "";

	std::string middle;
	std::size_t old_index = m_index;

	// First, parse a nospcrlfcl string
	middle += try_parse_nospcrlfcl();
	if (middle.empty())
		return return_empty_string_and_restore_index(old_index);

	// Then, try to parse ':' or nospcrlfcl strings
	while (true) {
		if (consume_char(0x3A)) { // ':'
			middle += ':';
			continue;
		}

		std::string nospcrlfcl_string = try_parse_nospcrlfcl();
		if (nospcrlfcl_string.empty())
			break ;
		middle += nospcrlfcl_string;
	}

	return middle;
}

std::string Command::try_parse_trailing()
{
	if (characters_left() == 0)
		return "";

	std::string trailing;

	// Parse all ':', ' ' or nospcrlfcl strings
	while (true) {
		if (consume_char(0x3A)) { // ':'
			trailing += ':';
			continue;
		}

		if (consume_char(0x20)) { // ' '
			trailing += ' ';
			continue;
		}

		std::string nospcrlfcl_string = try_parse_nospcrlfcl();
		if (nospcrlfcl_string.empty())
			break ;
		trailing += nospcrlfcl_string;
	}

	return trailing;
}


// Wildcard
std::string Command::try_parse_mask()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_wildone()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_wildmany()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_nowild()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_noesc()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_matchone()
{
	if (characters_left() == 0)
		return "";
	return "";
}

std::string Command::try_parse_matchmany()
{
	if (characters_left() == 0)
		return "";
	return "";
}

// Hostname
std::string	Command::try_parse_host()
{
	if (characters_left() == 0)
		return "";

	// Try to parse a hostname
	std::string hostname = try_parse_hostname();
	if (!hostname.empty())
		return hostname;

	// If we couldn't parse a hostname, parse an ipv4 address
	std::string ipv4_address = try_parse_ipv4_address();
	return ipv4_address;
}

std::string	Command::try_parse_hostname()
{
	if (characters_left() == 0)
		return "";

	std::size_t old_index = m_index;
	std::string hostname;
	std::string domain_label = try_parse_domain_label();

	// Parse all domain labels
	while (!domain_label.empty()) {
		hostname += domain_label;
		if (!consume_char(0x2E)) // '.'
			break ;
		domain_label = try_parse_domain_label();
	}

	// A hostname must end with a top label and optional '.'
	std::string top_label = try_parse_top_label();
	if (!top_label.empty()) {
		if (consume_char(0x2E)) // '.'
			hostname += '.';
		return hostname;
	}

	// No hostname could be parsed
	return return_empty_string_and_restore_index(old_index);
}

std::string	Command::try_parse_ipv4_address()
{
	if (characters_left() == 0)
		return "";

	std::size_t old_index = m_index;
	std::string address;

	for (int i = 0; i < 3; i++)
	{
		std::string numbers = consume_integer();
		if (numbers.empty())
			return return_empty_string_and_restore_index(old_index);
		address += numbers;
		if (!consume_char(0x2E)) // '.'
			return return_empty_string_and_restore_index(old_index);
		address += '.';
	}

	std::string numbers = consume_integer();
	if (numbers.empty())
		return return_empty_string_and_restore_index(old_index);
	address += numbers;

	return address;
}

std::string	Command::try_parse_domain_label()
{
	if (characters_left() == 0)
		return "";

	std::string domain_label;

	// Domain labels must start with amn alphanum
	if (isalnum(current_char())) {
		domain_label += current_char();
		m_index++;
	} else {
		return "";
	}

	// Get all the alphanum or '-'
	while (isalnum(current_char()) || current_char() == 0x2D) {
		domain_label += current_char();
		m_index++;
	}

	// Domain labels must end with an alphanum
	while (!isalnum(domain_label[domain_label.size() - 1])) {
		m_index--;
		domain_label.erase(domain_label.size() - 1, 1);
	}

	return domain_label;
}

std::string Command::try_parse_top_label()
{
	if (characters_left() == 0)
		return "";

	std::size_t old_index = m_index;
	std::string top_label;

	// Top label must start with an alpha
	if (isalpha(current_char())) {
		top_label += current_char();
		m_index++;
	} else {
		return "";
	}

	// If we don't find another alpha, we return the one already found
	if (isalpha(current_char())) {
		top_label += current_char();
		m_index++;
	} else {
		return top_label;
	}

	// Get all the alphanum or '-'
	while (isalnum(current_char()) || current_char() == 0x2D) {
		top_label += current_char();
		m_index++;
	}

	// Top labels must end with an alphanum
	while (!isalnum(top_label[top_label.size() - 1]) && isalpha(top_label[top_label.size() - 1])) {
		m_index--;
		top_label.erase(top_label.size() - 1, 1);
	}

	// If no alphanum was found, the domain name is ill formed
	if (!isalnum(top_label[top_label.size() - 1]))
		return return_empty_string_and_restore_index(old_index);

	return top_label;
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
	std::string number;
	while (isdigit(current_char())) {
		number += current_char();
		m_index++;
	}
	return number;
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
								 "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return letters;
}

const std::string& Command::digits()
{
	static std::string digits = "0123456789";
	return digits;
}

std::string Command::return_empty_string_and_restore_index(std::size_t old_index) {
	m_index = old_index;
	return "";
}

void Command::print()
{
	CORE_DEBUG("Command :");
	CORE_DEBUG("  Tags :");
	for (size_t i = 0; i < m_tags.size(); i++) {
		CORE_DEBUG("    - Key :");
		CORE_DEBUG("      - client_prefix : [%s]", m_tags[i].key.client_prefix.c_str());
		CORE_DEBUG("      - vendor :        [%s]", m_tags[i].key.vendor.c_str());
		CORE_DEBUG("      - key_str :       [%s]", m_tags[i].key.key_str.c_str());
		CORE_DEBUG("    - Value:            [%s]", m_tags[i].value.c_str());
	}
	CORE_DEBUG("  Source :");
	CORE_DEBUG("    - source_name : [%s]", m_source.source_name.c_str());
	CORE_DEBUG("    - user :        [%s]", m_source.user.c_str());
	CORE_DEBUG("    - host :        [%s]", m_source.host.c_str());
	CORE_DEBUG("  Command string : [%s]", m_command.c_str());
	CORE_DEBUG("  Parameters :");
	for (size_t i = 0; i < m_parameters.size(); i++)
		CORE_DEBUG("    - [%s]", m_parameters[i].c_str());
	CORE_DEBUG("  Validity : %s", is_valid() ? "Yes" : "No");
}
