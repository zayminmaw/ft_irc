/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandRouter.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zmin <zmin@student.42bangkok.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 19:51:39 by zmin              #+#    #+#             */
/*   Updated: 2026/05/06 20:17:48 by zmin             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef COMMANDROUTER_HPP
#define COMMANDROUTER_HPP

#include "Server.hpp"

class CommandRouter {
	private:
		Server &_server;

		CommandRouter(const CommandRouter &src);
		CommandRouter &operator=(const CommandRouter &src);
	public:
		~CommandRouter();
};

#endif