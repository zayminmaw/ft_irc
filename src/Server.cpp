/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 22:07:06 by wmin-kha         ###   ########.fr       */
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
														_port(port), _password(password), _name("ircserv"), _router(NULL)
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

	std::cout << "\n\r[Server] Shutdown signal received. Closing gracefully..." << std::endl;
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

	//
	while (!_shutdownRequested)
	{
		usleep(100000);
	}

	if (_serverFd >= 0)
	{
		close(_serverFd);
		std::cout << "[Server] Server socket closed." << std::endl;
	}
}