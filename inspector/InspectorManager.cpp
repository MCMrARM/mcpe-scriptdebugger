#include "InspectorManager.h"
#include "InspectorWebSocketSession.h"
#include "Listener.h"

using namespace InspectorServer;


void InspectorManager::init(v8::Isolate *isolate, v8::Local<v8::Context> context) {
    if (inspector)
        throw std::runtime_error("InspectorManager already initialized");
    inspector = v8_inspector::V8Inspector::create(isolate, this);
    const char* namec = "Primary Context";
    v8_inspector::StringView name((unsigned char*) namec, strlen(namec));
    inspector->contextCreated(v8_inspector::V8ContextInfo(context, 1, name));
}

void InspectorManager::finalize(v8::Isolate *isolate, v8::Local<v8::Context> context) {
    std::set<InspectorWebSocketSession*> channels;
    {
        std::lock_guard<std::mutex> lck(channelsMutex);
        channels = std::move(this->channels);
    }
    channels.clear();
    inspector->contextDestroyed(context);
    inspector.reset();
}

void InspectorManager::onConnectionOpened(InspectorWebSocketSession &channel) {
    channel.setSession(inspector->connect(1, &channel, v8_inspector::StringView()));
    channel.setPostMessageCallback([this]() {
        std::lock_guard<std::mutex> lck2 (runMessageLoopMutex);
        runMessageLoopCv.notify_all();
    });
    channel.setCloseCallback([this, &channel]() {
        onConnectionClosed(channel);
    });
    {
        std::lock_guard<std::mutex> lck(channelsMutex);
        channels.insert(&channel);
    }
}

void InspectorManager::onConnectionClosed(InspectorServer::InspectorWebSocketSession &channel) {
    std::lock_guard<std::mutex> lck(channelsMutex);
    channels.erase(&channel);
}

void InspectorManager::update() {
    std::lock_guard<std::mutex> lck (channelsMutex);
    for (auto const& channel : channels)
        channel->update();
}


void InspectorManager::runMessageLoopOnPause(int contextGroupId) {
    std::unique_lock<std::mutex> lck (runMessageLoopMutex);
    runMessageLoopBool = true;
    while (runMessageLoopBool) {
        {
            std::lock_guard<std::mutex> lck2(channelsMutex);
            for (auto const &channel : channels)
                channel->update();
        }
        if (runMessageLoopBool)
            runMessageLoopCv.wait(lck);
    }
}

void InspectorManager::quitMessageLoopOnPause() {
    // same thread
    runMessageLoopBool = false;
}

void InspectorManager::waitForDebugger() {
    runMessageLoopOnPause(-1);
}

void InspectorManager::runIfWaitingForDebugger(int contextGroupId) {
    // same thread
    runMessageLoopBool = false;
}

void InspectorManager::pauseOnNextStatement() {
    std::lock_guard<std::mutex> lck (channelsMutex);
    for (auto const& channel : channels)
        channel->getSession()->schedulePauseOnNextStatement(v8_inspector::StringView(), v8_inspector::StringView());
}