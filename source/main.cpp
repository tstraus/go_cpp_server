#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>
#include <argh.h>
#include <rang.hpp>
#include <loguru.hpp>
#include <sole.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <array>
#include <deque>
#include <algorithm>

#include "game.h"

using namespace std;
using namespace argh;
using namespace rang;
using namespace sole;
using json = nlohmann::json;

mutex mtx;

unordered_map<string, shared_ptr<Game>> games;
deque<string> unmatchedGames;

int main(int, char** argv) {
    auto args = parser(argv);

    auto serverMode = args["--server"];

    auto port = [&args]() -> uint16_t {
        try {
            return (uint16_t)stoul(args("--port").str());
        } catch (...) {
            return 1234;
        }
    } ();

    auto host = [&args]() -> string {
        auto host = args("--host").str();

        if (host == "") {
            return "127.0.0.1";
        } else {
            return host;
        }
    } ();

    crow::SimpleApp app;
    crow::mustache::set_base("./templates/");

    CROW_ROUTE(app, "/")([&host, &port, &serverMode]() {
        crow::mustache::context mustache;
        stringstream hostStream;

        if (serverMode) {
            hostStream << "tstraus.net";
        } else {
            hostStream << host << ":" << port;
        }

        mustache["host"] = hostStream.str();

        return crow::mustache::load("go.html").render(mustache);
    });

    CROW_ROUTE(app, "/go")
        .websocket()
            .onopen([&](crow::websocket::connection& conn) {
                lock_guard<mutex> lock(mtx);

                if (unmatchedGames.empty()) { // creating a new game
                    auto game = make_shared<Game>(uuid4().str());
                    game->black = &conn;
                    games[game->gameID] = game;

                    json newGame = {
                        {"gameID", game->gameID},
                        {"action", "newGame"},
                        {"data", {
                            {"black", true}
                        }}
                    };
                    conn.send_text(newGame.dump());

                    cout << fg::green << "New Game: " << style::reset << game->gameID << endl;

                    unmatchedGames.push_back(game->gameID);
                } else { // joining an existing game
                    auto game = games[unmatchedGames.front()];
                    bool black;

                    if (game->white == nullptr) {
                        game->white = &conn;
                        black = false;
                    } else {
                        game->black = &conn;
                        black = true;
                    }

                    json stones = json::array();
                    for (int y = 0; y < 19; y++) {
                        for (int x = 0; x < 19; x++) {
                            auto state = game->checkState(x, y);
                            if (state != Game::EMPTY) {
                                stones.push_back({
                                    {"color", state == Game::BLACK ? "b" : "w"},
                                    {"x", x},
                                    {"y", y}
                                });
                            }
                        }
                    }

                    json joinGame = {
                        {"gameID", unmatchedGames.front()},
                        {"action", "joinGame"},
                        {"data", {
                            {"black", black},
                            {"stones", stones}
                        }}
                    };
                    conn.send_text(joinGame.dump());

                    cout << fg::green << "Joined Game: " << style::reset << game->gameID << endl;

                    unmatchedGames.pop_front();
                }

            })
            .onclose([&](crow::websocket::connection& conn, const string& /*reason*/) {
                lock_guard<mutex> lock(mtx);

                json msg = {
                    {"action", "chat"},
                    {"data", {
                        {"msg", "opponent lost connection, waiting for new player..."}
                    }}
                };

                string gameToRemove;

                // remove dead keys
                for (auto it = begin(games); it != end(games);) {
                    if (it->second == nullptr) {
                        it = games.erase(it);
                    } else ++it;
                }

                // search for the game with the lost connection
                for (auto& game : games) {
                    // remove connection from game
                    if (game.second->white == &conn) {
                        game.second->white = nullptr;
                        msg["gameID"] = game.second->gameID;
                        unmatchedGames.push_back(game.second->gameID);
                    } else if (game.second->black == &conn) {
                        game.second->black = nullptr;
                        msg["gameID"] = game.second->gameID;
                        unmatchedGames.push_back(game.second->gameID);
                    }
            
                    // send message to other player in game
                    if (game.second->black != nullptr) {
                        cout << fg::yellow << "Lost White: " << style::reset << game.second->gameID << endl;

                        game.second->black->send_text(msg.dump());
                    } else if (game.second->white != nullptr) {
                        cout << fg::yellow << "Lost Black : " << style::reset << game.second->gameID << endl;

                        game.second->white->send_text(msg.dump());
                    }

                    // game no longer has any players
                    if (game.second->white == nullptr && game.second->black == nullptr)
                        gameToRemove = game.second->gameID;
                }

                // remove game if it no longer has any players
                if (!gameToRemove.empty()) {
                    cout << fg::yellow << "Removing Game: " << style::reset << gameToRemove << endl;

                    games.erase(gameToRemove);

                    unmatchedGames.erase(
                        remove_if(unmatchedGames.begin(), unmatchedGames.end(), [&gameToRemove](string game) {
                            return game == gameToRemove;
                        }), unmatchedGames.end()
                    );
                }

                // figure out how to set the pointer of the player to nullptr in games
                cout << fg::red << "Connection Lost: " << style::reset << &conn << endl;
            })
            .onmessage([&](crow::websocket::connection& /*conn*/, const string& data, bool is_binary) {
                lock_guard<mutex> lock(mtx);

                if (!is_binary) {
                    cout << "Msg: " << data << endl;

                    auto msg = json::parse(data);

                    auto action = msg["action"];
                    auto gameID = msg["gameID"];
                    auto game = games[gameID];

                    if (game != nullptr) {
                        if (action=="chat") {
                            auto game = games[gameID];

                            // forward chat message
                            if (game->white!=nullptr && game->black!=nullptr) {
                                game->white->send_text(data);
                                game->black->send_text(data);
                            }
                        }

                        else if (action=="attemptMove") {
                            auto game = games[gameID];

                            auto color = msg["data"]["black"] ? Game::State::BLACK : Game::State::WHITE;
                            auto x = msg["data"]["x"];
                            auto y = msg["data"]["y"];

                            // attempt to make the received move
                            if (game->white!=nullptr && game->black!=nullptr && game->attemptMove(color, x, y)) {
                                json move = {
                                    {"gameID", gameID},
                                    {"action", "move"},
                                    {"data", {
                                        {"black", msg["data"]["black"]},
                                        {"x", x},
                                        {"y", y}
                                    }}
                                };

                                auto output = move.dump();

                                // send the completed move to both clients
                                game->white->send_text(output);
                                game->black->send_text(output);
                            }
                        }
                    }
                }
            });

    app.bindaddr(host).port(port).multithreaded().run();

    return 0;
}
