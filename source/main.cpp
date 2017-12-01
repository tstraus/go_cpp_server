#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>
#include <argh.h>
#include <rang.hpp>
#include <loguru.hpp>
#include <httplib.h>

#include <iostream>
#include <unordered_set>
#include <mutex>

using namespace std;

mutex mtx;
unordered_set<crow::websocket::connection*> users;

int main(int, char** argv) {
    auto args = argh::parser(argv);

    auto port = [&args]() -> uint16_t {
        try {
            return (uint16_t)stoul(args("--port").str());
        } catch (...) {
            return 1234;
        }
    }();

    char name[256];
    gethostname(name, 256);

    crow::SimpleApp app;
    crow::mustache::set_base("./templates/");

    CROW_ROUTE(app, "/")([&name, &port]() {
        crow::mustache::context mustache;
        mustache["servername"] = name;
        mustache["port"] = port;
        //mustache["board"] = "http://tstra.us/images/goBoard.svg";
        mustache["board"] = "board.svg";
        mustache["go"] = "http://tstra.us/scripts/go.js";

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

    app.port(port).multithreaded().run();

    return 0;
}