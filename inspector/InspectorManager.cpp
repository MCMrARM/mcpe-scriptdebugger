#include "InspectorManager.h"
#include "InspectorWebSocketChannel.h"
#include "InspectorWebSocketManager.h"

void InspectorManager::init(v8::Isolate *isolate, v8::Local<v8::Context> context) {
    if (inspector)
        throw std::runtime_error("InspectorManager already initialized");
    inspector = v8_inspector::V8Inspector::create(isolate, this);
    const char* namec = "Primary Context";
    v8_inspector::StringView name((unsigned char*) namec, strlen(namec));
    inspector->contextCreated(v8_inspector::V8ContextInfo(context, 1, name));
}

void InspectorManager::onConnectionOpened(InspectorWebSocketChannel &channel) {
    channel.setSession(inspector->connect(1, &channel, v8_inspector::StringView()));
    channel.setPostMessageCallback([this]() {
        std::lock_guard<std::mutex> lck2 (runMessageLoopMutex);
        runMessageLoopCv.notify_all();
    });
    {
        std::lock_guard<std::mutex> lck(runMessageLoopMutex);
        channels[channel.getConnection().get()] = &channel;
    }
}

void InspectorManager::update() {
    std::lock_guard<std::mutex> lck (runMessageLoopMutex);
    for (auto const& channel : channels)
        channel.second->update();
}


void InspectorManager::runMessageLoopOnPause(int contextGroupId) {
    std::unique_lock<std::mutex> lck (runMessageLoopMutex);
    runMessageLoopBool = true;
    while (runMessageLoopBool) {
        {
            std::lock_guard<std::mutex> lck2(channelsMutex);
            for (auto const &channel : channels)
                channel.second->update();
        }
        if (runMessageLoopBool)
            runMessageLoopCv.wait(lck);
    }
}

void InspectorManager::quitMessageLoopOnPause() {
    // same thread
    runMessageLoopBool = false;
}