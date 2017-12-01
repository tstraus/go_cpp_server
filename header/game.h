#ifndef __GAME_H__
#define __GAME_H__

#include <crow.h>
#include <iostream>
#include <array>

using namespace std;

class Game {
public:
    string gameID;

    crow::websocket::connection* white;
    crow::websocket::connection* black;

    enum State {
        EMPTY,
        WHITE,
        BLACK
    };

    array<array<State, 19>, 19> state; // [x][y]

    Game(string gameID);

    void clearState();
};

#endif // __GAME_H__