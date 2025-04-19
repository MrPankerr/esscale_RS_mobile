#include "utilits.hpp"


//----------------------------------Auxiliary functions----------------------------------------------//


//checking whether the contents of a string are an int number. For correct ID reading
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

//the function runs through the contents of the line 
//to the first space and outputs the contents of this line to the space. For correct ID reading
std::string extract_until_space(const std::string& input) 
{
    std::string result;
    for (char c : input) {
        if (c == ' ') { break; }
        result += c;
    }
    return result;
}

//deletes everything that was in the line before the first space. 
//To extract a text message without an ID from a string
void remove_until_space(std::string& str) 
{
    size_t space_pos = str.find(' ');
    if (space_pos != std::string::npos) {
        str.erase(0, space_pos + 1); // Удаляем до пробела включительно (+1 для самого пробела)
    } else {
        str.clear(); // Если пробела нет - очищаем всю строку
    }
}

//A function that randomly generates an employee ID
void gen_id(std::string id)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(0, 100000);
    int random_number = distr(gen);

    id = std::to_string(random_number);
    if(ID_Worker.find(id) != ID_Worker.end()) { gen_id(id); }
}

//A function that checks the completeness of the package
bool check_packet(int numb, const std::string& packet)
{
    int counter = 0;
    for(char c : packet)
    {
        if (c == ' ') { counter++; }
    }
    if (counter == numb) { return true; }
    else { return false; }
}



//-------------------------------------Session Methods-----------------------------------------------//


//Function for changing event handlers
void session::handler_move(message_handler&& on_message, error_handler&& on_error)
{
    this->on_message = std::move(on_message);
    this->on_error = std::move(on_error);
}

//Asynchronous reading of information sent by the client
void session::start(message_handler&& on_message, error_handler&& on_error) 
{
    this->on_message = std::move(on_message);
    this->on_error = std::move(on_error);

    async_handshake();

    async_read();
}


//A function for sending a message to a client
void session::post(const std::string& message)
{
    bool idle = outgoing.empty();
    outgoing.push(message);

    if (idle) {
        async_write();
    }
}

//A function for closing a connection with a client
void session::session_close()
{
    error_code ec;
    ssl_socket.shutdown(ec);
    ssl_socket.lowest_layer().close();
    on_error();
}

//Function for TLS handshake
void session::async_handshake() 
{
    ssl_socket.async_handshake(ssl::stream_base::server,
        [self = shared_from_this()](const error_code& error) {
            if (!error) {
                self->async_read();
            } else {
                error_code ec;
                self->ssl_socket.shutdown(ec);
                self->ssl_socket.lowest_layer().close();
                self->on_error();
            }
        });
}

//A function for extracting a socket from a class
tcp::socket& session::get_soc(){ return ssl_socket.next_layer(); }


//-------------------------------------Server Methods-------------------------------------------------//


//Connection processing function
void server::async_accept() 
{
    socket.emplace(io_context);

    acceptor.async_accept(*socket, [&](error_code error) {
        std::stringstream addr;

        if (socket.has_value()) 
        {
            addr << socket.value().remote_endpoint(error);
            std::cout << addr.str() << "New connection" << std::endl;
        } 
        else 
        {
            std::cout << "Error: Socket is not initialized." << std::endl;
        }
        std::string addr_str = addr.str();

        auto client = std::make_shared<session>(std::move(*socket), ssl_context);
        clients[addr_str] = client;

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
            [this, weak = std::weak_ptr(client), addr_str] { 
                 if (auto shared = weak.lock(); shared && clients.erase(addr_str)) {
                        std::cout << "We are one less\n\r";
                }
            });

        async_accept();
    });
}

