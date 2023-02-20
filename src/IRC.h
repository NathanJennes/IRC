//
// Created by nathan on 2/16/23.
//

#ifndef IRC_H
#define IRC_H

#define SERVER_VERSION "0.0.1"
#define MAX_MESSAGE_LENGTH 512

// RPL CODES
#define RPL_WELCOME(networkname, nickname)									(" :Welcome to the " networkname " Network, " + nickname + "\r\n")
#define RPL_YOURHOST(servename, version)									(" :Your host is " servename ", running version " SERVER_VERSION "\r\n")
#define RPL_CREATED(date) 													(" :This server was created " date "\r\n")

#define RPL_MYINFO(servename, version, usermodes, channelmodes, chanparam)	(" " servename " " SERVER_VERSION " " usermodes " " channelmodes " " chanparam "\r\n")
#define RPL_ISUPPORT														("<TOKENS> :are supported by this server\r\n")

#define RPL_BOUNCE // RECOMMENDED BY RFC TO NOT BE USED
#define RPL_UMODEIS(set_usermodes)											(" " set_usermodes "\r\n")

#define RPL_LUSERCLIENT(nbr_users, nbr_invisible, nbr_servers)				(" :There are " nbr_users " users and " nbr_invisible " invisible on " nbr_servers " servers\r\n")
#define RPL_LUSEROP(nbr_opers)												(" " nbr_opers " :operator(s) online\r\n")
#define RPL_LUSERUNKNOWN(nbr_unknown)										(" " nbr_unknown " :unknown connection(s)\r\n")
#define RPL_LUSERCHANNELS(nbr_channels)										(" " nbr_channels " :channels formed\r\n")
#define RPL_LUSERME(nbr_users, nbr_servers)									(" :I have " nbr_users " clients and " nbr_servers " servers\r\n")
#define RPL_ADMINME(servename)												(" " servename " :Administrative info\r\n")
#define RPL_ADMINLOC1(admin_location)										(" " admin_location "\r\n")
#define RPL_ADMINLOC2(hosting_location)										(" " hosting_location "\r\n")
#define RPL_ADMINEMAIL(admin_email)											(" " admin_email "\r\n")
#define RPL_TRYAGAIN(command)												(" " command " :Please wait a while and try again.\r\n")
#define RPL_LOCALUSERS(current_users, max_users)							(" :Current local users " current_users " max " max_users "\r\n")
#define RPL_GLOBALUSERS(current_users, max_users)							(" :Current global users " current_users " max " max_users "\r\n")
#define RPL_WHOISCERTFP(nickname, certfp)									(" " + nickname + " :has client certificate fingerprint " certfp "\r\n")
#define RPL_NONE															(" :Unknown format\r\n")
#define RPL_AWAY(nickname, away_message)									(" " + nickname + " :" away_message "\r\n")
#define RPL_USERHOST														// TODO : Implement function to return a list of userhost
#define RPL_UNAWAY															(":You are no longer marked as being away\r\n")
#define RPL_NOWAWAY															(":You have been marked as being away\r\n")
#define RPL_WHOREPLY(channel, username, hostname, servername, \
					 nickname, flags, hopcount, realname)					(" " channel " " username " " hostname " " servername " " + nickname + " " flags " :"hopcount " " realname "\r\n")
