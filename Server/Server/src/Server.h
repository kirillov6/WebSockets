#pragma once

// Include Mongoose
#include "../lib/mongoose.h"

class Server
{
public:
    Server(int port, mg_event_handler_t eventHandler);
    ~Server();

    void Start();

private:
    int m_Port; // Port
    mg_mgr m_MgManager; // Event manager
};