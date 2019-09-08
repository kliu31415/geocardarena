#include "connection.h"
#include "sdl_base.h"
#include "fundamentals.h"
#include <SDL2/SDL_net.h>
#include <string>
#include <cstring>
using namespace std;
string IPv4_to_str_no_port(const IPaddress &IP)
{
    string res;
    auto cur = IP.host; //assume it's the opposite endianness (which is probably true)
    while(true)
    {
        res += to_str((unsigned)(cur & 0xff));
        cur >>= 8;
        if(cur == 0)
            break;
        res += ".";
    }
    return res;
}
string IPv4_to_str(const IPaddress &IP)
{
    return IPv4_to_str_no_port(IP) + ":" + to_str(SDLNet_Read16(&IP.port));
}
//
unsigned Connection::IDcounter = 0;
void Connection::finalize_packet(string &data, PacketFlag flag)
{
    data.resize(data.size() + 2);
    SDLNet_Write16(data.size() - 2, &data.back() - 1); //packet size
    add_data_flag(data, flag);
}
void Connection::send_packet(string &data, PacketFlag flag)
{
    finalize_packet(data, flag);
    if(send_packet_buffer.size() + data.size() > MAX_PACKET_SIZE)
        Connection::flush();
    send_packet_buffer += data;
}
void Connection::send_ping()
{
    string data;
    data.resize(4);
    unsigned tim = getTicks(); //should be fine unless the program is running for more than 2^32-1 ms (~50 days)
    SDLNet_Write32(tim, &data[0]);
    send_packet(data, PacketFlag::ping_send);
}
void Connection::send_ping_ack(string &data)
{
    send_packet(data, PacketFlag::ping_ack);
}
void Connection::flush()
{
    last_flush_time = getTicksS();
    SDLNet_TCP_Send(socket, &send_packet_buffer[0], send_packet_buffer.size());
    send_packet_buffer.clear();
}
void Connection::construct(TCPsocket socket, IPaddress IP)
{
    this->last_activity_time = this->last_flush_time = getTicksS();
    this->last_ping_sent_time = -INF;
    this->socket = socket;
    this->IP = IP;
    this->ID = IDcounter++;
    this->ping_sum = 0;
}
void Connection::receive(function<void(string)> output_func, function<void(Connection&, string&)> &process_data)
{
    if(is_connected() && SDLNet_SocketReady(socket))
    {
        string buffer;
        buffer.resize(MAX_PACKET_SIZE);
        int len = SDLNet_TCP_Recv(socket, &buffer[0], buffer.size());
        if(len < 0)
        {
            last_activity_time = DISCONNECT_ERROR;
            return;
        }
        buffer.resize(len);
        last_activity_time = getTicksS();
        while(!buffer.empty())
        {
            PacketFlag type = poll_data_flag<PacketFlag>(buffer);
            if(buffer.size() < 2)
            {
                output_func("corrupted packet without size information received from " + IPv4_to_str(IP));
                break;
            }
            Uint16 len = SDLNet_Read16(&buffer.back() - 1);
            buffer.resize(buffer.size() - 2);
            if(buffer.size() < len)
            {
                output_func("corrupted packet with incorrect size information received from " + IPv4_to_str(IP));
                output_func(to_str(len));
                break;
            }
            string data = buffer.substr(buffer.size() - len);
            buffer.resize(buffer.size() - len);
            switch(type)
            {
            case PacketFlag::standard:
                process_data(*this, data);
                break;
            case PacketFlag::disconnect:
                last_activity_time = DISCONNECT_SUCCESS;
                break;
            case PacketFlag::ping_ack:
                if(data.size() != 4)
                    output_func("corrupted ping_ack packet received with body size " + to_str(data.size()));
                else
                {
                    unsigned cur_time = getTicks();
                    unsigned ping_time = cur_time - SDLNet_Read32(&data[0]);
                    ping_times.emplace(cur_time, ping_time);
                    ping_sum += ping_time;
                }
                break;
            case PacketFlag::ping_send:
                if(data.size() != 4)
                    output_func("corrupted ping_send packet received with body size " + to_str(data.size()));
                else send_ping_ack(data);
                break;
            case PacketFlag::command:
                break;
            case PacketFlag::admin_command:
                break;
            default:
                output_func("unknown message of type " + to_str((int)type) + " and body size " + to_str(data.size()) + " received from " + IPv4_to_str(IP));
                break;
            }
        }
    }
}
bool Connection::operate(function<void(string)> output_func, function<void(Connection&, string&)> &process_data)
{
    double curTime = getTicksS();
    //don't send too many small packets. latency isn't important in a turn-based game anyway. combine packets like nagle's algorithm
    if(last_flush_time + FLUSH_INTERVAL < curTime)
    {
        Connection::flush();
    }
    //don't spam pings. AUTO_PING_INTERVAL seems to work well for limiting ping rates on a quiet connection
    if(last_ping_sent_time + AUTO_PING_INTERVAL < curTime)
    {
        send_ping();
        last_ping_sent_time = curTime;
    }
    if(last_activity_time + CONNECTION_DISCONNECT_TIME < curTime)
    {
        if(last_activity_time == DISCONNECT_SUCCESS)
            output_func("connection (" + IPv4_to_str(IP) + ") closed due to request");
        if(last_activity_time == DISCONNECT_ERROR)
            output_func("connection (" + IPv4_to_str(IP) + ") closed due to an error");
        else output_func("connection (" + IPv4_to_str(IP) + ") closed due to inactivity");
        Connection::flush();
        return true;
    }
    return false;
}
bool Connection::is_connected() const
{
    return socket != nullptr;
}
unsigned Connection::get_ping()
{
    unsigned cur_time = getTicks();
    while(!ping_times.empty() && ping_times.front().first + PING_WINDOW < cur_time)
    {
        ping_sum -= ping_times.front().second;
        ping_times.pop();
    }
    if(ping_times.empty())
        return 1000;
    return ping_sum / ping_times.size();
}
