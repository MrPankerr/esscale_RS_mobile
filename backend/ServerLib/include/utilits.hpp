#ifndef _LIBRARY_UTILITS_H_
#define _LIBRARY_UTILITS_H_

#if defined (_WIN32) || defined (_WIN64)

    #define MYLIB_EXPORT __declspec(dllexport)
    #define MYLIB_IMPORT __declspec(dllimport)

#else
    #define MYLIB_EXPORT __attribute__((visibility("default")))
    #define MYLIB_IMPORT __attribute__((visibility("default")))
    #define MYLIB_HIDDEN __attribute__((visibility("hidden")))
#endif

#include <iostream>
#include <optional>
#include <queue>
#include <unordered_set>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <random>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include "Data.hpp"

namespace io = boost::asio;
namespace ssl = io::ssl;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;
using namespace std::placeholders;

using error_handler = std::function<void()>;
using message_handler = std::function<void(const std::string&)>;

//Enumeration of package types
enum packettypein{
    P_authorization,
    P_registration,
    P_id,
    P_message,
    P_confirmation,
    P_closed
};
  
MYLIB_EXPORT bool is_integer(const std::string& s);

MYLIB_EXPORT std::string extract_until_space(const std::string& input);

MYLIB_EXPORT void remove_until_space(std::string& str);

MYLIB_EXPORT void gen_id(std::string id);

MYLIB_EXPORT bool check_packet(int numb, const std::string& packet);

//A class for handling a separate connection
class MYLIB_EXPORT session: public std::enable_shared_from_this<session> {
    private:
        ssl::stream<tcp::socket> ssl_socket;
        io::streambuf streambuf;
        std::queue<std::string> outgoing;
        message_handler on_message;
        error_handler on_error;

        void async_read() 
        {
            io::async_read_until(
                ssl_socket,
                streambuf,
                "\n",
                std::bind(&session::on_read, shared_from_this(), _1, _2));
        }
        void on_read(error_code error, std::size_t bytes_transferred) 
        {
            if (!error) 
            {
                std::stringstream addr, message;
                message << std::istream(&streambuf).rdbuf();
                streambuf.consume(bytes_transferred);
                on_message(message.str());
                async_read();
            } 
            else 
            {
                error_code ec;
                ssl_socket.shutdown(ec);
                ssl_socket.lowest_layer().close();
                on_error();
            }
        }

        void async_write() 
        {
            io::async_write(
                ssl_socket,
                io::buffer(outgoing.front()),
                std::bind(&session::on_write, shared_from_this(), _1, _2));
        }
        void on_write(error_code error, std::size_t bytes_transferred) 
        {
            if (!error) 
            {
                outgoing.pop();
    
                if (!outgoing.empty()) {
                    async_write();
                }
            } 
            else 
            {
                error_code ec;
                ssl_socket.shutdown(ec);
                ssl_socket.lowest_layer().close();
                on_error();
            }
        }
    
    public:
        MYLIB_EXPORT session(tcp::socket&& socket, ssl::context& ssl_ctx) :
        ssl_socket(std::move(socket), ssl_ctx)
        {
            // Настройка SSL
            ssl_socket.set_verify_mode(ssl::verify_peer);
            ssl_socket.set_verify_callback(
                [](bool preverified, ssl::verify_context& ctx) {
                    return true; // Или ваша собственная логика проверки
                });
        }

        MYLIB_EXPORT void handler_move(message_handler&& on_message, error_handler&& on_error);
        MYLIB_EXPORT void start(message_handler&& on_message, error_handler&& on_error);
        MYLIB_EXPORT void post(const std::string& message);
        MYLIB_EXPORT void async_handshake();
        MYLIB_EXPORT void session_close();
        MYLIB_EXPORT tcp::socket& get_soc();
  };
  
  //A class for handling all connections
  class MYLIB_EXPORT server {
    private:
        io::io_context& io_context;
        tcp::acceptor acceptor;
        std::optional<tcp::socket> socket;
        std::unordered_map<std::string, std::shared_ptr<session>> clients;
        ssl::context ssl_context;

    public:
        MYLIB_EXPORT server(io::io_context& io_context, std::uint16_t port) :
            io_context(io_context),
            acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
            ssl_context(ssl::context::tlsv13) {
        
                // Настройка SSL-контекста
                ssl_context.set_options(
                    ssl::context::no_sslv2 |
                    ssl::context::no_sslv3 |
                    ssl::context::no_tlsv1 |
                    ssl::context::no_tlsv1_1 |
                    ssl::context::no_tlsv1_2);
                
                SSL_CTX_set_ciphersuites(ssl_context.native_handle(), 
                    "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256");

                SSL_CTX_set1_curves_list(ssl_context.native_handle(), "prime256v1");
                
                ssl_context.use_certificate_chain_file("../TLSsecret/server.crt");
                ssl_context.use_private_key_file("../TLSsecret/server.key", ssl::context::pem);
            }
  
        MYLIB_EXPORT void async_accept();
        MYLIB_EXPORT void post_W(std::map<std::string, std::string> packet, std::shared_ptr<session> Client);
        MYLIB_EXPORT void post_E(std::map<std::string, std::string> packet, std::shared_ptr<session> Client);
        MYLIB_EXPORT void post(std::map<std::string, std::string> packet, std::shared_ptr<session> Client);
        MYLIB_EXPORT void change_on_message(std::string Type_of_user, std::string addres, std::shared_ptr<session> Client);
  };

  #endif /* _LIBRARY_UTILITS_H_ */