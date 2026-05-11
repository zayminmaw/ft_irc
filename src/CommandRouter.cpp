/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 21:33:53 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// PHASE 0 STUB. Person B replaces this whole file in their Phase 1-3.
// Do not add logic here -- leave it empty so the build stays green until B
// drops in the real router.

#include "CommandRouter.hpp"
#include "Reply.hpp"
#include "Client.hpp"
#include "Server.hpp"

CommandRouter::CommandRouter(Server& server)
	: _server(server)
{
	registerHandlers();
}

CommandRouter::~CommandRouter()
{
}

void CommandRouter::dispatch(Client& c, const std::string& line)
{
	IRCMessage msg = tokenize(line);

	if (msg.command.empty())
		return;

	std::map<std::string, Handler>::iterator it = _handlers.find(msg.command);

	if (it != _handlers.end()) {
		(this->*(it->second))(c, msg);
	} else {
		c.send(Reply::errUnknownCommand(_server.getName(), c.getNick(), msg.command));
	}
}

IRCMessage CommandRouter::tokenize(const std::string& line)
{
	IRCMessage msg;
	std::string ref = line;
	
	// 1. Handle prefix (starts with :)
	if (!ref.empty() && ref[0] == ':') {
		size_t space = ref.find(' ');
		if (space != std::string::npos)
		{
			msg.prefix = ref.substr(1, space - 1);
			ref = ref.substr(space + 1);
		}
	}

	// 2. Handle trailing (space followed by colon)
	size_t colon = ref.find(" :");
	std::string trailing;
	bool hasTrailing = false;
	if (colon != std::string::npos) {
		trailing = ref.substr(colon + 2);
		ref = ref.substr(0, colon);
		hasTrailing = true;
	}

	// 3. Handle command and middle params
	std::stringstream ss(ref);
	std::string word;
	if (ss >> word) {
		for (size_t i = 0; i < word.length(); i++)
			word[i] = std::toupper(static_cast<unsigned char>(word[i]));
		msg.command = word;
		while (ss >> word) {
			msg.params.push_back(word);
		}
	}

	if (hasTrailing)
		msg.params.push_back(trailing);
		
	return msg;
}

void CommandRouter::registerHandlers() {
	_handlers["PASS"] = &CommandRouter::handlePass;
	_handlers["NICK"] = &CommandRouter::handleNick;
	_handlers["USER"] = &CommandRouter::handleUser;
	_handlers["CAP"] = &CommandRouter::handleCap;
	_handlers["JOIN"] = &CommandRouter::handleJoin;
	_handlers["PART"] = &CommandRouter::handlePart;
	_handlers["TOPIC"] = &CommandRouter::handleTopic;
	_handlers["MODE"] = &CommandRouter::handleMode;
	_handlers["KICK"] = &CommandRouter::handleKick;
	_handlers["INVITE"] = &CommandRouter::handleInvite;
	_handlers["PRIVMSG"] = &CommandRouter::handlePrivmsg;
	_handlers["NOTICE"] = &CommandRouter::handleNotice;
	_handlers["PING"] = &CommandRouter::handlePing;
	_handlers["QUIT"] = &CommandRouter::handleQuit;
}

bool CommandRouter::_ensureRegistered(Client& c) {
	if (c.isRegistered()) {
		return true;
	}
	c.send(Reply::errNotRegistered(_server.getName(), c.getNick()));
	return false;
}

// --- STUBS FOR PHASE 3 ---

void CommandRouter::handlePass(Client& c, const IRCMessage& m) { (void)c; (void)m; }
void CommandRouter::handleNick(Client& c, const IRCMessage& m) { (void)c; (void)m; }
void CommandRouter::handleUser(Client& c, const IRCMessage& m) { (void)c; (void)m; }
void CommandRouter::handleCap(Client& c, const IRCMessage& m) {
    (void)m;
    c.send(":" + _server.getName() + " CAP * LS :");
}

void CommandRouter::handleJoin(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handlePart(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleTopic(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleMode(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleKick(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleInvite(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handlePrivmsg(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }
void CommandRouter::handleNotice(Client& c, const IRCMessage& m) { if (!_ensureRegistered(c)) return; (void)m; }

void CommandRouter::handlePing(Client& c, const IRCMessage& m) { (void)c; (void)m; }
void CommandRouter::handleQuit(Client& c, const IRCMessage& m) { (void)c; (void)m; }
void CommandRouter::tryRegister(Client& c) { (void)c; }
