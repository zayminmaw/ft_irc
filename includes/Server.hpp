/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:05:21 by zmin              #+#    #+#             */
/*   Updated: 2026/05/14 20:41:28 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include "CommandRouter.hpp"

class Client;
class Channel;
class CommandRouter;

struct DisconnectAction
{
	int fd;
	std::string reason;
	bool droppedByPeer;

	// just to make it easy to add to the vector
	DisconnectAction(int f, const std::string &r, bool d) : fd(f), reason(r), droppedByPeer(d) {}
};

/*
 * Server
 * ------
 * Owns the listening socket, the poll() loop, the client registry, and the
 * channel registry. All Client* and Channel* lifetimes are managed here.
 *
 * The server delegates IRC protocol logic to CommandRouter via dispatch().
 * It knows nothing about commands beyond "extract a line, hand it off."
 */
class Server
{
public:
	Server(int port, const std::string &password);
	~Server();

	// Boots the listening socket and runs the poll loop until shutdown.
	void run();

	// Signal handler entry point. Sets an internal flag that breaks run().
	static void requestShutdown(int signum);

	// ---- registry lookup (used by CommandRouter) -------------------------
	Client *findClientByNick(const std::string &nick);
	Channel *findChannel(const std::string &name);
	Channel *getOrCreateChannel(const std::string &name);
	void removeChannelIfEmpty(const std::string &name);

	// All connected clients (for WHO-style queries, broadcasts, etc.).
	const std::map<int, Client *> &getClients() const;

	const std::string &getPassword() const;
	const std::string &getName() const; // server name used in numeric prefixes

	// ---- disconnect ------------------------------------------------------
	// Cleanly removes a client: dispatches a synthetic QUIT so channel-mates
	// receive a proper QUIT broadcast, then closes the fd, removes from
	// pollFds, removes from _clients, and deletes the Client.
	//
	// `reason` is the QUIT message ("Client closed connection",
	//  "Read error", "ERROR :Buffer overflow", etc.).
	void disconnectClient(Client *c, const std::string &reason, bool droppedByPeer);
	void scheduleDisconnect(Client *c, const std::string &reason, bool droppedByPeer = false);

private:
	Server(const Server &);
	Server &operator=(const Server &);

	void setupListenSocket();
	void acceptNewClient();
	void handleClientRead(size_t pollIndex);
	void handleClientWrite(size_t pollIndex);

	// Translates a pollfd index back to a Client*. Index 0 is the listener.
	Client *clientAt(size_t pollIndex);

	int _serverFd;
	int _port;
	std::string _password;
	std::string _name;

	std::vector<struct pollfd> _pollFds;
	std::map<int, Client *> _clients;			// fd -> Client
	std::map<std::string, Channel *> _channels; // name (lowercased) -> Channel
	std::vector<DisconnectAction> _disconnectQueue;

	CommandRouter *_router;

	static volatile bool _shutdownRequested;
};

#endif
