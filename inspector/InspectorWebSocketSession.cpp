#include "InspectorWebSocketSession.h"
#include "String16.h"
#include "../Log.h"

using namespace InspectorServer;

void InspectorWebSocketSession::update() {
    std::vector<std::string> q;
    {
        std::lock_guard<std::mutex> g(queue_mutex);
        q = std::move(queue);
        queue.clear();
    }
    for (std::string const& str : q) {
        Log::trace("InspectorWebSocketSession", "Receive: %s", str.c_str());
        session->dispatchProtocolMessage(v8_inspector::StringView((unsigned char *) str.data(), str.size()));
    }
}

void InspectorWebSocketSession::sendInspectorMessage(const v8_inspector::StringView &msg) {
    if (msg.is8Bit()) {
        std::string str((char *) msg.characters8(), msg.length());
        Log::trace("InspectorWebSocketSession", "Send: %s", str.c_str());
        sendMessage(std::string((char *) msg.characters8(), msg.length()));
    } else {
        v8_inspector::String16 str16 = v8_inspector::toString16(msg);
        std::string str = str16.utf8();
        Log::trace("InspectorWebSocketSession", "Send: %s", str.c_str());
        sendMessage(str);
    }
}