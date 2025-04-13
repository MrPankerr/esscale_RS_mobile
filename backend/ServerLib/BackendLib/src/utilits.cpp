#include "utilits.hpp"

bool is_integer(const std::string& s) 
{
    try {
        size_t pos;
        std::stoi(s, &pos);
        return pos == s.size(); // Проверяем что вся строка обработана
    } catch (...) {
        return false;
    }
}

std::string extract_until_space(const std::string& input) 
{
    std::string result;
    for (char c : input) {
        if (c == ' ') { break; }
        result += c;
    }
    return result;
}

void remove_until_space(std::string& str) 
{
    size_t space_pos = str.find(' ');
    if (space_pos != std::string::npos) {
        str.erase(0, space_pos + 1); // Удаляем до пробела включительно (+1 для самого пробела)
    } else {
        str.clear(); // Если пробела нет - очищаем всю строку
    }
}

//Session
void session::handler_move(message_handler&& on_message, error_handler&& on_error)
{
    this->on_message = std::move(on_message);
    this->on_error = std::move(on_error);
}

void session::start(message_handler&& on_message, error_handler&& on_error) 
{
    this->on_message = std::move(on_message);
    this->on_error = std::move(on_error);

    async_handshake();

    async_read();
}

void session::post(const std::string& message)
{
    bool idle = outgoing.empty();
    outgoing.push(message);

    if (idle) {
        async_write();
    }
}

void session::session_close()
{
    error_code ec;
    ssl_socket.shutdown(ec);
    ssl_socket.lowest_layer().close();
    on_error();
}

void session::async_handshake() 
{
    ssl_socket.async_handshake(ssl::stream_base::server,
        [self = shared_from_this()](const error_code& error) {
            if (!error) {
                self->async_read();
            } else {
                self->on_error();
            }
        });
}

tcp::socket& session::get_soc(){ return ssl_socket.next_layer(); }

//Server
void server::async_accept() 
{
    socket.emplace(io_context);

    acceptor.async_accept(*socket, [&](error_code error) {
        std::stringstream addr;

        if (socket.has_value()) 
        {
            addr << socket.value().remote_endpoint(error) << ": ";
            std::cout << addr.str() << "New connection" << std::endl;
        } 
        else 
        {
            std::cout << "Error: Socket is not initialized." << std::endl;
        }

        auto client = std::make_shared<session>(std::move(*socket), ssl_context);
        clients.insert(client);

        std::string addr_str = addr.str();

        client->start(
            [this, shared_client = client, addr_str](const std::string& message) {
                std::map<std::string, std::string> packet;
                std::string mes = message;
                packet["Addres"] = addr_str;
                if (!is_integer(extract_until_space(mes))) { packet["Packet_type"] = "404"; }
                else { packet["Packet_type"] = extract_until_space(mes); }
                remove_until_space(mes);
                packet["Message"] = mes; 
                post(packet, shared_client);
            },
            [this, weak = std::weak_ptr(client)] { 
                 if (auto shared = weak.lock(); shared && clients.erase(shared)) {
                        std::map<std::string, std::string> packet;
                        packet["Addres"] = "Server";
                        packet["Packet_type"] = "0";
                        packet["Message"] = "We are one less\n\r";
                        post(packet, shared);
                }
            });

        async_accept();
    });
}

void server::post_W(std::map<std::string, std::string> packet, std::shared_ptr<session> Client )
{
    int pt = std::stoi(packet["Packet_type"]);

    packettype pac_type = packettype(pt);
    switch (pac_type)
    {
    case P_message:
    {
        for (auto& client : clients) {
            client->post(packet["Addres"]);
            client->post(packet["Message"]);
        }
        break;
    }
    case P_closed:
    {
        Client->session_close();
        std::cout << "Disconnect: " << packet["Addres"] << std::endl;
        break;
    }
    default:
    {
        std::cout << "Unrecognized packet: " << pac_type << std::endl;
        break;
    }
    }
    
}

void server::post_E(std::map<std::string, std::string> packet, std::shared_ptr<session> Client )
{
    int pt = std::stoi(packet["Packet_type"]);

    packettype pac_type = packettype(pt);
    switch (pac_type)
    {
    case P_message:
    {
        for (auto& client : clients) {
            client->post(packet["Addres"]);
            client->post(packet["Message"]);
        }
        break;
    }
    case P_closed:
    {
        Client->session_close();
        std::cout << "Disconnect: " << packet["Addres"] << std::endl;
        break;
    }
    default:
    {
        std::cout << "Unrecognized packet: " << pac_type << std::endl;
        break;
    }
    }
    
}

void server::post(std::map<std::string, std::string> packet, std::shared_ptr<session> Client)
{
    int pt = std::stoi(packet["Packet_type"]);

    packettype pac_type = packettype(pt);
    switch (pac_type)
    {
    case P_id:
    {
        std::string id = packet["Message"];

        if(!id.empty()) { id.pop_back(); }
        if(!id.empty()) { id.pop_back(); }

        std::cout << id << ' ' << id << std::endl;

        if (ID_Employer.find(id) != ID_Employer.end()) 
        {
            Client->handler_move(
                [this, shared_client = Client, addr_str = packet["Addres"]](const std::string& message) {
                    std::map<std::string, std::string> packet;
                    std::string mes = message;
                    packet["Addres"] = addr_str;
                    if (!is_integer(extract_until_space(mes))) { packet["Packet_type"] = "404"; }
                    else { packet["Packet_type"] = extract_until_space(mes); }
                    remove_until_space(mes);
                    packet["Message"] = mes; 
                    post_E(packet, shared_client);
                },
                [this, weak = std::weak_ptr(Client)] { 
                     if (auto shared = weak.lock(); shared && clients.erase(shared)) {
                            std::map<std::string, std::string> packet;
                            packet["Addres"] = "Server";
                            packet["Packet_type"] = "0";
                            packet["Message"] = "We are one less\n\r";
                            post_E(packet, shared);
                    }
                });
        } 
        else if (ID_Worker.find(id) != ID_Worker.end()) 
        {
            Client->handler_move(
                [this, shared_client = Client, addr_str = packet["Addres"]](const std::string& message) {
                    std::map<std::string, std::string> packet;
                    std::string mes = message;
                    packet["Addres"] = addr_str;
                    if (!is_integer(extract_until_space(mes))) { packet["Packet_type"] = "404"; }
                    else { packet["Packet_type"] = extract_until_space(mes); }
                    remove_until_space(mes);
                    packet["Message"] = mes;
                    post_W(packet, shared_client);
                },
                [this, weak = std::weak_ptr(Client)] { 
                     if (auto shared = weak.lock(); shared && clients.erase(shared)) {
                            std::map<std::string, std::string> packet;
                            packet["Addres"] = "Server";
                            packet["Packet_type"] = "0";
                            packet["Message"] = "We are one less\n\r";
                            post_W(packet, shared);
                    }
                });
        } 
        else 
        {
            Client->post("This client is not registered\n");
        }

        break;
    }
    default:
    {
        std::cout << "Unrecognized packet: " << pac_type << std::endl;
        break;
    }
    }
}