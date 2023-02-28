//
// Created by nathan on 2/25/23.
//

#include <iostream>
#include "CommandBuilder.h"

int main()
{

	while (true) {
		CommandBuilder command_builder(true, true, true, true, true, true);
		std::string str = command_builder.to_string();
		Command command(str);
		if (!command.is_valid() || !command_builder.check_validity(command)) {
			std::cout << "INVALID:" << std::endl;
			std::cout << "[" << str << "]" << std::endl;
			command.print();
		}
	}
	return 0;
}
