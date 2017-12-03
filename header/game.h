#ifndef __GAME_H__
#define __GAME_H__

#include <crow.h>
#include <iostream>
#include <array>

using namespace std;

class Game {
public:
    enum State {
        EMPTY,
        WHITE,
        BLACK
    };

    Game(string gameID);

    void clearState();

    bool attemptMove(State color, uint16_t x, uint16_t y);

    string gameID;

    crow::websocket::connection* white;
    crow::websocket::connection* black;

private:
    array<array<State, 19>, 19> state; // [x][y]
};

#endif // __GAME_H__