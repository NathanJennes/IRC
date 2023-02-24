//
// Created by Cyril Battistolo on 17/02/2023.
//

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include "User.h"

struct Tag
{
	std::string	key;
	std::string	value;
};

class Command
{
public:
	Command(const std::string& command_str);

	bool	is_valid();
	void	execute(User& user);

private:
	bool	m_ill_formed;

	std::string	m_str;
	std::size_t	m_index;

	std::vector<Tag>			m_tags;
	std::string					m_source;
	std::string					m_command;
	std::vector<std::string>	m_parameters;

	// Message
	std::string	parse_message();
	bool		consume_spaces();
	bool		consume_crlf();

	// Tags
	void		parse_tags();
	Tag			parse_tag();
	std::string	parse_key();
	std::string	parse_client_prefix();
	std::string	parse_escaped_value();
	std::string	parse_vendor();

	// Source
	std::string	parse_source();

	// Command
	std::string	parse_command();

	// Parameters
	std::string	parse_parameter();
	std::string	parse_nospcrlfcl();
	std::string	parse_middle();
	std::string	parse_trailing();

	// Wildcard
	std::string	parse_mask();
	std::string	parse_wildone();
	std::string	parse_wildmany();
	std::string	parse_nowild();
	std::string	parse_noesc();
	std::string	parse_matchone();
	std::string	parse_matchmany();

	// Hostname
	std::string	parse_host();
	std::string	parse_hostname();
	std::string	parse_ipv4_address();
	std::string	parse_domain_label();
	bool		contains_top_label(const std::string& hostname);

	// ABNF
	bool		consume_char(char c);
	std::string	consume_integer();
	std::string	consume_string();

	// Helpers
	char				current_char();
	std::size_t			characters_left();
	const std::string&	letters();
	const std::string&	digits();
};


#endif //COMMAND_H
