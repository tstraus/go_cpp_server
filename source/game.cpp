#include "game.h"

Game::Game(string gameID) : gameID(gameID), white(nullptr), black(nullptr) {
    clearState();
}

void Game::clearState() {
    for (auto& column : board) {
        for (auto& place : column)
            place = EMPTY;
    }
}

bool Game::attemptMove(State color, uint16_t x, uint16_t y) {
    if (board[x][y] == EMPTY) {
        board[x][y] = color;

        return true;
    } else return false;
}