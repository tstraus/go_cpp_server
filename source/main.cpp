#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>
#include <argh.h>
#include <rang.hpp>
#include <loguru.hpp>
#include <sole.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <array>

#include "game.h"

using namespace std;
using namespace argh;
using namespace rang;
using namespace sole;
using json = nlohmann::json;

mutex mtx;

unordered_map<string, shared_ptr<Game>> games;

int main(int, char** argv) {
    auto args = parser(argv);

    auto port = [&args]() -> uint16_t {
        try {
            return (uint16_t)stoul(args("--port").str());
        } catch (...) {
            return 1234;
        }
    } ();

    auto host = [&args]() -> string {
        auto host = args("--ip").str();

        if (host == "") {
            return "127.0.0.1";
        } else {
            return host;
        }
    } ();

    crow::SimpleApp app;
    crow::mustache::set_base("./templates/");

    CROW_ROUTE(app, "/")([&host, &port]() {
        crow::mustache::context mustache;
        mustache["host"] = host;
        mustache["port"] = port;

        return crow::mustache::load("go.html").render(mustache);
    });

    CROW_ROUTE(app, "/ws")
        .websocket()
            .onopen([&](crow::websocket::connection& conn) {
                lock_guard<mutex> lock(mtx);

                auto game = make_shared<Game>(uuid4().str());
                game->white = &conn;
                game->black = &conn; // for now...
                games[game->gameID] = game;

                json newGame = {
                        {"action", "newGame"},
                        {"data", {
                            {"gameID", game->gameID}
                        }}
                };
                conn.send_text(newGame.dump());

                cout << fg::green << "New Game: " << style::reset << game->gameID << endl;
            })
            .onclose([&](crow::websocket::connection& /*conn*/, const string& reason) {
                lock_guard<mutex> lock(mtx);

                // figure out how to set the pointer of the player to nullptr in games
                cout << fg::red << "Connection Lost: " << style::reset << reason << endl;
            })
            .onmessage([&](crow::websocket::connection& /*conn*/, const string& data, bool is_binary) {
                lock_guard<mutex> lock(mtx);

                if (!is_binary) {
                    cout << "Msg: " << data << endl;

                    auto msg = json::parse(data);

                    if (msg["action"] == "chat") {
                        auto game = games[msg["gameID"]];

                        if (game->white != nullptr && game->black != nullptr) {
                            game->white->send_text(data);
                            game->black->send_text(data);
                        }
                    }
                }
            });

    app.bindaddr(host).port(port).multithreaded().run();

    return 0;
}