/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 21:14:10 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_ROUTER_HPP
# define COMMAND_ROUTER_HPP

# include <sstream>
# include <string>
# include <vector>
# include <map>

class Server;
class Client;

/*
 * IRCMessage
 * ----------
 * Parsed form of one IRC line. RFC 1459 strict.
 *
 *   [:prefix] <command> [param]* [:trailing]
 *
 * - `prefix` is empty for client-originated commands (clients don't send prefixes).
 * - `command` is uppercased by the tokenizer for case-insensitive dispatch.
 * - `params` includes the trailing parameter as the last element, with the
 *   leading ':' stripped.
 */
struct IRCMessage
{
	std::string                 prefix;
	std::string                 command;
	std::vector<std::string>    params;
};

/*
 * CommandRouter
 * -------------
 * The protocol layer. Network hands it raw lines via dispatch(); router
 * tokenizes, validates, and executes.
 *
 * Replies go back through Client::send. Broadcasts go through Channel::broadcast.
 * Router never touches sockets, fds, or poll().
 */
class CommandRouter
{
	public:
		explicit CommandRouter(Server& server);
		~CommandRouter();

		// THE seam between network and protocol.
		// `line` is guaranteed: no \r\n, length <= 510, may be empty (router will
		// silently drop empty lines per RFC).
		void                        dispatch(Client& client, const std::string& line);

		// Exposed for testing the tokenizer in isolation.
		static IRCMessage           tokenize(const std::string& line);

	private:
		CommandRouter(const CommandRouter&);
		CommandRouter& operator=(const CommandRouter&);

		typedef void (CommandRouter::*Handler)(Client&, const IRCMessage&);

		void                        registerHandlers();

		// ---- registration ----------------------------------------------------
		void                        handlePass(Client&, const IRCMessage&);
		void                        handleNick(Client&, const IRCMessage&);
		void                        handleUser(Client&, const IRCMessage&);
		void                        handleCap(Client&, const IRCMessage&);

		// Called after any of PASS/NICK/USER changes state; if all three are
		// satisfied and the client isn't yet registered, sends 001 welcome.
		void                        tryRegister(Client&);

		// ---- channel ops -----------------------------------------------------
		void                        handleJoin(Client&, const IRCMessage&);
		void                        handlePart(Client&, const IRCMessage&);
		void                        handleTopic(Client&, const IRCMessage&);
		void                        handleMode(Client&, const IRCMessage&);
		void                        handleKick(Client&, const IRCMessage&);
		void                        handleInvite(Client&, const IRCMessage&);

		// ---- messaging -------------------------------------------------------
		void                        handlePrivmsg(Client&, const IRCMessage&);
		void                        handleNotice(Client&, const IRCMessage&);

		// ---- session ---------------------------------------------------------
		void                        handlePing(Client&, const IRCMessage&);
		void                        handleQuit(Client&, const IRCMessage&);

		Server&                     _server;
		std::map<std::string, Handler> _handlers;
};

#endif
