// ---------- Libraries ----------
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

// ---------- Constants ----------
const int TOTAL_CARDS = 108;
const int MAX_HAND = TOTAL_CARDS;
const int MAX_PLAYERS = 4;
const int MIN_PLAYERS = 2;

// ---------- Enums and Structures ----------
enum Color { RED, GREEN, BLUE, YELLOW, WILD };

enum Value {
    ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
    SKIP, REVERSE, PLUS2, WILD_CARD, WILD_PLUS4
};

struct Card {
    Color color;
    Value value;
};

struct Player {
    Card hand[MAX_HAND];
    int cardCount;
};

struct CardEffect {
    int drawCount;     // 0, 2, 4
    bool skipNext;     // Skip, +2, +4, Reverse (when 2 players)
    bool reverseDir;   // Reverse (when 3-4 players)
    bool chooseColor;  // Wild / Wild+4
};

// ---------- Utilities ----------
char colorToChar(Color c) {
    if (c == RED) return 'R';
    if (c == GREEN) return 'G';
    if (c == BLUE) return 'B';
    if (c == YELLOW) return 'Y';
    return 'W';
}

void printCard(const Card& c) {
    if (c.color == WILD) {
        if (c.value == WILD_CARD) cout << "Wild";
        else if (c.value == WILD_PLUS4) cout << "Wild+4";
		else cout << "Wild?"; //fallback
        return;
    }

    cout << colorToChar(c.color);

    if (c.value <= NINE) cout << (int)c.value;
    else if (c.value == SKIP) cout << "Skip";
    else if (c.value == REVERSE) cout << "Reverse";
    else if (c.value == PLUS2) cout << "+2";
}


bool isValidMove(const Card& played, const Card& topCard, Color activeColor) {
    if (played.color == WILD) return true;
    if (played.color == activeColor) return true;
    if (played.value == topCard.value) return true;
    return false;
}

void addToHand(Player& p, const Card& c) {
    p.hand[p.cardCount++] = c;
}

void removeCard(Player& p, int index) {
    for (int i = index; i < p.cardCount - 1; i++) {
        p.hand[i] = p.hand[i + 1];
    }
    p.cardCount--;
}

bool hasAnyValidMove(const Player& p, const Card& topCard, Color activeColor) {
    for (int i = 0; i < p.cardCount; i++) {
        if (isValidMove(p.hand[i], topCard, activeColor)) return true;
    }
    return false;
}

void printPlayerHand(const Player& p) {
    for (int i = 0; i < p.cardCount; i++) {
        cout << "[" << i << "] ";
        printCard(p.hand[i]);
        cout << " ";
    }
    cout << "\n";
}

void nextPlayerIndex(int& currentPlayer, int direction, int playersCount) {
    currentPlayer = (currentPlayer + direction + playersCount) % playersCount;
}

// ---------- Deck build & shuffle ----------
void pushCard(Card deck[], int& deckSize, Color color, Value value) {
    deck[deckSize].color = color;
    deck[deckSize].value = value;
    deckSize++;
}

// Correct UNO deck = 108 cards:
// For each color: 1x0, 2x(1-9), 2xSkip, 2xReverse, 2x+2  => 25 per color => 100
// Plus: 4xWild, 4xWild+4 => 8
void buildUnoDeck(Card deck[], int& deckSize) {
    deckSize = 0;
    Color colors[4] = { RED, GREEN, BLUE, YELLOW };

    for (int ci = 0; ci < 4; ci++) {
        Color c = colors[ci];

        // 1x ZERO
        pushCard(deck, deckSize, c, ZERO);

        // 2x (1..9)
        for (int v = (int)ONE; v <= (int)NINE; v++) {
            pushCard(deck, deckSize, c, (Value)v);
            pushCard(deck, deckSize, c, (Value)v);
        }

        // 2x action cards
        pushCard(deck, deckSize, c, SKIP);
        pushCard(deck, deckSize, c, SKIP);

        pushCard(deck, deckSize, c, REVERSE);
        pushCard(deck, deckSize, c, REVERSE);

        pushCard(deck, deckSize, c, PLUS2);
        pushCard(deck, deckSize, c, PLUS2);
    }

    // 4x Wild, 4x Wild+4
    for (int i = 0; i < 4; i++) pushCard(deck, deckSize, WILD, WILD_CARD);
    for (int i = 0; i < 4; i++) pushCard(deck, deckSize, WILD, WILD_PLUS4);
}

