#include <getopt.h>
#include <iostream>
#include <vector>
#include "socket.h"
#include <memory>
#include <algorithm>

void get_options(int argc, char** argv, Protocol& protocol, uint16_t& server_port) {

    int res;
    while ((res = getopt(argc, argv, "utp:")) != -1) {
        switch (res) {
            case 'u':
                if (protocol != UNDEFINED) {
                    std::cerr << "choose one protocol\n";
                    exit(-1);
                }
                protocol = UDP;
                break;

            case 't':
                if (protocol != UNDEFINED) {
                    std::cerr << "choose one protocol\n";
                    exit(-1);
                }
                protocol = TCP;
                break;

            case 'p':
                if (server_port != 0) {
                    std::cerr << "input only one server port\n";
                    exit(-1);
                }
                server_port = static_cast<uint16_t>(atoi(optarg));
                break;

            default:
                std::cerr << "unknown option character\n";
                exit(-1);
        }
    }

    if (server_port == 0 || protocol == UNDEFINED) {
        std::cerr << "Not enough arguments\n";
        exit(-1);
    }
}

void get_stat(const std::vector<char>& buffer) {
    std::vector<short> vec;
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (std::isdigit(buffer[i])) {
            vec.push_back(buffer[i] - '0');
        }
    }
    std::sort(vec.begin(), vec.end(),
              [](uint8_t a, uint8_t b) { return a > b;});

    std::cout << std::accumulate(vec.begin(), vec.end(), 0) << std::endl;
    std::for_each(vec.begin(), vec.end(), [](short a) {std::cout << a << " ";});
    std::cout << std::endl << vec.front() << "  " << vec.back() << std::endl;
}

void handle_TCPClient(std::unique_ptr<TCPSocket> sock) {

    std::vector<char> buffer_for_stat;
    char buffer[TCPSocket::MAX_BUF_SIZE + 1];
    ssize_t recv_msg_size;
    std::cout << "Received:\n";
    while ((recv_msg_size = sock->recv(buffer, TCPSocket::MAX_BUF_SIZE)) > 0) {
        buffer_for_stat.insert(buffer_for_stat.end(), buffer, buffer + recv_msg_size);
        buffer[recv_msg_size] = '\0';
        std::cout << buffer;
        sock->send(buffer, recv_msg_size);
        if (recv_msg_size < TCPSocket::MAX_BUF_SIZE) {
            break;
        }
    }

    get_stat(buffer_for_stat);

}

void tcp_connection(uint16_t& server_port) {
    try {
        TCPServer_Socket server_sock(server_port);
        while (true) {
            handle_TCPClient(server_sock.accept());
        }
    } catch (Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void udp_connection(uint16_t& server_port) {
    try {
        UDPSocket sock(server_port);
        auto buffer = std::make_unique<char[]>(UDPSocket::MAX_BUF_SIZE + 1);

        std::vector<char> buffer_for_stat;
        ssize_t recv_msg_size;
        std::string source_address;
        uint16_t source_port;
        while (true) {
            recv_msg_size = sock.recv_from(buffer.get(), UDPSocket::MAX_BUF_SIZE,
                                           source_address, source_port);
            buffer_for_stat.insert(buffer_for_stat.end(), buffer.get(), buffer.get() + recv_msg_size);
            buffer[recv_msg_size] = '\0';
            std::cout << "Received\n" << buffer.get() << std::endl;

            sock.send_to(buffer.get(), recv_msg_size, source_address, source_port);
            get_stat(buffer_for_stat);
            buffer_for_stat.clear();
        }

    } catch (Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}



