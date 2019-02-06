#pragma once

#include <thread>
#include "WebSockets.h"
#include "InspectorWebSocketChannel.h"

class InspectorManager;
class InspectorWebSocketChannel;

class InspectorWebSocketManager {

private:
    using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

    std::map<std::string, InspectorManager*> inspectorManagers;
    std::map<std::shared_ptr<WsServer::Connection>, std::unique_ptr<InspectorWebSocketChannel>> channels;
    std::mutex channelsMutex;

public:
    WsServer server;

    void addInspector(std::string const& name, InspectorManager* mgr);

    void removeInspector(std::string const& name);

    void start(unsigned short port);

};