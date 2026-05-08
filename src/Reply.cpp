/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/08 20:08:42 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Reply.hpp"

static std::string _fmt(const std::string &sv, const std::string &nr, const std::string &n) {
	return ":" + sv + " " + nr + " " + (n.empty() ? "*" : n) + " ";	
}

std::string Reply::welcome(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "001", nick) + ":Welcome to the Internet Relat Network " + nick; 
}

std::string Reply::yourHost(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "002", nick) + ":Your host is " + serverName + ", running version 1.0";
}

std::string Reply::created(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "002", nick) + ":This server was created " + "May 2026";
}

std::string Reply::myInfo(const std::string& serverName, const std::string& nick) {
	return _fmt(serverName, "002", nick) + serverName + " 1.0 " + "<usermodes> <chanmodes>";
}

std::string Reply::topic(const std::string& serverName, const std::string& nick, const std::string& chan, const std::string& topic) {
	return _fmt(serverName, "332", nick) + chan + " :" + topic; 
}

std::string Reply::noTopic(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "331", nick) +  chan + " :No topic is set";
}

std::string Reply::namReply(const std::string& serverName, const std::string& nick, const std::string& chan, const std::string& names) {
	return _fmt(serverName, "353", nick) + "= " + chan + " :" + names;
}

std::string Reply::endOfNames(const std::string& serverName, const std::string& nick, const std::string& chan) {
	return _fmt(serverName, "366", nick) + chan + ":End of /NAMES list";
}