#pragma once

#include "HttpSession.h"

namespace InspectorServer {

class HttpSessionHandler {

public:
    virtual void onRequest(HttpSession& session, HttpSession::Request const& req) = 0;

};

}