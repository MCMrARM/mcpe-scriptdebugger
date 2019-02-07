#pragma once

#include "Listener.h"
#include "HttpSessionHandler.h"
#include <mutex>

namespace InspectorServer {

class InspectorManager;

class InspectorServer : public HttpSessionHandler {

private:
    Listener listener;

    std::mutex inspectorManagersMutex;
    std::map<std::string, InspectorManager*> inspectorManagers;

public:
    InspectorServer() : listener(*this) {}

    void onRequest(HttpSession& session, HttpSession::Request const& req) override;

    void addInspector(std::string const &name, InspectorManager *mgr);

    void removeInspector(std::string const &name);

    void start(unsigned short port) {
        listener.start(port);
    }

};

}