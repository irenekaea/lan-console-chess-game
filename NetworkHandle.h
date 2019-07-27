#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NETWORK_HANDLE_H
#define NETWORK_HANDLE_H

#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <iostream>

// Common header
#include <mutex>
#include <vector>
#include <condition_variable>

#include <string>
#pragma comment(lib, "ws2_32.lib")

namespace CS260
{
  struct ServerInfo
  {
    unsigned short port;
    std::string ip_addr;
  };

  class WSAData
  {
  public:
    WSAData();
    ~WSAData();
    
    void Init();
    int GetMajorVersion() const;
    int GetMinorVersion() const;

  private:
    WSADATA wsaData;
    DWORD tableSize;
    std::string ipAddr;
  };

  class Socket
  {
  public:
    Socket(addrinfo *info = nullptr);
    Socket(Socket&& socket);
    ~Socket();

    Socket& operator=(Socket&& rhs);
    ServerInfo GetServerInfo(addrinfo& hints);
    void ConnectSocket(const addrinfo* info);
    void BindSocket(const addrinfo* info) const;
    SOCKET Accept(SOCKET oppSocket);
    void Listen() const;
    int Receive();
    bool Send(const std::string &data) const;

    SOCKET m_socket = INVALID_SOCKET;
    int bytesInBuffer = 0;
    char buffer[BUFSIZ];
  };
}
#endif