#define RPL_ENDOFWHO(mask)													(" " mask " :End of /WHO list.\r\n")
#define RPL_WHOISREGNICK(nickname)											(" " + nickname + " :is a registered nick\r\n")
#define RPL_WHOISUSER(nickname, username, hostname, realname)				(" " + nickname + " " username " " hostname " * :" realname "\r\n")
#define RPL_WHOISSERVER(nickname, servername, serverinfo)					(" " + nickname + " " servername " :" serverinfo "\r\n")
#define RPL_WHOISOPERATOR(nickname)											(" " + nickname + " :is an IRC operator\r\n")
#define RPL_WHOWASUSER(nickname, username, hostname, realname)				(" " + nickname + " " username " " hostname " * :" realname "\r\n")
#define RPL_WHOISIDLE(nickname, idle_time, signon)							(" " + nickname + " " idle_time " :seconds idle, " signon "\r\n")
#define RPL_ENDOFWHOIS(nickname)											(" " + nickname + " :End of /WHOIS list.\r\n")
#define RPL_WHOISCHANNELS() 												""// TODO : Implement function to return a list of channels
#define RPL_WHOISSPECIAL(nickname, msg)										(" " + nickname + " :" msg "\r\n")
#define RPL_LISTSTART 3214
#define RPL_LIST 322
#define RPL_LISTEND 323
#define RPL_CHANNELMODEIS 324
#define RPL_CREATIONTIME 329
#define RPL_WHOISACCOUNT 330
#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_TOPICWHOTIME 333
#define RPL_INVITELIST 336
#define RPL_ENDOFINVITELIST 337
#define RPL_WHOISACTUALLY 338
#define RPL_INVITING 341
#define RPL_INVEXLIST 346
#define RPL_ENDOFINVEXLIST 347
#define RPL_EXCEPTLIST 348
#define RPL_ENDOFEXCEPTLIST 349
#define RPL_VERSION 351
#define RPL_NAMREPLY 353
#define RPL_ENDOFNAMES 366
#define RPL_LINKS 364
#define RPL_ENDOFLINKS 365
#define RPL_BANLIST 367
#define RPL_ENDOFBANLIST 368
#define RPL_ENDOFWHOWAS 369
#define RPL_INFO 371
#define RPL_ENDOFINFO 374
#define ERR_UNKNOWNERROR(ERRCODE)											(" " ERRCODE " :Unknown error\r\n")
#define ERR_NOSUCHNICK(nickname)											(" " + nickname + " :No such nick/channel\r\n")
#define ERR_NOSUCHSERVER(servername)										(" " servername " :No such server\r\n")
#define ERR_NOSUCHCHANNEL(channel)											(" " channel " :No such channel\r\n")
#define ERR_CANNOTSENDTOCHAN(channel)										(" " channel " :Cannot send to channel\r\n")
#define ERR_TOOMANYCHANNELS(channel)										(" " channel " :You have joined too many channels\r\n")
#define ERR_WASNOSUCHNICK(nickname)											(" " + nickname + " :There was no such nickname\r\n")
#define ERR_NOORIGIN														(" :No origin specified\r\n")
#define ERR_INPUTTOOLONG													(" :Input too long\r\n")
#define ERR_UNKNOWNCOMMAND(command)											(" " command " :Unknown command\r\n")
#define ERR_NOMOTD															(" :MOTD File is missing\r\n")
#define ERR_ERRONEUSNICKNAME(nickname)										(" " nickname " :Erroneous nickname\r\n")
#define ERR_NICKNAMEINUSE(nickname)											(" " + nickname + " :Nickname is already in use\r\n")
#define ERR_USERNOTINCHANNEL(nickname, channel)								(" " + nickname + " " channel " :They aren't on that channel\r\n")
#define ERR_NOTONCHANNEL(channel)											(" " channel " :You're not on that channel\r\n")
#define ERR_USERONCHANNEL(nickname, channel)								(" " + nickname + " " channel " :is already on channel\r\n")
#define ERR_NOTREGISTERED													(" :You have not registered\r\n")
#define ERR_NEEDMOREPARAMS(command)											(" " command " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTERED												(" :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH													(" :Password incorrect\r\n")
#define ERR_YOUREBANNEDCREEP												(" :You are banned from this server\r\n")
#define ERR_CHANNELISFULL(channel)											(" " channel " :Cannot join channel (+l)\r\n")
#define ERR_UNKNOWNMODE(mode)												(" " mode " :is unknown mode char to me\r\n")
#define ERR_INVITEONLYCHAN(channel)											(" " channel " :Cannot join channel (+i)\r\n")
#define ERR_BANNEDFROMCHAN(channel)											(" " channel " :Cannot join channel (+b)\r\n")
#define ERR_BADCHANNELKEY(channel)											(" " channel " :Cannot join channel (+k)\r\n")
#define ERR_BADCHANMASK(channel)											(" " channel " :Bad Channel Mask\r\n")
#define ERR_NOPRIVILEGES													(" :Permission Denied- You're not an IRC operator\r\n")
#define ERR_CHANOPRIVSNEEDED(channel)										(" " channel " :You're not channel operator\r\n")
#define ERR_CANTKILLSERVER													(" :You cant kill a server!\r\n")
#define ERR_NOOPERHOST														(" :No O-lines for your host\r\n")
#define ERR_UMODEUNKNOWNFLAG												(" :Unknown MODE flag\r\n")
#define ERR_USERSDONTMATCH													(" :Cant change mode for other users\r\n")
#define ERR_HELPNOTFOUND(topic)												(" " topic " :No help available on this topic\r\n")
#define ERR_INVALIDKEY(target_chan)											(" " target_chan " :Key is not well-formed\r\n")
#define RPL_STARTTLS														(" :STARTTLS successful, proceed with TLS handshake\r\n")
#define RPL_WHOISSECURE(nickname)											(" " + nickname + " :is using a secure connection\r\n")
#define ERR_STARTTLS(msg)													(" :STARTTLS failed, (" msg ")\r\n")
#define ERR_INVALIDMODEPARAM
#define RPL_HELPSTART 704
#define RPL_HELPTXT 705
#define RPL_ENDOFHELP 706
#define ERR_NOPRIVS(priv)													(" " priv " :Insufficient oper privileges.\r\n")
#define RPL_LOGGEDIN(nickname, username, account)							(" " + nickname + " " account " :You are now logged in as " username "\r\n")
#define RPL_LOGGEDOUT(nickname)												(" " + nickname + " :You are now logged out\r\n")
#define ERR_NICKLOCKED														(" :You must use a nick assigned to you\r\n")
#define RPL_SASLSUCCESS														(" :SASL authentication successful\r\n")
#define ERR_SASLFAIL														(" :SASL authentication failed\r\n")
#define ERR_SASLTOOLONG														(" :SASL message too long\r\n")
#define ERR_SASLABORTED														(" :SASL authentication aborted\r\n")
#define ERR_SASLALREADY														(" :You have already authenticated using SASL\r\n")
#define RPL_SASLMECHS(mechs)												(" " mechs " :are available SASL mechanisms\r\n")

#endif //IRC_H
