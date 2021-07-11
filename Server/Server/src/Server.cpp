#include "Server.h"
#include <string>
#include <iostream>


Server::Server(int port, mg_event_handler_t eventHandler)
{
    std::string listen_on = "http://localhost:" + std::to_string(port);

    mg_mgr_init(&m_MgManager);

    auto conn = mg_http_listen(&m_MgManager, listen_on.c_str(), eventHandler, nullptr);
    if (!conn)
        std::cout << "Failed to create listener";

    std::cout << "Server started at " + listen_on;
}

Server::~Server()
{
    mg_mgr_free(&m_MgManager);
    std::cout << "Server stopped";
}

void Server::Start()
{
    for (;;)
        mg_mgr_poll(&m_MgManager, 1000);
}
