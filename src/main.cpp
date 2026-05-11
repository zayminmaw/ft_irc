/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:05:23 by zmin              #+#    #+#             */
/*   Updated: 2026/05/11 21:09:29 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <cstdlib> // For std::atoi
#include <stdexcept>

// Helper to check if a string is strictly numeric
static bool isNumeric(const std::string &str)
{
	if (str.empty())
		return false;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	std::string portStr = argv[1];
	std::string password = argv[2];

	// Port Validation (Must be a number between 1 and 65535)
	if (!isNumeric(portStr))
	{
		std::cerr << "Error: Port must be a numeric value." << std::endl;
		return 1;
	}

	int port = std::atoi(portStr.c_str());
	if (port < 1 || port > 65535)
	{
		std::cerr << "Error: Port must be between 1 and 65535." << std::endl;
		return 1;
	}

	// Server Instantiation & Execution
	try
	{
		Server ircServer(port, password);

		std::cout << "Starting ircserv on port " << port << "..." << std::endl;

		// This will block and run the infinite poll() loop
		// ircServer.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}