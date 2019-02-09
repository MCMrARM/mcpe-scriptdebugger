#pragma once

#include <cstdlib>

namespace cohtml {

    namespace server {

        struct TCPListener {
            virtual ~TCPListener() {}
            virtual void OnConnectionAccepted(int) = 0;
            virtual void OnConnectionFailed(int) = 0;
            virtual void OnReadData(int,char const*,unsigned int,bool) = 0;
            virtual int RecvDataChunkCallback(char const*,unsigned int) = 0;
        };

        struct TCPServer {
            virtual ~TCPServer() {}
            virtual void Update() = 0;
            virtual void SendData(int fd, const char* data, size_t len) = 0;
            virtual void Unknown1() {}
            virtual void CloseConnection(int fd) = 0;
            virtual void Unknown2() {}
        };

    }

    struct SystemSettings {
        char filler[0x34];
        int debuggerPort;
        bool debuggerEnabled;
    };

}