#include "../statichook.h"

#include <utility>
#include <cstring>
#include <map>
#include <vector>
#include "TCPServerImpl.h"
#include "../main.h"
#include "../minecraft/MinecraftGame.h"
#include "../minecraft/FilePathManager.h"
#include "../Log.h"

struct DynamicLibraryLoader {
    void* handle;
};

TClasslessInstanceHook(void, _ZN6cohtml11LibraryImpl12CreateSystemERKNS_14SystemSettingsE,
        cohtml::SystemSettings& settings) {
    settings.debuggerEnabled = true;
    settings.debuggerPort = 4243;
    original(this, settings);
}

TInstanceHook(void, _ZN6cohtml20DynamicLibraryLoaderC2EPKc, DynamicLibraryLoader, const char* name) {
    if (strcmp(name, "HttpServer") == 0) {
        handle = dlopen("libsdebugger.so", RTLD_LAZY);
        return;
    }
    original(this, name);
}


struct ResourceData {
    void* data = nullptr;
    size_t dataSize = 0;

    ResourceData() {}
    ResourceData(ResourceData&& d) {
        data = d.data;
        dataSize = d.dataSize;
        d.data = nullptr;
    }
    ~ResourceData() {
        if (data)
            free(data);
    }

    ResourceData& operator=(ResourceData&& d) {
        if (data)
            free(data);
        data = d.data;
        dataSize = d.dataSize;
        d.data = nullptr;
        return *this;
    }
};

static std::map<std::string, ResourceData> contents;

extern "C" bool GetResourceData(const char* name, void const*& res, size_t& resSize) {
    Log::trace("CohtmlDebug", "GetResourceData %s", name);
    std::string fullpath = minecraftGame->getFilePathManager().getRootPath() + name;
    auto el = contents.find(fullpath);
    if (el == contents.end()) {
        FILE* fd = fopen(fullpath.c_str(), "rb");
        ResourceData newResData;
        if (fd) {
            fseek(fd, 0, SEEK_END);
            newResData.dataSize = ftell(fd);
            fseek(fd, 0, SEEK_SET);
            newResData.data = malloc(newResData.dataSize);
            if (fread(newResData.data, newResData.dataSize, 1, fd) != 1) {
                Log::error("CohtmlDebug", "GetResourceData %s: Failed to read file", name);
                free(newResData.data);
                newResData.data = nullptr;
            }
            fclose(fd);
        }
        contents[fullpath] = std::move(newResData);
        el = contents.find(fullpath);
    }
    if (el->second.data) {
        res = el->second.data;
        resSize = el->second.dataSize;
        return true;
    }
    return false;
}

extern "C" cohtml::server::TCPServer* CreateTCPServer(cohtml::server::TCPListener& interface, unsigned short port) {
    TCPServerImpl* ret = new TCPServerImpl(interface);
    ret->Run(port);
    return ret;
}