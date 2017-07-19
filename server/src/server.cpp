#include <getopt.h>
#include <iostream>
#include <vector>
#include "socket.h"
#include <algorithm>
#include <map>
#include <thread>
#include <mutex>

std::mutex mutex;
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
    std::map<short, uint32_t, std::greater<short>> digit_map;
    std::for_each(buffer.begin(), buffer.end(),
                  [&digit_map](char ch) {
                      if (std::isdigit(ch)) ++digit_map[ch - '0'];
                  });
    std::lock_guard<std::mutex>(::mutex);
    std::cout << "sum = " << std::accumulate(digit_map.begin(), digit_map.end(), 0,
                                [](int value, const std::pair<short, uint32_t>& b) {
                                    return value + b.first * b.second;
                                })
              << std::endl;
    std::for_each(digit_map.begin(), digit_map.end(),
                  [](const std::pair<short, uint32_t>& p) {
                      for (size_t i = 0; i < p.second; ++i) {
                          std::cout << p.first << " ";
                      }
                  });
    if (!digit_map.empty()) {
        std::cout << std::endl << digit_map.begin()->first << "  " << digit_map.rbegin()->first << std::endl;
    }
}

void handle_TCPClient(std::unique_ptr<TCPSocket> sock) {

    std::vector<char> buffer_for_stat;
    size_t msg_size;
    sock->recv(&msg_size, sizeof(size_t));
    char buffer[TCPSocket::MAX_BUF_SIZE + 1];

    ssize_t bytes_received = 0;
    ssize_t total_bytes_received = 0;
    std::cout << "Received:\n";
    while ((bytes_received = sock->recv(buffer, TCPSocket::MAX_BUF_SIZE)) > 0) {
        buffer_for_stat.insert(buffer_for_stat.end(), buffer, buffer + bytes_received);
        buffer[bytes_received] = '\0';
        std::cout << buffer;
        sock->send(buffer, bytes_received);
        total_bytes_received += bytes_received;
        if (total_bytes_received >= msg_size) {
            break;
        }
    }

    get_stat(buffer_for_stat);

}

void tcp_connection(uint16_t& server_port) {
    try {
        TCPServer_Socket server_sock(server_port);
        while (true) {
            std::thread(handle_TCPClient, server_sock.accept()).detach();
        }
    } catch (Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void handle_UDPClient(UDPSocket& sock, ssize_t recv_msg_size, std::string source_address,
                      uint16_t source_port, std::unique_ptr<char[]>&& buffer) {
    std::vector<char> buffer_for_stat;
    buffer_for_stat.insert(buffer_for_stat.end(), buffer.get(), buffer.get() + recv_msg_size);
    buffer[recv_msg_size] = '\0';
    {
        std::lock_guard<std::mutex>(::mutex);
        std::cout << "Received:\n" << buffer.get() << std::endl;
        sock.send_to(buffer.get(), recv_msg_size, source_address, source_port);
    }
    get_stat(buffer_for_stat);

}

void udp_connection(uint16_t& server_port) {
    try {
        UDPSocket sock(server_port);

        while (true) {
            ssize_t recv_msg_size;
            std::string source_address;
            uint16_t source_port;
            auto buffer = std::make_unique<char[]>(UDPSocket::MAX_BUF_SIZE + 1);
            recv_msg_size = sock.recv_from(buffer.get(), UDPSocket::MAX_BUF_SIZE,
                                           source_address, source_port);
            std::thread(handle_UDPClient, std::ref(sock), recv_msg_size, source_address, source_port, std::move(buffer)).detach();
        }

    } catch (Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}


