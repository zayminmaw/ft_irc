/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/21 17:06:53 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <cctype>
#include <iostream>
#include <sys/socket.h> // for socket(), bind(), listen()
#include <netinet/in.h> // for struct sockaddr_in and htons()
#include <fcntl.h>		// for fcntl() and O_NONBLOCK
#include <stdexcept>	// for std::runtime_error
#include <cstring>		// for std::memset
#include <unistd.h>		// for close()
#include <csignal>		// for signal()
#include <errno.h>
#include <arpa/inet.h> // For inet_ntoa
#include "Reply.hpp"

static std::string toLower(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		str[i] = std::tolower(static_cast<unsigned char>(str[i]));
	}
	return (str);
}

volatile bool Server::_shutdownRequested = false;

Server::Server(int port, const std::string &password) : _serverFd(-1),
														_port(port), _password(password), _name("ircserv"), _router(new CommandRouter(*this))
{
}

Server::~Server()
{
	if (_router)
		delete _router;
}

// Getters

const std::map<int, Client *> &Server::getClients() const { return _clients; }
const std::string &Server::getPassword() const { return _password; }
const std::string &Server::getName() const { return _name; }

// client registry
Client *Server::findClientByNick(const std::string &nick)
{
	std::string target_lower = toLower(nick);

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (toLower(it->second->getNick()) == target_lower)
		{
			return it->second;
		}
	}
	return (NULL);
}

// Channel registry
Channel *Server::findChannel(const std::string &name)
{
	std::string lower_name = toLower(name);
	std::map<std::string, Channel *>::iterator it = _channels.find(lower_name);

	if (it != _channels.end())
	{
		return it->second;
	}
	return NULL;
}

Channel *Server::getOrCreateChannel(const std::string &name)
{
	std::string lower_name = toLower(name);
	std::map<std::string, Channel *>::iterator it = _channels.find(lower_name);

	// if exist, return it
	if (it != _channels.end())
	{
		return it->second;
	}

	// if doesn't, create it whth the ORIGINAL case name
	Channel *new_channel = new Channel(name);
	_channels[lower_name] = new_channel;

	return new_channel;
}

void Server::removeChannelIfEmpty(const std::string &name)
{
	std::string lower_name = toLower(name);
	std::map<std::string, Channel *>::iterator it = _channels.find(lower_name);

	if (it != _channels.end())
	{
		Channel *channel = it->second;

		if (channel->memberCount() == 0)
		{
			delete channel;
			_channels.erase(it);
		}
	}
}

void Server::requestShutdown(int signum)
{
	(void)signum;

	_shutdownRequested = true;

	std::cout << "\n\r[Server] Shutdown signal received." << std::endl;
}

void Server::setupListenSocket()
{
	// create socket
	// AF_INET = IPv4, SOCK_STREAM = TCP protocol
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
	{
		throw std::runtime_error("Failed to create server socket.");
	}

	// set SO_REUSEADDR (ghost port fix)
	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to set SO_REUSEADDR.");
	}

	// set O_NONBLOCK
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to set socket to non-blocking.");
	}

	// bind the socket to the port
	struct sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr)); // zero out the struct
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // liston all network interfaces (127.0.0.1, LAN IP...)
	serv_addr.sin_port = htons(_port);		// Host TO Network Short - flips the bytes
	// PC reads memory backwards (Little ENdian). internet reads memory forwards (big Endian)

	if (bind(_serverFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		close(_serverFd);
		// throw std::runtime_error("Port is already in use (bind failed).");
		throw std::runtime_error(std::string("Bind failed: ") + strerror(errno));
	}

	// SOMAXCOON to queue as many pending connections as it can
	if (listen(_serverFd, SOMAXCONN) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to start listening on socket.");
	}
}

