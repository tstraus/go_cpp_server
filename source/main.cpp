#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>
#include <argh.h>
#include <rang.hpp>
#include <loguru.hpp>

#include <iostream>

using namespace std;

int main(int, char** argv) {
    auto args = argh::parser(argv);

    auto port = [&args]() -> uint16_t {
        try {
            return (uint16_t)stoul(args("--port").str());
        } catch (...) {
            return 1234;
        }
    }();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello world";
    });

    app.port(port).multithreaded().run();

    return 0;
}