#pragma once
#include <string>
#include <SDL2/SDL_net.h>
#include <cstdint>
#include <functional>
#include <queue>
#include <utility>
#include <exception>

constexpr Uint16 LOBBY_PORT = 13800, GAME_PORT = 13801, INIT_CONNECT_PORT = 13802;
std::string IPv4_to_str_no_port(const IPaddress &IP);
std::string IPv4_to_str(const IPaddress &IP);

class Connection
{
    enum class PacketFlag: uint8_t{standard, admin_command, command, ping_send, ping_ack, disconnect};
    constexpr static int MAX_PACKET_SIZE = 1400;
    constexpr static double FLUSH_INTERVAL = 0.01;
    constexpr static double AUTO_PING_INTERVAL = 0.5;
    constexpr static double CONNECTION_DISCONNECT_TIME = 10;
    constexpr static unsigned PING_WINDOW = 5000; //keep the last x ms of pings to get a running average
    constexpr static double DISCONNECT_SUCCESS = -1000, DISCONNECT_INACTIVE = -1001, DISCONNECT_ERROR = -1002; //flags
    static unsigned IDcounter;
    double last_activity_time, last_ping_sent_time, last_flush_time;
    unsigned ping_sum;
    std::queue<std::pair<unsigned, unsigned> > ping_times;
    static void finalize_packet(std::string &data, PacketFlag flag);
    void send_packet(std::string &data, PacketFlag flag);
    void send_ping();
    void send_ping_ack(std::string &data);
    std::string send_packet_buffer;
public:
    TCPsocket socket;
    IPaddress IP;
    unsigned ID;
    std::string name;
    template<class T> static void add_data_flag(std::string &data, T flag)
    {
        uint8_t x = (uint8_t)flag;
        data += *reinterpret_cast<char*>(&x);
    }
    template<class T> static T poll_data_flag(std::string &data)
    {
        if(data.empty())
            throw std::out_of_range("attempted to poll the flag of an empty data packet");
        T flag = (T)data.back();
        data.pop_back();
        return flag;
    }
    void flush();
    void construct(TCPsocket socket, IPaddress IP);
    template<class Flag> void send(std::string data, Flag flag)
    {
        if(is_connected())
        {
            add_data_flag(data, flag);
            send_packet(data, PacketFlag::standard);
        }
    }
    void receive(std::function<void(std::string)> output_func, std::function<void(Connection&, std::string&)> &process_data);
    bool operate(std::function<void(std::string)> output_func, std::function<void(Connection&, std::string&)> &process_data); //returns true if the connection is dead
    bool is_connected() const;
    unsigned get_ping();
};
