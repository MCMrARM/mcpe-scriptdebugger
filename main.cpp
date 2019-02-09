#include "main.h"
#include "statichook.h"
#include "inspector/InspectorServer.h"
#include "inspector/InspectorManager.h"
#include "minecraft/Keyboard.h"

extern "C" void* mcpelauncher_hook(void* symbol, void* hook, void** original) { return nullptr; }
extern "C" void mcpelauncher_vlog(mcpelauncher_log_level level, const char* tag, const char* text, va_list args) {}

MinecraftGame* minecraftGame;

bool ON_SERVER_THREAD();

namespace ScriptApi {
    struct V8CoreInterface {
        char filler[0x48];
        v8::Isolate *isolate; // 0x4C
        v8::Persistent<v8::Context> context; // 0x50
        char filler2[0x7C - 0x50];
        InspectorServer::InspectorManager inspectorManager;
        bool waitForDebugger;
    };
    struct ScriptFramework {
        int filler;
        V8CoreInterface* core;
    };
}
struct ScriptEngine : public ScriptApi::ScriptFramework {
};

InspectorServer::InspectorServer inspectorServer;


TInstanceHook(void, _ZN9ScriptApi15V8CoreInterfaceC2Ev, ScriptApi::V8CoreInterface) {
    original(this);
    new (&inspectorManager)InspectorServer::InspectorManager;
    waitForDebugger = false;
}
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterfaceD2Ev, ScriptApi::V8CoreInterface) {
    if (isolate) {
        v8::HandleScope handleScope(isolate);
        if (ON_SERVER_THREAD())
            inspectorServer.removeInspector("server");
        else
            inspectorServer.removeInspector("client");
        inspectorManager.finalize(isolate, context.Get(isolate));
        inspectorManager.~InspectorManager();
    }
    original(this);
}
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterface10initializeERNS_12ScriptReportE, ScriptApi::V8CoreInterface, void* report) {
    original(this, report);
    v8::HandleScope handleScope (isolate);
    inspectorManager.init(isolate, context.Get(isolate));
    if (ON_SERVER_THREAD())
        inspectorServer.addInspector("server", &inspectorManager);
    else
        inspectorServer.addInspector("client", &inspectorManager);
    if (Keyboard::_states[17]) // ctrl
        inspectorManager.waitForDebugger();
}
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterface8shutdownERNS_12ScriptReportE, ScriptApi::V8CoreInterface, void* report) {
    {
        v8::HandleScope handleScope (isolate);
        inspectorManager.finalize(isolate, context.Get(isolate));
    }
    original(this, report);
}
TInstanceHook(void, _ZN12ScriptEngine6updateEv, ScriptEngine) {
    original(this);
    v8::HandleScope handleScope (core->isolate);
    core->inspectorManager.update();
}

static thread_local ScriptApi::V8CoreInterface* currentInterface;
TInstanceHook(void, _ZN9ScriptApi15V8CoreInterface9runScriptERKSsS2_RNS_12ScriptReportE, ScriptApi::V8CoreInterface, std::string const& a, std::string const& b, void* c) {
    // Remove the hash from the filename end
    auto iof = a.rfind('_');
    auto as = a.substr(0, iof);
    currentInterface = this;
    original(this, as, b, c);
    currentInterface = nullptr;
}
TClasslessInstanceHook(v8::Local<v8::Value>, _ZN2v86Script3RunENS_5LocalINS_7ContextEEE, v8::Local<v8::Context> ctx) {
    if (currentInterface)
        currentInterface->inspectorManager.pauseOnNextStatement();
    return original(this, std::move(ctx));
}


extern "C" void mod_init() {
    void* sym = dlsym(MinecraftHandle(), "_ZN9ScriptApi15ScriptFrameworkC2Ev");
    unsigned char* s = &((unsigned char*) sym)[0x22 + 3];
    if (*(int*) s != 0x7C)
        throw std::runtime_error("V8CoreInterface size changed");
    *((int*) s) = sizeof(ScriptApi::V8CoreInterface);

    sym = dlsym(MinecraftHandle(), "_ZN12ScriptEngine15ScriptQueueDataC2ERKSsS2_S2_S2_");
    s =  &((unsigned char*) sym)[0x5C];
    s[0] = 0xB8;
    const char* replacementStr = "(function() { ";
    *((size_t*) &s[1]) = (size_t) replacementStr;
    s[5] = 0x90;
    s[7] = (unsigned char) strlen(replacementStr);

    inspectorServer.start(4242);
}

extern "C" void mod_set_minecraft(MinecraftGame* game) {
    minecraftGame = game;
}