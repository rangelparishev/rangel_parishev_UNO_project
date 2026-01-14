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
#include <algorithm>
#include <random>
#include <ctime>

const int TOTAL_CARDS = 108;
const int MAX_HAND = TOTAL_CARDS;
const int MAX_CARDS_IN_DECK = TOTAL_CARDS;
const int MAX_PLAYERS = 4;
const int MIN_PLAYERS = 2;

enum Color 
{ 
	RED, GREEN, BLUE, YELLOW, WILD 
};

enum Value
{
	ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
	SKIP, REVERSE, PLUS2, WILD_CARD, WILD_PLUS4
};

struct Card 
{
	Color color;
	Value value;
};

struct Player 
{
	Card hand[MAX_HAND];
	int cardCount;
};

Card deck[MAX_CARDS_IN_DECK];
int deckSize;

Player players[MAX_PLAYERS];
int playersCount;

Card topCard;
Color activeColor;
int currentPlayer;
int direction = 1;

void printCard(Card c) 
{
	if (c.color == RED) std::cout << "R";
	else if (c.color == GREEN) std::cout << "G";
	else if (c.color == BLUE) std:: cout << "B";
	else if (c.color == YELLOW) std::cout << "Y";
	else std::cout << "W";

	if (c.value <= NINE) std::cout << (int)c.value;
	else if (c.value == SKIP) std::cout << "Skip";
	else if (c.value == REVERSE) std::cout << "Reverse";
	else if (c.value == PLUS2) std::cout << "+2";
	else if (c.value == WILD_CARD) std::cout << "ild";
	else if (c.value == WILD_PLUS4) std::cout << "+4";
}

bool isValidMove(Card played) 
{
	if (played.color == WILD) return true;
	if (played.color == activeColor) return true;
	if (played.value == topCard.value) return true;
	return false;
}

void drawCard(Player& p) {
	if (deckSize == 0) return;
	p.hand[p.cardCount++] = deck[--deckSize];
}

void nextPlayer() {
	currentPlayer = (currentPlayer + direction + playersCount) % playersCount;
}

int main()
{
	std::cout << "--- UNO ---" << std::endl;

	int numOfPlayers;
	bool playersCondition;
	// Input number of players with validation
	do
	{
		std::cout << "Enter number of players (" << MIN_PLAYERS << "-" << MAX_PLAYERS << "): ";
		std::cin >> numOfPlayers;
		playersCondition = numOfPlayers >= MIN_PLAYERS && numOfPlayers <= MAX_PLAYERS;
		if (!playersCondition)
		{
			std::cout << "Invalid number of players. Please try again." << std::endl;
		}
	} while (!playersCondition);

	return 0;
}
