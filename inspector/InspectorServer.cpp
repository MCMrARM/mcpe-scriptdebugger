#include "InspectorServer.h"
#include "InspectorManager.h"
#include "InspectorWebSocketSession.h"
#include "../extra/StdPolyfills.h"
#include <nlohmann/json.hpp>
#include "../Log.h"

namespace beast = boost::beast;

using namespace InspectorServer;

#define SERVER_NAME "InspectorServer"

void ::InspectorServer::InspectorServer::onRequest(HttpSession &session,
                                                   const HttpSession::Request &req) {
    auto url = req.target();
    if (url.ends_with('/'))
        url.remove_suffix(1);
    Log::trace("InspectorServer", "Request: %s %s", url.to_string().c_str(), req.method_string().to_string().c_str());
    if (url.starts_with("/inspector/") && beast::websocket::is_upgrade(req)) {
        auto name = url.substr(sizeof("/inspector/") - 1);
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
        Log::trace("InspectorServer", "Accepted inspector for %s", name.to_string().c_str());
        auto wsSession = std::make_shared<InspectorWebSocketSession>(std::move(session.getSocket()));
        inspector->second->onConnectionOpened(*wsSession);
        wsSession->run(req);
    } else if (url == "/json" || url == "/json/list" || url == "/json/version") {
        nlohmann::json val;
        if (req.target() == "/json/version") {
            val["Browser"] = "node.js/v10.15.1";
            val["Protocol-Version"] = "1.1";
        } else {
            val = nlohmann::json::array();
            std::lock_guard<std::mutex> lck(inspectorManagersMutex);
            for (auto const& inspector : inspectorManagers) {
                nlohmann::json e;
                std::string wsUrl = "127.0.0.1:" + std::to_string(port) + "/inspector/" + inspector.first;
                e["description"] = "Minecraft script engine";
                e["devtoolsFrontendUrl"] = "chrome-devtools://devtools/bundled/js_app.html?experiments=true&v8only=true&ws=" + wsUrl;
                e["devtoolsFrontendUrlCompat"] = "chrome-devtools://devtools/bundled/inspector.html?experiments=true&v8only=true&ws=" + wsUrl;
                e["faviconUrl"] = "http://mrarm.io/u/mcpelauncher-icon.png";
                e["id"] = inspector.first;
                e["title"] = inspector.first;
                e["type"] = "node";
                e["url"] = "minecraft://" + inspector.first;
                e["webSocketDebuggerUrl"] = "ws://" + wsUrl;
                val.push_back(std::move(e));
            }
        }

        beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
        res.set(beast::http::field::server, SERVER_NAME);
        res.set(beast::http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = val.dump();
        session.sendResponse(res);
    } else if (url == "") {
        std::stringstream body;
        body << "<!doctype html><html><head><title>Minecraft Script Engine Debugger</title></head><body>";
        body << "<h1>Minecraft Script Engine</h1>";
        body << "<ul>";
        {
            std::lock_guard<std::mutex> lck(inspectorManagersMutex);
            for (auto const& inspector : inspectorManagers) {
                std::string wsUrl = "127.0.0.1:" + std::to_string(port) + "/inspector/" + inspector.first;
                std::string devtoolsUrlBase = "chrome-devtools://devtools/bundled/js_app.html?experiments=true&v8only=true&ws=";
                body << "<li>" << devtoolsUrlBase << "<strong>" << wsUrl << "</strong>" << "</li>";
            }
            if (inspectorManagers.empty())
                body << "<li>No scripts are loaded</li>";
        }
        body << "</ul>";
        body << "</body></html>";

        beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
        res.set(beast::http::field::server, SERVER_NAME);
        res.set(beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = body.str();
        session.sendResponse(res);
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