#include "Server.h"
#include <string>
#include <iostream>


namespace
{
    void Log(const std::string& str) { std::cout << str << std::endl; }
}

Server::Server(int port)
{
    std::string listen_on = "http://localhost:" + std::to_string(port);

    mg_mgr_init(&m_MgManager);

    auto conn = mg_http_listen(&m_MgManager, listen_on.c_str(), EventHandler, nullptr);
    if (!conn)
        Log("Failed to create listener");

    Log("Server started at " + listen_on);
}

Server::~Server()
{
    mg_mgr_free(&m_MgManager);
}

void Server::Start()
{
    for (;;)
        mg_mgr_poll(&m_MgManager, 1000);
}

void Server::EventHandler(mg_connection* conn, int ev, void* ev_data, void* fn_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        auto httpMsg = static_cast<mg_http_message*>(ev_data);
        if (mg_http_match_uri(httpMsg, "/websocket"))
            mg_ws_upgrade(conn, httpMsg, nullptr); // Upgrade to websocket
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // Got websocket frame
        auto wsMsg = static_cast<mg_ws_message*>(ev_data);

        std::string msg{ wsMsg->data.ptr };
        size_t len = wsMsg->data.len;

        std::string addText = " haha";

        msg.insert(len, addText);
        len += addText.length();

        // Send back
        mg_ws_send(conn, msg.c_str(), len, WEBSOCKET_OP_TEXT);
        mg_iobuf_delete(&conn->recv, conn->recv.len);
    }
}
