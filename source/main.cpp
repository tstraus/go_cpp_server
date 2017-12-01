#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>
#include <argh.h>
#include <rang.hpp>
#include <loguru.hpp>

#include <iostream>
#include <unordered_set>
#include <mutex>
#include <unordered_map>
#include <array>

using namespace std;

mutex mtx;
unordered_set<crow::websocket::connection*> users;

enum State {
    EMPTY,
    WHITE,
    BLACK
};

unordered_map<string, array<array<State, 19>, 19>> games; // [x][y]

bool clearGame(string id) {
    if (games.count(id)) {
        auto game = games[id];
        for (auto column : game)
            column.empty();

        return true;
    } else {
        return false;
    }
}

int main(int, char** argv) {
    auto args = argh::parser(argv);

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

        return crow::mustache::load("ws.html").render(mustache);
    });

    CROW_ROUTE(app, "/ws")
            .websocket()
            .onopen([&](crow::websocket::connection& conn) {
                cout << "new connection" << endl;
                lock_guard<mutex> lock(mtx);

                users.insert(&conn);
            })
            .onclose([&](crow::websocket::connection& conn, const string& reason) {
                cout << "connection lost: " << reason << endl;
                lock_guard<mutex> lock(mtx);

                users.erase(&conn);
            })
            .onmessage([&](crow::websocket::connection& /*conn*/, const string& data, bool is_binary) {
                cout << "data: " << data << endl;
                lock_guard<mutex> lock(mtx);

                for (auto u : users)
                {
                    if (is_binary)
                        u->send_binary(data);
                    else
                        u->send_text(data);
                }
            });

    app.bindaddr(host).port(port).multithreaded().run();

    return 0;
}