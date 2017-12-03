#include "game.h"

Game::Game(string gameID) : gameID(gameID), white(nullptr), black(nullptr) {
    clearState();
}

void Game::clearState() {
    turn = BLACK;

    for (auto& column : board) {
        for (auto& place : column)
            place = EMPTY;
    }
}

bool Game::attemptMove(State color, uint16_t x, uint16_t y) {
    if (turn == color && board[x][y] == EMPTY) {
        board[x][y] = color;
        turn = color == BLACK ? WHITE : BLACK;

        return true;
    } else return false;
}