void Server::run()
{

	signal(SIGINT, Server::requestShutdown); // handle Ctrl+C
	signal(SIGPIPE, SIG_IGN);				 // ignore broken pipes to prevent crashes

	//
	setupListenSocket();
	std::cout << "[Server] Listening on port " << _port << " (Non-Blocking)" << std::endl;

	// init the poll list with master server socket at index 0
	struct pollfd server_pfd;
	server_pfd.fd = _serverFd;
	server_pfd.events = POLLIN;
	server_pfd.revents = 0;
	_pollFds.push_back(server_pfd);

	// event loop
	while (!_shutdownRequested)
	{
		// POLLOUT toggling
		for (size_t i = 1; i < _pollFds.size(); i++)
		{
			int fd = _pollFds[i].fd;
			Client *client = _clients[fd];

			if (client->hasPendingOutbound())
			{
				_pollFds[i].events |= POLLOUT;
			}
			else
			{
				_pollFds[i].events &= ~POLLOUT;
			}
		}

		// put the program to sleep until a socket wake it up
		// '-1' to wait forever without timing out
		int poll_count = poll(&_pollFds[0], _pollFds.size(), -1);

		if (poll_count < 0)
		{
			// if Ctrl+C was pressed, poll() will return -1 with EINTR;
			// while loop already caught it for that
			if (!_shutdownRequested)
			{
				std::cerr << "Error in poll()" << std::endl;
			}
			break;
		}

		// should not happen with -1 timeout
		// but safe to have
		if (poll_count == 0)
			continue;

		// check the bouncer/server
		// to check new connections
		if (_pollFds[0].revents & POLLIN)
		{
			acceptNewClient();
		}

		// TODO: check client chat messages

		for (size_t i = 1; i < _pollFds.size(); ++i)
		{
			int fd = _pollFds[i].fd;
			Client *client = _clients[fd];

			// check did client send data
			if (_pollFds[i].revents & POLLIN)
			{
				char buffer[1024];
				int bytes_read = recv(fd, buffer, sizeof(buffer), 0);

				if (bytes_read <= 0)
				{
					scheduleDisconnect(client, "Client disconnected abruptly", true);
					continue;
				}

				client->appendToBuffer(buffer, bytes_read);

				if (client->isInboundOverflow())
				{
					scheduleDisconnect(client, "Excess flood");
					continue;
				}

				std::string line;
				while (client->extractLine(line))
				{
					std::cout << "[FD " << fd << "] Incoming: " << line << std::endl;
					// client->queueOutbound("Server heard: " + line);
					_router->dispatch(*client, line);
				}
			}

			// can we send client data
			if (_pollFds[i].revents & POLLOUT)
			{
				if (!client->flushOutbound())
				{
					scheduleDisconnect(client, "Fatal send error");
				}
			}
		}

		// Registration timeout reaper: any unregistered client that has been
		// connected longer than the grace window gets kicked. Runs once per
		// poll cycle, not once per fd.
		time_t now = time(NULL);
		for (size_t i = 1; i < _pollFds.size(); ++i)
		{
			Client *client = _clients[_pollFds[i].fd];
			if (client && !client->isRegistered() && now - client->getConnectionTime() > 60)
				scheduleDisconnect(client, "Registration timeout");
		}

		// SendQ overflow sweep: any client whose outbound buffer has crossed
		// the 64KB cap gets booted. Sticky cap means once flagged they stay
		// flagged until disconnectClient runs.
		for (size_t i = 1; i < _pollFds.size(); ++i)
		{
			Client *client = _clients[_pollFds[i].fd];
			if (client && client->isOutboundOverflow())
				scheduleDisconnect(client, "SendQ exceeded");
		}

		// Execute disconnections
		for (size_t i = 0; i < _disconnectQueue.size(); ++i)
		{
			// .first Client*, .second reason
			int target_fd = _disconnectQueue[i].fd;

			if (_clients.find(target_fd) != _clients.end())
			{
				disconnectClient(_clients[target_fd], _disconnectQueue[i].reason, _disconnectQueue[i].droppedByPeer);
			}
		}
		_disconnectQueue.clear();
	}

	// cleanup after ctrl+c
	std::cout << "[Server] Shutting down" << std::endl;

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->second->getFd());
		delete it->second;
	}
	_clients.clear();

	if (_serverFd >= 0)
	{
		close(_serverFd);
		std::cout << "[Server] Server socket closed." << std::endl;
	}
}

// client
void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// accept connection
	int client_fd = accept(_serverFd, (struct sockaddr *)&client_addr, &client_len);

	if (client_fd < 0)
	{
		// socket is not-blocking and accept might fail if the user disconnected
		// before we could accept. do let's ignore it
		return;
	}

	//!! Make new client non-blocking
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Failed to set client socket to non-blocking." << std::endl;
		close(client_fd);
		return;
	}

	// extract ip adress
	std::string ip = inet_ntoa(client_addr.sin_addr);

	// create new client object
	Client *new_client = new Client(client_fd, ip);
	_clients[client_fd] = new_client;

	// add client to the poll tracking list
	struct pollfd pfd;
	pfd.fd = client_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollFds.push_back(pfd);

	std::cout << "[Server] Accepted new connection from " << ip << " on FD " << client_fd << std::endl;
}

// I/O handlers
void Server::disconnectClient(Client *client, const std::string &reason, bool droppedByPeer)
{
	int fd = client->getFd();

	std::cout << "[Server] Disconnecting FD " << fd << " - Reason: " << reason << std::endl;

	// Broadcast QUIT to channel members and drop this client from every channel
	// before deleting it, so no Channel ends up holding a dangling Client*.
	std::vector<Channel *> joined = client->getChannels();
	std::string quitLine = Reply::quitMsg(client->getPrefix(), reason);
	for (size_t i = 0; i < joined.size(); ++i)
	{
		Channel *ch = joined[i];
		ch->broadcast(quitLine, client);
		ch->removeMember(client);
		removeChannelIfEmpty(ch->getName());
	}
	

	// Best-effort drain of any queued replies (e.g. 464 ERR_PASSWDMISMATCH)
	// so the client gets them before we tear the socket down.
	client->flushOutbound();

	if (!droppedByPeer)
	{

		std::string error_msg = Reply::errorMsg("Closing Link: [" + client->getIp() + "] (" + reason + ")");
		send(fd, error_msg.c_str(), error_msg.length(), 0);
	}

	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			break;
		}
	}

	_clients.erase(fd);

	close(fd);
	delete client;
}

void Server::scheduleDisconnect(Client *c, const std::string &reason, bool droppedByPeer)
{
	_disconnectQueue.push_back(DisconnectAction(c->getFd(), reason, droppedByPeer));
}