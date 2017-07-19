#include <getopt.h>
#include <iostream>
#include <memory>
#include "socket.h"

std::string read_text() {
    std::string text, temp;
    while (std::getline(std::cin, temp)) {
        if (temp.compare("exit") == 0) {
            break;
        }
        text.append(temp).append("\n");
    }
    return text;
}

void get_options(int argc, char** argv, Protocol& protocol,
                 std::string& server_addr, uint16_t& server_port) {

    int res;
    while ((res = getopt(argc, argv, "uta:p:")) != -1) {
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

            case 'a':
                if (!server_addr.empty()) {
                    std::cerr << "input only one server address\n";
                    exit(-1);
                }
                server_addr = optarg;
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

    if (server_port == 0 || server_addr.empty() || protocol == UNDEFINED) {
        std::cerr << "Not enough arguments\n";
        exit(-1);
    }
}


void tcp_connection(const std::string& server_addr, uint16_t server_port) {
    auto text = read_text();
    size_t size = text.length();
    try {
        TCPSocket sock(server_addr, server_port);
        sock.send(static_cast<void*>(&size), sizeof(size_t));
        sock.send(text.c_str(), text.length());

        char buffer[TCPSocket::MAX_BUF_SIZE + 1];
        ssize_t bytes_received = 0;
        ssize_t total_bytes_received = 0;
        std::cout << "Received:\n";
        while (total_bytes_received < text.length()) {
            if ((bytes_received = (sock.recv(buffer, TCPSocket::MAX_BUF_SIZE ))) <= 0) {
                std::cerr << "Unable to read";
                exit(-1);
            }
            total_bytes_received += bytes_received;
            buffer[bytes_received] = '\0';
            std::cout << buffer;
        }

    } catch(Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void udp_connection(const std::string& server_addr, uint16_t server_port) {
    auto text = read_text();
    try {
        UDPSocket sock;
        sock.send_to(text.c_str(), text.length(), server_addr, server_port);
        auto buffer = std::make_unique<char[]>(text.length() + 1);

        if (sock.recv(buffer.get(), text.length()) != text.length()) {
            std::cerr << "Unable to receive" << std::endl;
            exit(1);
        }

        buffer[text.length()] = '\0';
        std::cout << "Received: " << buffer.get() << std::endl;
    } catch (Socket_Exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}