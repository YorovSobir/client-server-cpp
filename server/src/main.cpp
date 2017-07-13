#include "socket.h"

void get_options(int argc, char** argv, Protocol& protocol, uint16_t& server_port);

void tcp_connection(uint16_t& server_port);
void udp_connection(uint16_t& server_port);

int main(int argc, char** argv) {
    uint16_t server_port = 0;
    Protocol protocol = UNDEFINED;

    get_options(argc, argv, protocol, server_port);

    if (protocol == TCP) {
        tcp_connection(server_port);
    } else {
        udp_connection(server_port);
    }
    return 0;
}
