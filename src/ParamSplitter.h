//
// Created by nathan on 3/7/23.
//

#ifndef PARAMSPLITTER_H
#define PARAMSPLITTER_H

#include <string>
#include "Command.h"

template<char separator>
class ParamSplitter
{
public:
	explicit ParamSplitter(const std::string& parameter);
	ParamSplitter(const Command& command, size_t command_parameter_index);
	ParamSplitter(bool condition, const Command& command, size_t command_parameter_index);

	std::string	next_param();
	bool		reached_end() const { return m_pos >= m_param.size(); }

private:
	typedef std::string::size_type	index_type;

	index_type	find_separator();

	std::string	m_param;
	index_type	m_pos;
};

template<char separator>
ParamSplitter<separator>::ParamSplitter(const std::string &parameter)
	: m_param(parameter), m_pos(0)
{
}

template<char separator>
ParamSplitter<separator>::ParamSplitter(const Command& command, size_t command_parameter_index)
	: m_param(command.get_parameters()[command_parameter_index]), m_pos(0)
{
}
template<char separator>
ParamSplitter<separator>::ParamSplitter(bool condition, const Command& command, size_t command_parameter_index)
	: m_pos(0)
{
	if (condition)
		m_param = command.get_parameters()[command_parameter_index];
}

template<char separator>
std::string ParamSplitter<separator>::next_param()
{
	if (reached_end())
		return "";

	index_type new_pos = find_separator();
	std::string new_param = m_param.substr(m_pos, new_pos - m_pos);
	m_pos = new_pos + 1;

	return new_param;
}

template<char separator>
typename ParamSplitter<separator>::index_type ParamSplitter<separator>::find_separator()
{
	index_type new_pos = m_pos;
	while (new_pos < m_param.size() && m_param[new_pos] != separator)
		new_pos ++;
	return new_pos;
}

#endif //PARAMSPLITTER_H
