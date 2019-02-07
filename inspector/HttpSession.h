#pragma once

#include <memory>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace InspectorServer {

class HttpSessionHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {

public:
    using Request = boost::beast::http::request<boost::beast::http::string_body>;

private:
    using tcp = boost::asio::ip::tcp;

    tcp::socket tcpSocket;
    boost::beast::flat_buffer buffer;
    Request req;
    HttpSessionHandler& handler;

    void onRead(boost::beast::error_code ec, std::size_t);

    void onWrite(boost::beast::error_code ec, bool close);

public:
    HttpSession(tcp::socket s, HttpSessionHandler& handler) : tcpSocket(std::move(s)), handler(handler) {}

    tcp::socket& getSocket() { return tcpSocket; }

    void run();


    template <typename Response>
    void sendResponse(Response res) {
        auto self = shared_from_this();
        using response_type = typename std::decay<decltype(res)>::type;
        auto sp = std::make_shared<response_type>(std::forward<decltype(res)>(res));
        boost::beast::http::async_write(tcpSocket, *sp,
                                        [self, sp](boost::beast::error_code ec, std::size_t bytes) {
                                            self->onWrite(ec, sp->need_eof());
                                        });
    }

};

}