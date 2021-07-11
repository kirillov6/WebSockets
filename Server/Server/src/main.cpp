#include "Server.h"


void EventHandler(mg_connection* conn, int ev, void* ev_data, void* fn_data)
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

        // Send back
        mg_ws_send(conn, wsMsg->data.ptr, wsMsg->data.len, WEBSOCKET_OP_TEXT);
        mg_iobuf_delete(&conn->recv, conn->recv.len);
    }
}

int main()
{
    Server serv{1000, EventHandler};
    serv.Start();
	return 0;
}