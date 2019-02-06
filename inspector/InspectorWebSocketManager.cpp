#include "InspectorWebSocketManager.h"

#include <thread>
#include "InspectorManager.h"

void InspectorWebSocketManager::start(unsigned short port) {
    server.config.port = port;

    auto& endp = server.endpoint["^/inspector/([a-z]+)/?$"];

    endp.on_open = [this](std::shared_ptr<WsServer::Connection> connection) {
        std::lock_guard<std::mutex> lck(channelsMutex);
        auto mgr = inspectorManagers.find(connection->path_match[1].str());
        if (mgr == inspectorManagers.end()) {
            connection->send_close(0, "Not found");
            return;
        }

        printf("Opened debugging session for %s\n", connection->path_match[1].str().c_str());
        std::unique_ptr<InspectorWebSocketChannel> channel(new InspectorWebSocketChannel(connection));
        mgr->second->onConnectionOpened(*channel);
        channels[connection] = std::move(channel);
    };
    endp.on_message = [this](std::shared_ptr<WsServer::Connection> connection,
                             std::shared_ptr<WsServer::InMessage> in_message) {
        std::lock_guard<std::mutex> lck(channelsMutex);
        channels.at(connection)->postMessage(in_message->string());
    };

    std::thread server_thread([this]() {
        // Start WS-server
        server.start();
    });
    server_thread.detach();
}

void InspectorWebSocketManager::addInspector(std::string const &name, InspectorManager *mgr) {
    std::lock_guard<std::mutex> lck(channelsMutex);
    inspectorManagers[name] = mgr;
}

void InspectorWebSocketManager::removeInspector(std::string const &name) {
    std::lock_guard<std::mutex> lck(channelsMutex);
    inspectorManagers.erase(name);
}