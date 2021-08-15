#include "WebsocketServer.h"
#include "Utils.h"
#include <thread>
#include <chrono>
#include <future>


// Static initialization
websocketpp::server<websocketpp::config::asio> WebsocketServer::m_Server;
std::map<std::string, websocketpp::connection_hdl> WebsocketServer::m_Websockets;
std::mutex WebsocketServer::m_WebsocketsMutex;
std::mutex WebsocketServer::m_SendMutex;
bool WebsocketServer::m_bInited = false;


bool WebsocketServer::Init(int port)
{
	if (m_bInited)
		return true;

	m_bInited = true;

	websocketpp::lib::error_code ec;

	// Init server and asio
	m_Server.init_asio(ec);
	if (ec)
	{
		std::cerr << "Error on init_asio: " << ec.message();
		return false;
	}

	// Register handlers
	m_Server.set_validate_handler(&WebsocketServer::OnValidate);
	m_Server.set_fail_handler(&WebsocketServer::OnFail);
	m_Server.set_close_handler(&WebsocketServer::OnClose);
	m_Server.set_message_handler(&WebsocketServer::OnMessage);

	// Start listen
	try
	{
		m_Server.listen(port);
		std::cout << "Server start listening on port " << port << std::endl;
	}
	catch (const websocketpp::exception& e)
	{
		std::cerr << "Error on listen: " << e.what();
		return false;
	}

	// Start accept websockets
	m_Server.start_accept(ec);
	if (ec)
	{
		std::cerr << "Error on start_accept: " << ec.message();
		return false;
	}

	return true;
}

void WebsocketServer::Run()
{
	if (!m_bInited)
		return;

	try
	{
		m_Server.run();
	}
	catch (const websocketpp::exception& e)
	{
		std::cerr << "Error on run: " << e.what();
		return;
	}
}

void WebsocketServer::Stop()
{
	websocketpp::lib::error_code ec;

	// Stop listen
	m_Server.stop_listening(ec);
	if (ec)
	{
		std::cerr << "Error on stop_listening: " << ec.message();
		return;
	}

	// Close all existing websockets
	for (auto it = m_Websockets.begin(); it != m_Websockets.end(); ++it)
	{
		m_Server.close(it->second, websocketpp::close::status::normal, "", ec);
		if (ec)
			std::cerr << "Error on close websocket " << it->first << ": " << ec.message();
	}

	// Stop server
	m_Server.stop();
}

bool WebsocketServer::SendData(const std::string& id, const std::string& data)
{
	// Get websocket with id
	websocketpp::connection_hdl hdl;
	if (!GetWebsocket(id, hdl))
		return false;

	// Send data
	return SendData(hdl, data);
}

bool WebsocketServer::SendData(websocketpp::connection_hdl hdl, const std::string& data)
{
	std::lock_guard<std::mutex> Guard(m_SendMutex);

	// Send data
	websocketpp::lib::error_code ec;
	m_Server.send(hdl, data, websocketpp::frame::opcode::text, ec);
	if (ec)
	{
		std::cerr << "Error on send data: " << ec.message();
		return false;
	}

	return true;
}

bool WebsocketServer::CloseWebsocket(const std::string& id)
{
	// Get websocket with id
	websocketpp::connection_hdl hdl;
	if (!GetWebsocket(id, hdl))
		return false;

	// Close websocket
	websocketpp::lib::error_code ec;
	m_Server.close(hdl, websocketpp::close::status::normal, "close request", ec);
	if (ec)
		std::cerr << "Error on close websocket " << id << ": " << ec.message();

	// Remove websocket from map
	std::lock_guard<std::mutex> Guard(m_WebsocketsMutex);
	m_Websockets.erase(id);

	return true;
}

bool WebsocketServer::GetWebsocket(const std::string& id, websocketpp::connection_hdl& hdl)
{
	std::lock_guard<std::mutex> Guard(m_WebsocketsMutex);
	for (const auto& elem : m_Websockets)
		if (elem.first == id)
		{
			hdl = elem.second;
			return true;
		}

	return false;
}

bool WebsocketServer::OnValidate(websocketpp::connection_hdl hdl)
{
	std::string id = "";
	if (!GetIdFromHdl(hdl, id))
		return false; // Reject validation

	// Insert into map
	std::lock_guard<std::mutex> Guard(m_WebsocketsMutex);
	if (m_Websockets.find(id) != m_Websockets.end())
		return false;

	m_Websockets.insert(std::make_pair(id, hdl));
	return true;
}

void WebsocketServer::OnFail(websocketpp::connection_hdl hdl)
{
	auto con = m_Server.get_con_from_hdl(hdl);
	auto ec = con->get_ec();
	std::cerr << ec.message();
}

void WebsocketServer::OnClose(websocketpp::connection_hdl hdl)
{
	std::string id = "";
	if (!GetIdFromHdl(hdl, id))
		return;

	// Remove websocket from map
	std::lock_guard<std::mutex> Guard(m_WebsocketsMutex);
	m_Websockets.erase(id);
}

void WebsocketServer::OnMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg)
{
	auto message = msg->get_payload();

	if (message == "ping")
		SendData(hdl, "pong");

	if (message == "closeme")
	{
		std::string id = "";
		GetIdFromHdl(hdl, id);
		CloseWebsocket(id);
	}

	if (message == "startproc")
	{
		//for (int i = 0; i < 5; ++i)
		//{
		//	std::thread{
		//		[&]() {
		//		auto start = std::chrono::high_resolution_clock::now();
		//		// Sleep for 1-3 seconds
		//		std::this_thread::sleep_for(std::chrono::seconds(rand() % 3 + 1));
		//		auto end = std::chrono::high_resolution_clock::now();
		//		std::chrono::duration<double, std::milli> elapsed = end - start;

		//		std::string response = "I slept " + std::to_string(elapsed.count()) + " ms";
		//		SendData(hdl, response);
		//		}
		//	}.detach();
		//}
	}
}

bool WebsocketServer::GetIdFromHdl(websocketpp::connection_hdl hdl, std::string& id)
{
	id = "";

	auto con = m_Server.get_con_from_hdl(hdl);
	auto uri = con->get_uri();
	std::string query = uri->get_query();

	if (!query.empty())
	{
		auto tokens = utils::SplitString(query, "&");
		size_t pos;
		for (const auto& token : tokens)
			if ((pos = token.find("id=")) != std::string::npos)
			{
				id = token.substr(3);
				break;
			}
		if (id.empty())
			return false;
	}
	else
		return false;

	return true;
}
