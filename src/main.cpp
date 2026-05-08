/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wmin-kha <wmin-kha@student.42bangkok.co    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 20:05:23 by zmin              #+#    #+#             */
/*   Updated: 2026/05/08 19:38:59 by wmin-kha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

int main(void)
{
	std::cout << "Hello world";
}

// #include "Client.hpp"
// #include <iostream>

// int main() {
//     std::cout << "--- Starting Client Phase 1 Test ---\n" << std::endl;

//     // incoming connection on FD 4
//     Client testClient(4, "192.168.1.100");

//     // Test Getters
//     std::cout << "Initial FD: " << testClient.getFd() << std::endl;
//     std::cout << "Initial IP: " << testClient.getIp() << std::endl;

//     // Test Registration State (Should be false initially)
//     std::cout << "\n--- Testing Initial State ---" << std::endl;
//     std::cout << "Has Nick? " << (testClient.hasNick() ? "Yes" : "No") << std::endl;
//     std::cout << "Has User? " << (testClient.hasUser() ? "Yes" : "No") << std::endl;
//     std::cout << "Is Registered? " << (testClient.isRegistered() ? "Yes" : "No") << std::endl;

//     // Set Data (Simulating a user sending NICK and USER commands)
//     std::cout << "\n--- Setting Nick and User ---" << std::endl;
//     testClient.setNick("the_hacker");
//     testClient.setUser("hacker_dude");
//     testClient.markPass();
//     testClient.markRegistered();

//     // Verify New State
//     std::cout << "New Nick: " << testClient.getNick() << std::endl;
//     std::cout << "New User: " << testClient.getUser() << std::endl;
//     std::cout << "Has Pass? " << (testClient.hasPass() ? "Yes" : "No") << std::endl;
//     std::cout << "Is Registered? " << (testClient.isRegistered() ? "Yes" : "No") << std::endl;

//     // Test the crucial Prefix Formatter
//     std::cout << "\n--- Testing Prefix Formatter ---" << std::endl;
//     std::cout << "Generated Prefix: " << testClient.getPrefix() << std::endl;
    
//     // Expected Output: the_hacker!hacker_dude@192.168.1.100
//     if (testClient.getPrefix() == "the_hacker!hacker_dude@192.168.1.100") {
//         std::cout << "✅ Prefix format is CORRECT!" << std::endl;
//     } else {
//         std::cout << "❌ Prefix format is INCORRECT!" << std::endl;
//     }

//     std::cout << "\n--- Test Complete ---" << std::endl;
//     return 0;
// }