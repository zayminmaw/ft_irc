/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 12:00:00 by zmin              #+#    #+#             */
/*   Updated: 2026/05/07 12:00:00 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

volatile bool Server::_shutdownRequested = false;

Server::Server(int port, const std::string& password)
	: _serverFd(-1)
	, _port(port)
	, _password(password)
	, _name("ircserv")
	, _router(0)
{
}

Server::~Server()
{
}