// Fisher–Yates shuffle (works everywhere)
void shuffleDeck(Card deck[], int deckSize) {
    for (int i = deckSize - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

// ---------- Discard / Refill / Draw ----------
bool refillDeckFromDiscard(Card deck[], int& deckSize, Card discard[], int& discardSize) {
    if (deckSize > 0) return true;
    if (discardSize == 0) return false;

    // move discard -> deck
    for (int i = 0; i < discardSize; i++) {
        deck[i] = discard[i];
    }
    deckSize = discardSize;
    discardSize = 0;

    shuffleDeck(deck, deckSize);
    cout << "(Deck refilled from discard pile.)\n";
    return true;
}

bool drawFromDeck(Card deck[], int& deckSize, Card discard[], int& discardSize, Card& outCard) {
    if (deckSize == 0) {
        if (!refillDeckFromDiscard(deck, deckSize, discard, discardSize)) return false;
    }
    outCard = deck[deckSize - 1];
    deckSize--;
    return true;
}

// When a player plays a card: old topCard goes to discard, new card becomes top.
void playCardFromHand(Player& p, int index, Card discard[], int& discardSize, Card& topCard, Color& activeColor) {
    discard[discardSize++] = topCard; // old top -> discard
    topCard = p.hand[index];          // new top
    if (topCard.color != WILD) activeColor = topCard.color;
    removeCard(p, index);
}

// ---------- Effects ----------
CardEffect getCardEffect(const Card& c) {
    CardEffect e;
    e.drawCount = 0;
    e.skipNext = false;
    e.reverseDir = false;
    e.chooseColor = false;

    if (c.color == WILD) {
        e.chooseColor = true;
        if (c.value == WILD_PLUS4) {
            e.drawCount = 4;
            e.skipNext = true;
        }
        return e;
    }

    if (c.value == SKIP) {
        e.skipNext = true;
    }
    else if (c.value == PLUS2) {
        e.drawCount = 2;
        e.skipNext = true;
    }
    else if (c.value == REVERSE) {
        e.reverseDir = true;
    }

    return e;
}

Color askForColorChoice() {
    while (true) {
        cout << "Choose color (R/G/B/Y): ";
        char ch;
        cin >> ch;

        if (ch == 'R' || ch == 'r') return RED;
        if (ch == 'G' || ch == 'g') return GREEN;
        if (ch == 'B' || ch == 'b') return BLUE;
        if (ch == 'Y' || ch == 'y') return YELLOW;

        cout << "Invalid color. Try again.\n";
    }
}

void applyDrawToPlayer(Player& p, Card deck[], int& deckSize, Card discard[], int& discardSize, int count) {
    for (int i = 0; i < count; i++) {
        Card drawn;
        if (drawFromDeck(deck, deckSize, discard, discardSize, drawn)) {
            addToHand(p, drawn);
        }
        else {
            cout << "No cards left to draw.\n";
            return;
        }
    }
}

// ---------- UNO rule ----------
bool checkUnoDeclaration() {
    cout << "Type 'uno' to declare UNO: ";
    char cmd[16];
    cin >> cmd;

    // accept uno/UNO/Uno/uNo etc (first 3 letters)
    if ((cmd[0] == 'u' || cmd[0] == 'U') &&
        (cmd[1] == 'n' || cmd[1] == 'N') &&
        (cmd[2] == 'o' || cmd[2] == 'O') &&
        cmd[3] == '\0') {
        return true;
    }
    return false;
}

void enforceUnoRuleIfNeeded(Player& p,
    Card deck[], int& deckSize,
    Card discard[], int& discardSize) {
    if (p.cardCount == 1) {
        bool ok = checkUnoDeclaration();
        if (!ok) {
            cout << "You forgot to declare UNO! Drawing 1 penalty card...\n";
            Card drawn;
            if (drawFromDeck(deck, deckSize, discard, discardSize, drawn)) {
                addToHand(p, drawn);
                cout << "Penalty card: ";
                printCard(drawn);
                cout << "\n";
            }
            else {
                cout << "No cards left to draw.\n";
            }
        }
        else {
            cout << "UNO declared!\n";
        }
    }
}

// ---------- Save / Load ----------
void writeCard(ofstream& out, const Card& c) {
    out << (int)c.color << " " << (int)c.value << "\n";
}

bool readCard(ifstream& in, Card& c) {
    int col, val;
    if (!(in >> col >> val)) return false;
    c.color = (Color)col;
    c.value = (Value)val;
    return true;
}

bool saveGame(const char* filename,
    Player players[], int playersCount,
    int currentPlayer, int direction,
    const Card& topCard, Color activeColor,
    Card deck[], int deckSize,
    Card discard[], int discardSize) {
    ofstream out(filename);
    if (!out.is_open()) return false;

    out << "UNO_SAVE_V1\n";
    out << playersCount << "\n";
    out << currentPlayer << " " << direction << "\n";
    out << (int)activeColor << "\n";
    writeCard(out, topCard);

    for (int i = 0; i < playersCount; i++) {
        out << players[i].cardCount << "\n";
        for (int j = 0; j < players[i].cardCount; j++) {
            writeCard(out, players[i].hand[j]);
        }
    }

    out << deckSize << "\n";
    for (int i = 0; i < deckSize; i++) {
        writeCard(out, deck[i]);
    }

    out << discardSize << "\n";
    for (int i = 0; i < discardSize; i++) {
        writeCard(out, discard[i]);
    }

    return true;
}

bool loadGame(const char* filename,
    Player players[], int& playersCount,
    int& currentPlayer, int& direction,
    Card& topCard, Color& activeColor,
    Card deck[], int& deckSize,
    Card discard[], int& discardSize) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    char header[32];
    in >> header;
    if (header[0] != 'U') return false; // very simple validation

    if (!(in >> playersCount)) return false;
    if (playersCount < MIN_PLAYERS || playersCount > MAX_PLAYERS) return false;

    if (!(in >> currentPlayer >> direction)) return false;

    int ac;
    if (!(in >> ac)) return false;
    activeColor = (Color)ac;

    if (!readCard(in, topCard)) return false;

    for (int i = 0; i < playersCount; i++) {
        if (!(in >> players[i].cardCount)) return false;
        if (players[i].cardCount < 0 || players[i].cardCount > TOTAL_CARDS) return false;
        for (int j = 0; j < players[i].cardCount; j++) {
            if (!readCard(in, players[i].hand[j])) return false;
        }
    }

    if (!(in >> deckSize)) return false;
    if (deckSize < 0 || deckSize > TOTAL_CARDS) return false;
    for (int i = 0; i < deckSize; i++) {
        if (!readCard(in, deck[i])) return false;
    }

    if (!(in >> discardSize)) return false;
    if (discardSize < 0 || discardSize > TOTAL_CARDS) return false;
    for (int i = 0; i < discardSize; i++) {
        if (!readCard(in, discard[i])) return false;
    }

    if (currentPlayer < 0 || currentPlayer >= playersCount) currentPlayer = 0;
    if (!(direction == 1 || direction == -1)) direction = 1;

    return true;
}

// ---------- Game setup ----------
void initPlayers(Player players[], int playersCount) {
    for (int i = 0; i < playersCount; i++) {
        players[i].cardCount = 0;
    }
}

void dealInitialCards(Player players[], int playersCount,
    Card deck[], int& deckSize,
    Card discard[], int& discardSize) {
    for (int r = 0; r < 7; r++) {
        for (int p = 0; p < playersCount; p++) {
            Card c;
            if (drawFromDeck(deck, deckSize, discard, discardSize, c)) {
                addToHand(players[p], c);
            }
        }
    }
}

void startTopCard(Card deck[], int& deckSize,
    Card discard[], int& discardSize,
    Card& topCard, Color& activeColor) {
    Card c;
    while (drawFromDeck(deck, deckSize, discard, discardSize, c)) {
        if (c.color != WILD) {
            topCard = c;
            activeColor = c.color;
            return;
        }
        // If wild at start, ignore it for simplicity.
        // (Alternative: put it in discard and draw another)
    }
    // fallback
    topCard.color = RED;
    topCard.value = ZERO;
    activeColor = RED;
}

// ---------- Game loop ----------
void runGameLoop(Player players[], int playersCount,
    Card deck[], int& deckSize,
    Card discard[], int& discardSize,
    Card& topCard, Color& activeColor,
    int& currentPlayer, int& direction) {
    while (true) {
        Player& p = players[currentPlayer];

        cout << "\n--- UNO ---\n";
        cout << "Current card: ";
        printCard(topCard);
        cout << "\n";

        cout << "Player " << (currentPlayer + 1) << " - Your cards:\n";
        printPlayerHand(p);

        // Win check (should happen right after play, but safe here too)
        if (p.cardCount == 0) {
            cout << "Player " << (currentPlayer + 1) << " wins!\n";
            return;
        }

        // If no valid move -> draw 1 and optionally play it
        if (!hasAnyValidMove(p, topCard, activeColor)) {
            cout << "No suitable cards. Automatically drawing 1 card...\n";

            Card drawn;
            if (!drawFromDeck(deck, deckSize, discard, discardSize, drawn)) {
                cout << "No cards left to draw.\n";
                return;
            }

            cout << "Drawn card: ";
            printCard(drawn);
            cout << "\n";

            addToHand(p, drawn);

            if (isValidMove(drawn, topCard, activeColor)) {
                cout << "You can play the drawn card. Play it now? (y/n): ";
                char ans;
                cin >> ans;

                if (ans == 'y' || ans == 'Y') {
                    int idx = p.cardCount - 1; // last card

                    CardEffect eff = getCardEffect(p.hand[idx]);

                    cout << "> You used ";
                    printCard(p.hand[idx]);
                    cout << "\n";

                    playCardFromHand(p, idx, discard, discardSize, topCard, activeColor);

                    if (eff.chooseColor) {
                        activeColor = askForColorChoice();
                    }

                    // UNO
                    enforceUnoRuleIfNeeded(p, deck, deckSize, discard, discardSize);

                    // Win
                    if (p.cardCount == 0) {
                        cout << "Player " << (currentPlayer + 1) << " wins!\n";
                        return;
                    }

                    // Reverse with 2 players = Skip
                    if (eff.reverseDir && playersCount == 2) {
                        eff.reverseDir = false;
                        eff.skipNext = true;
                    }

                    if (eff.reverseDir) direction *= -1;

                    // Apply effects to next player immediately
                    if (eff.drawCount > 0 || eff.skipNext) {
                        nextPlayerIndex(currentPlayer, direction, playersCount);
                        Player& nextP = players[currentPlayer];

                        if (eff.drawCount > 0) {
                            cout << "Player " << (currentPlayer + 1) << " draws " << eff.drawCount << " cards.\n";
                            applyDrawToPlayer(nextP, deck, deckSize, discard, discardSize, eff.drawCount);
                        }
                        if (eff.skipNext) {
                            cout << "Player " << (currentPlayer + 1) << " is skipped.\n";
                            nextPlayerIndex(currentPlayer, direction, playersCount);
                        }
                    }
                    else {
                        nextPlayerIndex(currentPlayer, direction, playersCount);
                    }

                    continue; // turn finished
                }
            }

            // If not played, next player
            nextPlayerIndex(currentPlayer, direction, playersCount);
            continue;
        }

        // Normal play: choose a card index
        cout << "Choose card index to play (or -1 to Save & Exit): ";
        int choice;
        cin >> choice;

        if (choice == -1) {
            bool ok = saveGame("save.txt",
                players, playersCount,
                currentPlayer, direction,
                topCard, activeColor,
                deck, deckSize,
                discard, discardSize);
            if (ok) cout << "Game saved to save.txt\n";
            else cout << "Failed to save game.\n";
            return;
        }

        if (choice < 0 || choice >= p.cardCount || !isValidMove(p.hand[choice], topCard, activeColor)) {
            cout << "Invalid move. Try again.\n";
            continue; // same player again
        }

        CardEffect eff = getCardEffect(p.hand[choice]);

        cout << "> You used ";
        printCard(p.hand[choice]);
        cout << "\n";

        playCardFromHand(p, choice, discard, discardSize, topCard, activeColor);

        if (eff.chooseColor) {
            activeColor = askForColorChoice();
        }

        // UNO
        enforceUnoRuleIfNeeded(p, deck, deckSize, discard, discardSize);

        // Win
        if (p.cardCount == 0) {
            cout << "Player " << (currentPlayer + 1) << " wins!\n";
            return;
        }

        // Reverse with 2 players = Skip
        if (eff.reverseDir && playersCount == 2) {
            eff.reverseDir = false;
            eff.skipNext = true;
        }

        if (eff.reverseDir) direction *= -1;

        // Apply effects to next player immediately
        if (eff.drawCount > 0 || eff.skipNext) {
            nextPlayerIndex(currentPlayer, direction, playersCount);
            Player& nextP = players[currentPlayer];

            if (eff.drawCount > 0) {
                cout << "Player " << (currentPlayer + 1) << " draws " << eff.drawCount << " cards.\n";
                applyDrawToPlayer(nextP, deck, deckSize, discard, discardSize, eff.drawCount);
            }
            if (eff.skipNext) {
                cout << "Player " << (currentPlayer + 1) << " is skipped.\n";
                nextPlayerIndex(currentPlayer, direction, playersCount);
            }
        }
        else {
            nextPlayerIndex(currentPlayer, direction, playersCount);
        }
    }
}

// ---------- Menu helpers ----------
int readMenuChoice() {
    cout << "--- UNO ---\n";
    cout << "[1] New Game\n";
    cout << "[2] Continue Game\n";
    cout << "[3] Exit\n";
    cout << "Choose: ";
    int c;
    cin >> c;
    return c;
}

int readPlayersCount() {
    int playersCount;
    while (true) {
        cout << "Enter number of players (2-4): ";
        cin >> playersCount;
        if (playersCount >= MIN_PLAYERS && playersCount <= MAX_PLAYERS) return playersCount;
        cout << "Invalid number of players.\n";
    }
}

// ---------- main ----------
int main() {
    srand((unsigned)time(0));

    Player players[MAX_PLAYERS];
    int playersCount = 0;

    Card deck[TOTAL_CARDS];
    int deckSize = 0;

    Card discard[TOTAL_CARDS];
    int discardSize = 0;

    Card topCard;
    Color activeColor = RED;

    int currentPlayer = 0;
    int direction = 1;

    int menu = readMenuChoice();
    if (menu == 3) return 0;

    if (menu == 2) {
        bool ok = loadGame("save.txt",
            players, playersCount,
            currentPlayer, direction,
            topCard, activeColor,
            deck, deckSize,
            discard, discardSize);
        if (!ok) {
            cout << "No saved game found or save file is corrupted.\n";
            return 0;
        }
        cout << "Game loaded from save.txt\n";
    }
    else {
        playersCount = readPlayersCount();

        initPlayers(players, playersCount);
        buildUnoDeck(deck, deckSize);
        shuffleDeck(deck, deckSize);

        discardSize = 0;

        dealInitialCards(players, playersCount, deck, deckSize, discard, discardSize);
        startTopCard(deck, deckSize, discard, discardSize, topCard, activeColor);

        currentPlayer = 0;
        direction = 1;
    }

    runGameLoop(players, playersCount,
        deck, deckSize,
        discard, discardSize,
        topCard, activeColor,
        currentPlayer, direction);

    cout << "Exiting...\n";
    return 0;
}

