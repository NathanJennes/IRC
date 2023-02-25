//
// Created by Cyril Battistolo on 17/02/2023.
//

#include <iostream>
#include "IRC.h"
#include "Server.h"

#define USERNAME "cyril"
#define HOSTNAME "localhost"
#define REALNAME "10.10.10.1"

void test_rpl()
{
	std::string nick = "cybattis";
	std::string server_name = "FT_IRC";

	std::cout << RPL_WELCOME(server_name, nick);
	std::cout << RPL_YOURHOST(server_name, "0.1");
	std::cout << RPL_CREATED("2023-02-17");
	std::cout << RPL_MYINFO(server_name, "0.1", "o", "o", "o");
	std::cout << RPL_ISUPPORT;
	std::cout << RPL_UMODEIS("o");
	std::cout << RPL_LUSERCLIENT("1", "0", "1");
	std::cout << RPL_LUSEROP("1");
	std::cout << RPL_LUSERUNKNOWN("0");
	std::cout << RPL_LUSERCHANNELS("1");
	std::cout << RPL_LUSERME("1", "1");
	std::cout << RPL_ADMINME(server_name);
	std::cout << RPL_ADMINLOC1("Lyon");
	std::cout << RPL_ADMINLOC2("Lyon");
	std::cout << RPL_ADMINEMAIL("bigboss@ft_irc.com");
	std::cout << RPL_TRYAGAIN("JOIN");
	std::cout << RPL_LOCALUSERS("1", "1");
	std::cout << RPL_GLOBALUSERS("1", "1");
	std::cout << RPL_WHOISCERTFP(nick, "123456789");
	std::cout << RPL_NONE;
	std::cout << RPL_AWAY(nick, "I'm away");
	//std::cout << RPL_USERHOST();
	std::cout << RPL_UNAWAY;
	std::cout << RPL_NOWAWAY;
	std::cout << RPL_WHOREPLY("*", USERNAME, HOSTNAME, server_name, nick, "H", "0", REALNAME);
	std::cout << RPL_ENDOFWHO("mask");
	std::cout << RPL_WHOISREGNICK(nick);
	std::cout << RPL_WHOISUSER(nick, USERNAME, HOSTNAME, REALNAME);
	std::cout << RPL_WHOISSERVER(nick, server_name, "les info du server!");
	std::cout << RPL_WHOISOPERATOR(nick);
	std::cout << RPL_WHOWASUSER(nick, USERNAME, HOSTNAME, REALNAME);
	std::cout << RPL_WHOISIDLE(nick, "0", "0");
	std::cout << RPL_ENDOFWHOIS(nick);
	std::cout << RPL_WHOISCHANNELS();
	std::cout << RPL_WHOISSPECIAL(nick, "blablablabla");

	// ERRORS
	std::cout << ERR_NOSUCHNICK(nick);
	std::cout << ERR_NOSUCHSERVER(server_name);
	std::cout << ERR_NOSUCHCHANNEL("#test");
	std::cout << ERR_CANNOTSENDTOCHAN("#test");
	std::cout << ERR_TOOMANYCHANNELS("#test");
	std::cout << ERR_WASNOSUCHNICK(nick);
	std::cout << ERR_NOORIGIN;
	std::cout << ERR_INPUTTOOLONG;
	std::cout << ERR_UNKNOWNCOMMAND("command");
	std::cout << ERR_NOMOTD;
	std::cout << ERR_ERRONEUSNICKNAME("gligli");
	std::cout << ERR_NICKNAMEINUSE(nick);
	std::cout << ERR_USERNOTINCHANNEL(nick, "#test");
	std::cout << ERR_NOTONCHANNEL("#test");
	std::cout << ERR_USERONCHANNEL(nick, "#test");
	std::cout << ERR_NOTREGISTERED;
//	std::cout << ERR_NEEDMOREPARAMS("command", "msg");
	std::cout << ERR_ALREADYREGISTERED;
	std::cout << ERR_PASSWDMISMATCH;
	std::cout << ERR_YOUREBANNEDCREEP;
	std::cout << ERR_CHANNELISFULL("#test");
	std::cout << ERR_UNKNOWNMODE("mode");
	std::cout << ERR_INVITEONLYCHAN("#test");
	std::cout << ERR_BANNEDFROMCHAN("#test");
	std::cout << ERR_BADCHANNELKEY("#test");
	std::cout << ERR_BADCHANMASK("#test");
	std::cout << ERR_NOPRIVILEGES;
	std::cout << ERR_CHANOPRIVSNEEDED("#test");
	std::cout << ERR_CANTKILLSERVER;
	std::cout << ERR_NOOPERHOST;
	std::cout << ERR_UMODEUNKNOWNFLAG;
	std::cout << ERR_USERSDONTMATCH;
	std::cout << ERR_HELPNOTFOUND("dev");
	std::cout << ERR_INVALIDKEY("key");
	std::cout << RPL_STARTTLS;
	std::cout << RPL_WHOISSECURE(nick);

	std::cout << ERR_STARTTLS("LE MESSAGE");
	std::cout << ERR_NOPRIVS("command");
	std::cout << RPL_LOGGEDIN(nick, USERNAME, "account");
	std::cout << RPL_LOGGEDOUT(nick);
	std::cout << ERR_NICKLOCKED;
	std::cout << RPL_SASLSUCCESS;
	std::cout << ERR_SASLFAIL;
	std::cout << ERR_SASLTOOLONG;
	std::cout << ERR_SASLABORTED;
	std::cout << ERR_SASLALREADY;
	std::cout << RPL_SASLMECHS("PLAIN");
}
