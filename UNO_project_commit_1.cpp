/**
*
* Solution to course project # 4
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2025/2026
*
* @author Rangel Parishev
* @idnumber 0MI0600668
* @compiler VS
*
* <c++ file with project>
*
*/
#include <iostream>
#include <fstream>

int main()
{
	std::cout << "--- UNO ---" << std::endl;

	int numOfPlayers;
	do
	{
		std::cout << "Enter number of players (2-4): ";
		std::cin >> numOfPlayers;
		if (numOfPlayers < 2 || numOfPlayers > 4)
		{
			std::cout << "Invalid number of players. Please try again." << std::endl;
		}
	} while (numOfPlayers < 2 || numOfPlayers > 4);
}
