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

// libraries
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <ctime>

// constants and structures
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

// printing
char colorToChar(Color c)
{
	if (c == RED) return 'R';
	if (c == GREEN) return 'G';
	if (c == BLUE) return 'B';
	if (c == YELLOW) return 'Y';
	return 'W';
}

void printCard(const Card& c)
{
	if (c.color == WILD)
	{
		if (c.value == WILD_CARD) std::cout << "Wild";
		else if (c.value == WILD_PLUS4) std::cout << "Wild+4";
		else std::cout << "W?"; // fallback
		return;
	}

	std::cout << colorToChar(c.color);

	if (c.value <= NINE) std::cout << (int)c.value;
	else if (c.value == SKIP) std::cout << "Skip";
	else if (c.value == REVERSE) std::cout << "Reverse";
	else if (c.value == PLUS2) std::cout << "+2";
}

// rules
bool isValidMove(const Card& played, const Card& topCard, Color activeColor)
{
	if (played.color == WILD) return true;
	if (played.color == activeColor) return true;
	if (played.value == topCard.value) return true;
	return false;
}

bool hasAnyValidMove(const Player& p, const Card& topCard, Color activeColor)
{
	for (int i = 0; i < p.cardCount; i++)
	{
		if (isValidMove(p.hand[i], topCard, activeColor)) return true;
	}
	return false;
}

// hand operations
void addToHand(Player& p, const Card& c)
{
	p.hand[p.cardCount++] = c;
}

void removeCard(Player& p, int index)
{
	for (int i = index; i < p.cardCount - 1; i++)
	{
		p.hand[i] = p.hand[i + 1];
	}
	p.cardCount--;
}

// deck building (108 cards)
void pushCard(Card deck[], int& deckSize, Color color, Value value)
{
	deck[deckSize].color = color;
	deck[deckSize].value = value;
	deckSize++;
}

void buildUnoDeck(Card deck[], int& deckSize)
{
	deckSize = 0;

	// for each color: 1x0, 2x(1-9), 2xSkip, 2xReverse, 2x+2, 1xWild, 1xWild+4
	Color colors[4] = { RED, GREEN, BLUE, YELLOW };

	for (int ci = 0; ci < 4; ci++)
	{
		Color c = colors[ci];

		// one ZERO
		pushCard(deck, deckSize, c, ZERO);

		// two of 1-9
		for (Value v = ONE; v <= NINE; v = (Value)((int)v + 1))
		{
			pushCard(deck, deckSize, c, v);
			pushCard(deck, deckSize, c, v);
		}

		// two of each action card
		pushCard(deck, deckSize, c, SKIP);
		pushCard(deck, deckSize, c, SKIP);

		pushCard(deck, deckSize, c, REVERSE);
		pushCard(deck, deckSize, c, REVERSE);

		pushCard(deck, deckSize, c, PLUS2);
		pushCard(deck, deckSize, c, PLUS2);

		// one of each wild card per color
		pushCard(deck, deckSize, WILD, WILD_CARD);
		pushCard(deck, deckSize, WILD, WILD_PLUS4);
	}
}

void shuffleDeck(Card deck[], int deckSize)
{
	// Allowed: <algorithm> + <random>
	unsigned seed = (unsigned)time(0);
	std::default_random_engine rng(seed);
	shuffle(deck, deck + deckSize, rng);
}

// We treat deck as a stack: draw from the end
bool drawFromDeck(Card deck[], int& deckSize, Card& outCard)
{
	if (deckSize <= 0) return false;
	outCard = deck[deckSize - 1];
	deckSize--;
	return true;
}

// input validation for number of players
void playersCountInput(int& playersCount)
{
	bool playersCondition = playersCount >= MIN_PLAYERS && playersCount <= MAX_PLAYERS;
	do
	{
		std::cout << "Enter number of players (" << MIN_PLAYERS << "-" << MAX_PLAYERS << "): ";
			std::cin >> playersCount;
		playersCondition = playersCount >= MIN_PLAYERS && playersCount <= MAX_PLAYERS;
		if (!playersCondition)
		{
			std::cout << "Invalid number of players. Please try again." << std::endl;
		}
	} while (!playersCondition);
}

int main()
{
	srand((unsigned)time(0));

	// defining cards, decks and players by using global structures
	Card deck[MAX_CARDS_IN_DECK];
	int deckSize = 0;

	Player players[MAX_PLAYERS];
	int playersCount;

	Card topCard;
	Color activeColor;
	int currentPlayer = 0;
	int direction = 1;

	// interface title
	std::cout << "--- UNO ---" << std::endl;

	playersCountInput(playersCount);

	return 0;
}
