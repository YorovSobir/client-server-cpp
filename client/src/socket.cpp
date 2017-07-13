#include "socket.h"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>

Socket_Exception::Socket_Exception(const std::string& message)
        : user_message(message)
{}

Socket_Exception::~Socket_Exception() noexcept
{}

const char *Socket_Exception::what() const noexcept {
    return user_message.c_str();
}

void fill_addr(const std::string &address, uint16_t port, sockaddr_in &addr) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    hostent *host;
    if ((host = gethostbyname(address.c_str())) == nullptr) {
        throw Socket_Exception("Failed to resolve name (gethostbyname())");
    }
    addr.sin_addr.s_addr = *(reinterpret_cast<unsigned long*>(host->h_addr_list[0]));

    addr.sin_port = htons(port);
}


Socket::Socket(int type, int protocol) {
    if ((sock_desc = socket(PF_INET, type, protocol)) < 0) {
        throw Socket_Exception("Socket creation failed (socket())");
    }
}

Socket::Socket(int sock_desc) {
    this->sock_desc = sock_desc;
}

Socket::~Socket() {
    ::close(sock_desc);
    sock_desc = -1;
}

void Socket::set_local_port(uint16_t local_port) {
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(local_port);

    if (bind(sock_desc, reinterpret_cast<sockaddr *>(&local_addr), sizeof(sockaddr_in)) < 0) {
        throw Socket_Exception("Set of local port failed (bind())");
    }
}

void Socket::set_local_address_port(const std::string &local_address,
                                    uint16_t local_port) {

    sockaddr_in local_addr;
    fill_addr(local_address, local_port, local_addr);

    if (bind(sock_desc, reinterpret_cast<sockaddr *>(&local_addr), sizeof(sockaddr_in)) < 0) {
        throw Socket_Exception("Set of local address and port failed (bind())");
    }
}

Communication::Communication(int type, int protocol)
        : Socket(type, protocol)
{}

Communication::Communication(int new_conn_SD)
        : Socket(new_conn_SD)
{}

void Communication::connect(const std::string &foreign_address,
                            uint16_t foreign_port) {
    sockaddr_in dest_addr;
    fill_addr(foreign_address, foreign_port, dest_addr);

    if (::connect(sock_desc, reinterpret_cast<sockaddr*>(&dest_addr), sizeof(dest_addr)) < 0) {
        throw Socket_Exception("Connect failed (connect())");
    }
}

void Communication::send(const void *buffer, size_t buffer_len) {
    if (::send(sock_desc, buffer, buffer_len, 0) < 0) {
        throw Socket_Exception("Send failed (send())");
    }
}

ssize_t Communication::recv(void *buffer, size_t buffer_len) {
    ssize_t rtn;
    if ((rtn = ::recv(sock_desc, buffer, buffer_len, 0)) < 0) {
        throw Socket_Exception("Received failed (recv())");
    }
    return rtn;
}

TCPSocket::TCPSocket()
        : Communication(SOCK_STREAM, IPPROTO_TCP) {
}

TCPSocket::TCPSocket(const std::string& foreign_address, uint16_t foreign_port)
        : Communication(SOCK_STREAM, IPPROTO_TCP)
{
    connect(foreign_address, foreign_port);
}

TCPSocket::TCPSocket(int new_conn_SD)
        : Communication(new_conn_SD) {
}

TCPServer_Socket::TCPServer_Socket(uint16_t local_port, int queue_len)
        : Socket(SOCK_STREAM, IPPROTO_TCP) {
    set_local_port(local_port);
    set_listen(queue_len);
}

TCPServer_Socket::TCPServer_Socket(const std::string &local_address,
                                   uint16_t local_port, int queue_len)
        : Socket(SOCK_STREAM, IPPROTO_TCP)
{
    set_local_address_port(local_address, local_port);
    set_listen(queue_len);
}

std::unique_ptr<TCPSocket> TCPServer_Socket::accept() {
    int new_conn_SD;
    if ((new_conn_SD = ::accept(sock_desc, NULL, 0)) < 0) {
        throw Socket_Exception("Accept failed (accept())");
    }
    return std::make_unique<TCPSocket>(new_conn_SD);
}

void TCPServer_Socket::set_listen(int queue_len) {
    if (listen(sock_desc, queue_len) < 0) {
        throw Socket_Exception("Set listening socket failed (listen())");
    }
}

UDPSocket::UDPSocket()
        : Communication(SOCK_DGRAM, IPPROTO_UDP) {
    set_broadcast();
}

UDPSocket::UDPSocket(uint16_t local_port) :
        Communication(SOCK_DGRAM, IPPROTO_UDP) {
    set_local_port(local_port);
    set_broadcast();
}

UDPSocket::UDPSocket(const std::string& local_address, uint16_t local_port)
        : Communication(SOCK_DGRAM, IPPROTO_UDP) {
    set_local_address_port(local_address, local_port);
    set_broadcast();
}

void UDPSocket::set_broadcast() {
    int broadcast_permission = 1;
    setsockopt(sock_desc, SOL_SOCKET, SO_BROADCAST,
               reinterpret_cast<void*>(&broadcast_permission), sizeof(broadcast_permission));
}

void UDPSocket::send_to(const void *buffer, size_t buffer_len,
                        const std::string &foreign_address, uint16_t foreign_port) {
    sockaddr_in dest_addr;
    fill_addr(foreign_address, foreign_port, dest_addr);

    if (sendto(sock_desc, buffer, buffer_len, 0,
               reinterpret_cast<sockaddr *> (&dest_addr), sizeof(dest_addr)) != buffer_len) {
        throw Socket_Exception("Send failed (sendto())");
    }
}

ssize_t UDPSocket::recv_from(void *buffer, size_t buffer_len, std::string &source_address,
                             uint16_t &source_port) {
    sockaddr_in clnt_addr;
    socklen_t addr_len = sizeof(clnt_addr);
    ssize_t rtn;
    if ((rtn = recvfrom(sock_desc, buffer, buffer_len, 0,
                        reinterpret_cast<sockaddr *>(&clnt_addr), &addr_len)) < 0) {
        throw Socket_Exception("Receive failed (recvfrom())");
    }
    source_address = inet_ntoa(clnt_addr.sin_addr);
    source_port = ntohs(clnt_addr.sin_port);

    return rtn;
}