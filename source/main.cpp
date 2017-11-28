#include <json.hpp>
#include <ThreadPool.h>
#include <crow.h>

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    app.port(18080).multithreaded().run();

    return 0;
}