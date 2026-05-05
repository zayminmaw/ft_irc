/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:05:21 by zmin              #+#    #+#             */
/*   Updated: 2026/05/05 22:10:56 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>

#include <poll.h>

#include "Channel.hpp"
#include "Client.hpp"

class Server {
	private:
		int	_running;
		int _sock;

		const std::string	_host;
		const std::string 	_port;
		const std::string	_pass;
		
		std::vector<pollfd> _pollfds;
		std::vector<Channel *> _channels;
		std::map<int, Client *> _clients;

		Server();
		Server(const Server &src);
	public:
		Server(const std::string &port, const std::string &pass);
		~Server();
};

#endif