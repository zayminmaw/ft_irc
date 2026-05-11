/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 20:57:24 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <cctype>

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