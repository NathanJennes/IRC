//
// Created by Cyril Battistolo on 25/02/2023.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include "User.h"
#include "Command.h"

// Connection messages
int auth(User& user, const Command& command);
int cap(User& user, const Command& command);
int error(User& user, const Command& command);
int nick(User& user, const Command& command);
int user(User& user, const Command& command);
int oper(User& user, const Command& command);
int pass(User& user, const Command& command);
int ping(User& user, const Command& command);
int pong(User& user, const Command& command);
int quit(User& user, const Command& command);

// Channel operations
int join(User& user, const Command& command);
int part(User& user, const Command& command);
int topic(User& user, const Command& command);
int names(User& user, const Command& command);
int list(User& user, const Command& command);
int invite(User& user, const Command& command);
int kick(User& user, const Command& command);

// server queries and commands
int motd(User& user, const Command& command);
int version(User& user, const Command& command);
int admin(User& user, const Command& command);
int connect(User& user, const Command& command);
int luser(User& user, const Command& command);
int time_cmd(User& user, const Command& command);
int stats(User& user, const Command& command);
int help(User& user, const Command& command);
int info(User& user, const Command& command);
int mode(User& user, const Command& command);

// sending messages
int privmsg(User& user, const Command& command);
int notice(User& user, const Command& command);

// user based queries
int who(User& user, const Command& command);
int whois(User& user, const Command& command);
int whowas(User& user, const Command& command);

// operator messages
int kill(User& user, const Command& command);
int reash(User& user, const Command& command);
int restart(User& user, const Command& command);
int quit(User& user, const Command& command);

// service messages
int away(User& user, const Command& command);
int links(User& user, const Command& command);
int userhost(User& user, const Command& command);
int wallops(User& user, const Command& command);

// ====================
// Message_utils
// ====================

bool is_channel(const std::string& name);

#endif //MESSAGE_H
