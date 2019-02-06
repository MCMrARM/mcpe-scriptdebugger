#pragma once

#include "WebSockets.h"
#include <v8-inspector.h>

class InspectorWebSocketChannel;
class InspectorWebSocketManager;

class InspectorManager : protected v8_inspector::V8InspectorClient {

private:
    using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

    std::unique_ptr<v8_inspector::V8Inspector> inspector;
    std::map<WsServer::Connection*, InspectorWebSocketChannel*> channels;
    std::mutex channelsMutex;


    std::condition_variable runMessageLoopCv;
    std::mutex runMessageLoopMutex;
    bool runMessageLoopBool = true;


protected:
    void runMessageLoopOnPause(int contextGroupId) override;

    void quitMessageLoopOnPause() override;

public:
    static InspectorManager instance;

    void init(v8::Isolate* isolate, v8::Local<v8::Context> context);

    void update();


    void onConnectionOpened(InspectorWebSocketChannel& channel);

};