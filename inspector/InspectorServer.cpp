#include "InspectorServer.h"
#include "InspectorManager.h"
#include "InspectorWebSocketSession.h"

namespace beast = boost::beast;

using namespace InspectorServer;

#define SERVER_NAME "InspectorServer"

void ::InspectorServer::InspectorServer::onRequest(HttpSession &session,
                                                   const HttpSession::Request &req) {
    if (req.target().starts_with("/inspector/") && beast::websocket::is_upgrade(req)) {
        auto name = req.target().substr(sizeof("/inspector/") - 1);
        if (name.ends_with('/'))
            name.remove_suffix(1);
        std::lock_guard<std::mutex> lck(inspectorManagersMutex);
        auto inspector = inspectorManagers.find(name.to_string());
        if (inspector == inspectorManagers.end()) {
            beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
            res.set(beast::http::field::server, SERVER_NAME);
            res.set(beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "<!doctype html><head><title>No such inspector</title></head><body><h1>The specified inspector has not been found</h1></body></html>";
            res.prepare_payload();
            session.sendResponse(res);
            return;
        }
        printf("Accepted inspector for %s\n", name.to_string().c_str());
        auto wsSession = std::make_shared<InspectorWebSocketSession>(std::move(session.getSocket()));
        inspector->second->onConnectionOpened(*wsSession);
        wsSession->run(req);
    } else {
        beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
        res.set(beast::http::field::server, SERVER_NAME);
        res.set(beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "<!doctype html><head><title>Not found</title></head><body><h1>Resource not found</h1></body></html>";
        res.prepare_payload();
        session.sendResponse(res);
    }
}

void ::InspectorServer::InspectorServer::addInspector(std::string const &name, InspectorManager *mgr) {
    std::lock_guard<std::mutex> lck(inspectorManagersMutex);
    inspectorManagers[name] = mgr;
}

void ::InspectorServer::InspectorServer::removeInspector(std::string const &name) {
    std::lock_guard<std::mutex> lck(inspectorManagersMutex);
    inspectorManagers.erase(name);
}