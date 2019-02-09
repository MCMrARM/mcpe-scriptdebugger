#include "InspectorWebSocketSession.h"
#include "String16.h"

using namespace InspectorServer;

void InspectorWebSocketSession::update() {
    std::vector<std::string> q;
    {
        std::lock_guard<std::mutex> g(queue_mutex);
        q = std::move(queue);
        queue.clear();
    }
    for (std::string const& str : q) {
        printf("[InspectorWebSocketSession] Got command: %s\n", str.c_str());
        session->dispatchProtocolMessage(v8_inspector::StringView((unsigned char *) str.data(), str.size()));
    }
}

void InspectorWebSocketSession::sendInspectorMessage(const v8_inspector::StringView &msg) {
    if (msg.is8Bit()) {
        std::string str((char *) msg.characters8(), msg.length());
        printf("[InspectorWebSocketSession] Send: %s[%li]\n", str.c_str(), str.size());
        sendMessage(std::string((char *) msg.characters8(), msg.length()));
    } else {
        v8_inspector::String16 str16 = v8_inspector::toString16(msg);
        std::string str = str16.utf8();
        printf("[InspectorWebSocketSession] Send: %s[%li]\n", str.c_str(), str.size());
        sendMessage(str);
    }
}