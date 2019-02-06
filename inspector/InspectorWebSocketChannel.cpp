#include "InspectorWebSocketChannel.h"
#include "String16.h"

void InspectorWebSocketChannel::update() {
    std::vector<std::string> q;
    {
        std::lock_guard<std::mutex> g(queue_mutex);
        q = std::move(queue);
        queue.clear();
    }
    for (std::string const& str : q) {
        printf("[InspectorWebSocketChannel] Got command: %s\n", str.c_str());
        session->dispatchProtocolMessage(v8_inspector::StringView((unsigned char *) str.data(), str.size()));
    }
}

void InspectorWebSocketChannel::sendMessage(const v8_inspector::StringView &msg) {
    if (msg.is8Bit()) {
        std::string str((char *) msg.characters8(), msg.length());
        printf("Send: %s[%li]\n", str.c_str(), str.size());
        connection->send(std::string((char *) msg.characters8(), msg.length()));
    } else {
        v8_inspector::String16 str16 = v8_inspector::toString16(msg);
        std::string str = str16.utf8();
        printf("Send: %s[%li]\n", str.c_str(), str.size());
        connection->send(str);
    }
}