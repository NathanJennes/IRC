//
// Created by nathan on 2/25/23.
//

#ifndef COMMANDBUILDER_H
#define COMMANDBUILDER_H

#include <string>
#include "Command.h"

class CommandBuilder
{
public:
	CommandBuilder(bool valid_tags, bool valid_source, bool valid_command, bool valid_parameters, bool valid_crlf, bool fuzz_spaces);

	std::string	to_string();
	bool		check_validity(const Command& command);

private:
	std::string		generate_alphanum_string(size_t min_len = 0, size_t max_len = 500);
	std::string		generate_alpha_string(size_t min_len = 0, size_t max_len = 500);
	std::string		generate_digits_string(size_t min_len = 0, size_t max_len = 500);
	std::string		generate_ascii_string(size_t min_len = 0, size_t max_len = 500);
	std::string		generate_spaces(size_t min_len = 0, size_t max_len = 500);

	Command::Tag	generate_tag(bool &tag_valid);
	Command::Source	generate_source(bool &source_valid);

	std::string					m_tags_address_sign;
	std::vector<Command::Tag>	m_tags;
	std::string					m_tags_spaces;

	std::string					m_colon;
	std::string					m_source_user_letter;
	std::string					m_source_host_letter;
	Command::Source				m_source;
	std::string					m_source_spaces;

	std::string					m_command;

	std::string					m_param_space;
	std::vector<std::string>	m_parameters;
	bool						m_has_last_param;
	std::string					m_last_parameter;
	std::string					m_last_param_space;

	std::string					m_crlf;

	bool	m_valid_tags;
	bool	m_valid_source;
	bool	m_valid_command;
	bool	m_valid_parameters;
	bool	m_valid_crlf;
	bool	m_fuzz_spaces;

};

#endif //COMMANDBUILDER_H
