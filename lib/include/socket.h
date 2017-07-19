#ifndef INTERVIEW_PROJECT_SOCKET_H
#define INTERVIEW_PROJECT_SOCKET_H

#include <string>
#include <exception>
//#include <bits/unique_ptr.h>
#include <memory>

enum Protocol {
    TCP,
    UDP,
    UNDEFINED
};

struct Socket_Exception : std::exception {

    Socket_Exception(const std::string& message);
    ~Socket_Exception() noexcept;
    const char *what() const noexcept override;

private:
    std::string user_message;
};

struct Socket {
    ~Socket();

    void set_local_port(uint16_t local_port);
    void set_local_address_port(const std::string &local_address,
                                uint16_t local_port = 0);
    Socket(const Socket &sock) = default;
    void operator=(const Socket &sock) = delete;

protected:
    int sock_desc;
    Socket(int type, int protocol);
    Socket(int sock_desc);
};

struct Communication : Socket {
    void connect(const std::string &foreign_address, uint16_t foreign_port);
    void send(const void *buffer, size_t buffer_len);
    ssize_t recv(void *buffer, size_t buffer_len);
    Communication(const Communication& com) = default;

protected:
    Communication(int type, int protocol);
    Communication(int new_conn_SD);
};

struct TCPSocket : Communication {
    static const size_t MAX_BUF_SIZE = 32;
    TCPSocket();
    TCPSocket(int new_conn_SD);
    TCPSocket(const std::string &foreign_address, uint16_t foreign_port);
private:
    friend class TCPServer_Socket;
};

struct TCPServer_Socket : Socket {
    TCPServer_Socket(uint16_t local_port, int queue_len = 5);
    TCPServer_Socket(const std::string &local_address, uint16_t local_port, int queue_len = 5);
    std::unique_ptr<TCPSocket> accept();
private:
    void set_listen(int queue_len);
};

struct UDPSocket : Communication {
    static const size_t MAX_BUF_SIZE = 64 * 1024;
    UDPSocket();
    UDPSocket(uint16_t local_port);
    UDPSocket(const UDPSocket& sock) = default;
    UDPSocket(const std::string& local_address, uint16_t local_port);

    void send_to(const void *buffer, size_t buffer_len, const std::string &foreign_address,
                 uint16_t foreign_port);

    ssize_t recv_from(void *buffer, size_t buffer_len, std::string &source_address,
                      uint16_t &source_port);
private:
    void set_broadcast();
};

#endif //INTERVIEW_PROJECT_SOCKET_H
