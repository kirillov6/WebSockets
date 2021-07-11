#pragma once

// Include Mongoose
#include "../lib/mongoose.h"

class Server
{
public:
    Server(int port);
    ~Server();

    void Start();

private:
    static void EventHandler(mg_connection* conn, int ev, void* ev_data, void* fn_data);

private:
    int m_Port; // Port
    mg_mgr m_MgManager; // Event manager
};