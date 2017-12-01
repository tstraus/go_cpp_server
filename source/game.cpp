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