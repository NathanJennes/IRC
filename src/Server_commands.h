//
// Created by Cyril Battistolo on 25/02/2023.
//

#ifndef SERVER_COMMANDS_H
#define SERVER_COMMANDS_H

#include "User.h"
#include "Command.h"

// Connection messages
int auth(Server& server, User& user, const Command& command);
int cap(Server& server, User& user, const Command& command);
int error(Server& server, User& user, const Command& command);
int nick(Server& server, User& user, const Command& command);
int user(Server& server, User& user, const Command& command);
int oper(Server& server, User& user, const Command& command);
int pass(Server& server, User& user, const Command& command);
int ping(Server& server, User& user, const Command& command);
int pong(Server& server, User& user, const Command& command);
int quit(Server& server, User& user, const Command& command);

// Channel operations
int join(Server& server, User& user, const Command& command);
int part(Server& server, User& user, const Command& command);
int topic(Server& server, User& user, const Command& command);
int names(Server& server, User& user, const Command& command);
int list(Server& server, User& user, const Command& command);
int invite(Server& server, User& user, const Command& command);
int kick(Server& server, User& user, const Command& command);

// server queries and commands
int motd(Server& server, User& user, const Command& command);
int version(Server& server, User& user, const Command& command);
int admin(Server& server, User& user, const Command& command);
int connect(Server& server, User& user, const Command& command);
int luser(Server& server, User& user, const Command& command);
int time_cmd(Server& server, User& user, const Command& command);
int stats(Server& server, User& user, const Command& command);
int help(Server& server, User& user, const Command& command);
int info(Server& server, User& user, const Command& command);
int mode(Server& server, User& user, const Command& command);

// sending messages
int privmsg(Server& server, User& user, const Command& command);
int notice(Server& server, User& user, const Command& command);

// user based queries
int who(Server& server, User& user, const Command& command);
int whois(Server& server, User& user, const Command& command);
int whowas(Server& server, User& user, const Command& command);

// operator messages
int kill(Server& server, User& user, const Command& command);
int reash(Server& server, User& user, const Command& command);
int restart(Server& server, User& user, const Command& command);
int quit(Server& server, User& user, const Command& command);

// service messages
int away(Server& server, User& user, const Command& command);
int links(Server& server, User& user, const Command& command);
int userhost(Server& server, User& user, const Command& command);
int wallops(Server& server, User& user, const Command& command);


#endif //SERVER_COMMANDS_H