//A function for interacting with workers
void server::post_W(std::map<std::string, std::string> packet, std::shared_ptr<session> Client )
{
    int pt = std::stoi(packet["Packet_type"]);

    packettypein pac_type = packettypein(pt);
    switch (pac_type)
    {
    case P_message:
    {
        for (auto& client : clients) {
            client.second->post(packet["Addres"]);
            client.second->post(packet["Message"]);
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

//A function for interacting with employers
void server::post_E(std::map<std::string, std::string> packet, std::shared_ptr<session> Client )
{
    int pt = std::stoi(packet["Packet_type"]);

    packettypein pac_type = packettypein(pt);
    switch (pac_type)
    {
    case P_message:
    {
        for (auto& client : clients) {
            client.second->post(packet["Addres"]);
            client.second->post(packet["Message"]);
        }
        break;
    }
    case P_confirmation:
    {
        if(!check_packet(8, packet["Message"]))
        {
            Client->post("Wrong Packet\n\r");
            break;
        }

        std::string conf = extract_until_space(packet["Message"]);
        remove_until_space(packet["Message"]);
        std::string Addr = extract_until_space(packet["Message"]);
        remove_until_space(packet["Message"]);

        if(conf == "No") { clients[Addr]->post("Your employer refused to register you.\n\r"); break; }

        std::string points = packet["Message"];
        std::string name = extract_until_space(points);
        remove_until_space(points);
        std::string firstname = extract_until_space(points);
        remove_until_space(points);
        std::string login = extract_until_space(points);
        remove_until_space(points);
        std::string log_employer = extract_until_space(points);
        remove_until_space(points);
        std::string password = extract_until_space(points);
        remove_until_space(points);
        std::string Number_of_point = extract_until_space(points);  //The names of points must be sent without spaces. Space separates the names of different points
        remove_until_space(points);
        int num_pt = std::stoi(Number_of_point);
        std::vector<std::string> Name_point = {Number_of_point};
        std::string buff;
        for(int i = 0; i < num_pt; i++)
        {
            buff = extract_until_space(points);
            remove_until_space(points);
            Name_point.push_back(buff);
        }

        std::string id_W;
        gen_id(id_W);

        Log_Pas[login] = {password, id_W, "W"};
        ID_Worker[id_W] = {Addr};
        Name_point.push_back(name);
        Name_point.push_back(firstname);
        Info_Worker[login] = Name_point;

        change_on_message("W", Addr, clients[Addr]);

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

//A function for authorization and registration of clients
void server::post(std::map<std::string, std::string> packet, std::shared_ptr<session> Client )
{
    int pt = std::stoi(packet["Packet_type"]);

    packettypein pac_type = packettypein(pt);
    switch (pac_type)
    {
    case P_authorization:
    {
        if(!check_packet(1, packet["Message"]))
        {
            Client->post("Wrong Packet\n\r");
            break;
        }
        std::string password = packet["Message"];
        std::string login = extract_until_space(password);
        remove_until_space(password);

        if(!password.empty()) { password.pop_back(); }
        if(!password.empty()) { password.pop_back(); }

        auto it = Log_Pas.find(login);
        if(it != Log_Pas.end())
        {
            std::string id = it->second[1];
            if(it->second[0] == password)
            {
                Client->post("Ok\n");
                if(it->second[2] == "E")
                {
                    auto it_id = ID_Employer.find(id);
                    if(it_id != ID_Employer.end())
                    {
                        int count = 0;
                        for(std::string addres : it_id->second)
                        {
                            if(addres == packet["Addres"]) { count++; }
                        }
                        if(!count) { it_id->second.push_back(packet["Addres"]); }
                    }
                    else
                    {
                        std::vector addres = {packet["Addres"]};
                        ID_Employer[id] = addres;
                    }
                }
                else
                {
                    auto it_id = ID_Worker.find(id);
                    if(it_id != ID_Worker.end())
                    {
                        int count = 0;
                        for(std::string addres : it_id->second)
                        {
                            if(addres == packet["Addres"]) { count++; }
                        }
                        if(!count) { it_id->second.push_back(packet["Addres"]); }
                    }
                    else
                    {
                        std::vector addres = {packet["Addres"]};
                        ID_Worker[id] = addres;
                    }
                }
                change_on_message(it->second[2], packet["Addres"], Client);
            }
            else { Client->post("Incorrect login or password\n"); }
        }
        else { Client->post("Incorrect login or password\n"); }

        break;
    }
    case P_registration:
    {
        if(!check_packet(6, packet["Message"]))
        {
            Client->post("Wrong Packet\n\r");
            break;
        }
        std::string points = packet["Message"];
        std::string name = extract_until_space(points);
        remove_until_space(points);
        std::string firstname = extract_until_space(points);
        remove_until_space(points);
        std::string login = extract_until_space(points);
        if(Log_Pas.find(login) != Log_Pas.end())
        {
            Client->post("This user is already registered\n");
            break;
        }
        remove_until_space(points);
        std::string log_employer = extract_until_space(points);
        auto it = Log_Pas.find(log_employer);
        if(it == Log_Pas.end()||it->second[2] != "E")
        {
            Client->post("This employer does not exist\n");
            break;
        }

        std::vector<std::string> Addres = ID_Employer.find(it->second[1])->second;
        std::stringstream user, user_info;
        user << name << ' ' << firstname;
        user_info << packet["Addres"] << " " << packet["Message"];
        for (std::string Addr : Addres)
        {
            if (clients.find(Addr) != clients.end())
            {
                clients[Addr]->post("An worker ");
                clients[Addr]->post(user.str());
                clients[Addr]->post(" wants to register for you\n\r");
                clients[Addr]->post(user_info.str());
            }
        }

        break;
    }
    default:
    {
        Client->post("No\n");
        std::cout << "Unrecognized packet: " << pac_type << std::endl;
        break;
    }
    }
}

//A method that modifies the event handler by user type
void server::change_on_message(std::string Type_of_user, std::string addres, std::shared_ptr<session> Client)
{
    if(Type_of_user == "E") 
    {
        Client->handler_move(
            [this, shared_client = Client, addr_str = addres](const std::string& message) {
                std::map<std::string, std::string> packet;
                std::string mes = message;
                packet["Addres"] = addr_str;
                if (!is_integer(extract_until_space(mes))) { packet["Packet_type"] = "404"; }
                else { packet["Packet_type"] = extract_until_space(mes); }
                remove_until_space(mes);
                packet["Message"] = mes; 
                post_E(packet, shared_client);
            },
            [this, weak = std::weak_ptr(Client), addr_str = addres] { 
                if (auto shared = weak.lock(); shared && clients.erase(addr_str)) {
                       std::cout << "We are one less\n\r";
                }
            });
    } 
    else if (Type_of_user == "W") 
    {
        Client->handler_move(
            [this, shared_client = Client, addr_str = addres](const std::string& message) {
                std::map<std::string, std::string> packet;
                std::string mes = message;
                packet["Addres"] = addr_str;
                if (!is_integer(extract_until_space(mes))) { packet["Packet_type"] = "404"; }
                else { packet["Packet_type"] = extract_until_space(mes); }
                remove_until_space(mes);
                packet["Message"] = mes;
                post_W(packet, shared_client);
            },
            [this, weak = std::weak_ptr(Client), addr_str = addres] { 
                if (auto shared = weak.lock(); shared && clients.erase(addr_str)) {
                       std::cout << "We are one less\n\r";
               }
            });
    } 
    else 
    {
        Client->post("This client is not registered\n");
    }
}