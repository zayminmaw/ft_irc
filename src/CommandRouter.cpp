/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/13 19:13:34 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		c.queueOutbound(Reply::errUnknownCommand(_server.getName(), c.getNick(), msg.command));
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
	c.queueOutbound(Reply::errNotRegistered(_server.getName(), c.getNick()));
	return false;
}

void CommandRouter::tryRegister(Client& c) { 
	if (c.isRegistered())
		return;
	if (!_server.getPassword().empty() && !c.hasPass())
		return;
	if (!c.hasNick() || !c.hasUser())
		return;
	c.markRegistered();
	c.queueOutbound(Reply::welcome(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::yourHost(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::created(_server.getName(), c.getNick()));
    c.queueOutbound(Reply::myInfo(_server.getName(), c.getNick()));
}

void CommandRouter::handlePass(Client& c, const IRCMessage& m) { 
	if (c.isRegistered()) {
		c.queueOutbound(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (m.params[0] != _server.getPassword()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	c.markPass();
}
 
void CommandRouter::handleNick(Client& c, const IRCMessage& m) { 
	if (m.params.empty()) {
		c.queueOutbound(Reply::errNoNicknameGiven(_server.getName(), c.getNick()));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	std::string newNick = m.params[0];
	if (newNick.find_first_of(" ,*?!@.") != std::string::npos) {
		c.queueOutbound(Reply::errErroneousNick(_server.getName(), c.getNick(), newNick));
		return;
	}
	Client* existing = _server.findClientByNick(newNick);
	if (existing && existing != &c) {
		c.queueOutbound(Reply::errNickInUse(_server.getName(), c.getNick(), newNick));
		return;
	}
	if (c.isRegistered()) {
		c.setNick(newNick);
	} else {
		c.setNick(newNick);
		tryRegister(c);
	}
}

void CommandRouter::handleUser(Client& c, const IRCMessage& m) { 
	if (c.isRegistered()) {
		c.queueOutbound(Reply::errAlreadyRegistered(_server.getName(), c.getNick()));
		return;
	}
	if (m.params.size() < 4) {
		c.queueOutbound(Reply::errNeedMoreParams(_server.getName(), c.getNick(), m.command));
		return;
	}
	if (!_server.getPassword().empty() && !c.hasPass()) {
		c.queueOutbound(Reply::errPasswdMismatch(_server.getName(), c.getNick()));
		return;
	}
	c.setUser(m.params[0]);
	tryRegister(c);
}

void CommandRouter::handleCap(Client& c, const IRCMessage& m) {
    (void)m;
    c.queueOutbound(":" + _server.getName() + " CAP * LS :");
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

