#include "socket.h"
#include <iostream>
#include <cstring>

void get_options(int argc, char** argv, Protocol& protocol,
                 std::string& server_addr, uint16_t& server_port);

void tcp_connection(const std::string& server_addr, uint16_t server_port);
void udp_connection(const std::string& server_addr, uint16_t aserver_port);

int main(int argc, char** argv) {
    std::string server_addr;
    uint16_t server_port = 0;
    Protocol protocol = UNDEFINED;

    get_options(argc, argv, protocol, server_addr, server_port);

    if (protocol == TCP) {
        tcp_connection(server_addr, server_port);
    } else {
        udp_connection(server_addr, server_port);
    }
    return 0;
}
