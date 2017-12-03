#include "game.h"

Game::Game(string gameID) : gameID(gameID), white(nullptr), black(nullptr) {
    clearState();
}

void Game::clearState() {
    for (auto& column : state) {
        for (auto& place : column)
            place = EMPTY;
    }
}

bool Game::attemptMove(State color, uint16_t x, uint16_t y) {
    if (state[x][y] == EMPTY) {
        state[x][y] = color;

        return true;
    } else return false;
}