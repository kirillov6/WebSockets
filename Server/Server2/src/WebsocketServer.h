#pragma once

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

class WebsocketServer
{
public:
	// Server management
	static bool Init(int port);
	static void Run();
	static void Stop();

	// Websockets management
	static bool SendData(const std::string& id, const std::string& data);
	static bool CloseWebsocket(const std::string& id);

private:
	// Get websocket
	static bool GetWebsocket(const std::string& id, websocketpp::connection_hdl &hdl);

	// Callbacks
	static bool OnValidate(websocketpp::connection_hdl hdl);
	static void OnFail(websocketpp::connection_hdl hdl);
	static void OnClose(websocketpp::connection_hdl hdl);
	
private:
	static websocketpp::server<websocketpp::config::asio> m_Server;
	static std::map<std::string, websocketpp::connection_hdl> m_Websockets;
	static std::mutex m_WebsocketsMutex;

	static bool m_bInited;
};