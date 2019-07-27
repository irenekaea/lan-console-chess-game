#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NETWORK_H
#define NETWORK_H

#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <iostream>

#include <string>
#pragma comment(lib, "ws2_32.lib")

struct ServerInfo
{
  unsigned short port;
  std::string ip_addr;
};

class CS260_WSAData
{
public:
  CS260_WSAData()
    : tableSize(0)
  {
    SecureZeroMemory(&wsaData, sizeof(wsaData));
  }

  ~CS260_WSAData()
  {
    WSACleanup();
  }

  void Init()
  {
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
      std::cout << "WSAStartup fail" << std::endl;
      exit(-1);
    }
  }

  int GetMajorVersion() const
  {
    return LOBYTE(wsaData.wVersion);
  }
  int GetMinorVersion() const
  {
    return HIBYTE(wsaData.wVersion);
  }

private:
  WSADATA wsaData;
  DWORD tableSize;
  std::string ipAddr;
};

class CS260_AddrInfo
{
public:
  CS260_AddrInfo() : result(nullptr)
  {
    SecureZeroMemory(&m_info, sizeof(m_info));
  }
  ~CS260_AddrInfo()
  {
    if (result)
      freeaddrinfo(result);
  }
  addrinfo& Get()
  {
    return m_info;
  }

  const addrinfo& Get() const
  {
    return m_info;
  }

  void Setup(int ai_fam, int ai_socktype, int protocol, int ai_flags = -1)
  {
    m_info.ai_family = ai_fam;
    m_info.ai_socktype = ai_socktype;
    m_info.ai_protocol = protocol;
    if (ai_flags != -1)
      m_info.ai_flags = ai_flags;
  }
  void GetAddressInfo(const char* ip, const char* port, CS260_AddrInfo &result)
  {
    addrinfo* result_info;
    if (getaddrinfo(ip, port, &m_info, &result_info))
    {
      std::cout << "Fail to get address info" << std::endl;
      exit(-1);
    }
    result.Get() = *result_info;
    result.result = result_info;
  }
  addrinfo *result;
private:
  addrinfo m_info;
  
};

class CS260_Socket
{
public:
  CS260_Socket(addrinfo *info = nullptr)
  {
    if (info)
    m_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (m_socket == INVALID_SOCKET && info)
    {
      std::cout << "Error in creating socket (connect)" << std::endl;
      exit(-1);
    }
  }

  CS260_Socket(const CS260_AddrInfo &info)
  {
    m_socket = socket(info.Get().ai_family, info.Get().ai_socktype, info.Get().ai_protocol);
    if (m_socket == INVALID_SOCKET)
    {
      std::cout << "Error in creating socket (connect)" << std::endl;
      exit(-1);
    }
  }

  ~CS260_Socket()
  {
    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);
  }
  SOCKET& GetInfo()
  {
    return m_socket;
  }

  ServerInfo GetServerInfo(CS260_AddrInfo& hints)
  {
    sockaddr_in port_info;
    int size = sizeof(port_info);
    SecureZeroMemory(&port_info, size);
    if (getsockname(m_socket, (sockaddr*)(&port_info), &size))
    {
      std::cout << "Fail to get sockname\n";
      exit(-1);
    }
    char host[BUFSIZ];
    char buffer[BUFSIZ];
    if (getnameinfo((sockaddr*)&port_info, size, host, NI_MAXHOST, buffer, NI_MAXSERV, NI_NUMERICSERV))
    {
      std::cout << "Fail to get host name\n";
      exit(-1);
    }
    addrinfo* hostAddr;
    if (getaddrinfo(host, NULL, &hints.Get(), &hostAddr))
    {
      std::cout << "Fail to get host addr info\n";
      exit(-1);
    }

    sockaddr_in* addr = (sockaddr_in*)hostAddr->ai_addr;
    inet_ntop(hostAddr->ai_family, &(addr->sin_addr), buffer, INET_ADDRSTRLEN);
    ServerInfo servinfo;
    std::string str{ buffer };
    servinfo.ip_addr = str;
    servinfo.port = ntohs(port_info.sin_port);
    return servinfo;
  }

  void ConnectSocket(const CS260_AddrInfo& info)
  {
    if (connect(m_socket, info.Get().ai_addr, (int)info.Get().ai_addrlen) == SOCKET_ERROR)
    {
      std::cout << "Unable to connect to server!" << std::endl;
      std::cout << "Press enter to exit the program\n";
      std::string str;
      std::getline(std::cin, str);
      exit(-1);
    }
  }

  void BindSocket(const CS260_AddrInfo& info) const
  {
    if (bind(m_socket, info.Get().ai_addr, (int)info.Get().ai_addrlen))
    {
      std::cout << "Error in bind" << std::endl;
      exit(-1);
    }
  }

  SOCKET Accept(SOCKET oppSocket, sockaddr* addr, int* size)
  {
    if (m_socket != INVALID_SOCKET)
    {
      shutdown(m_socket, SD_BOTH);
      closesocket(m_socket);
    }
    m_socket = accept(oppSocket, addr, size);
    return m_socket;
  }
  void Listen() const
  {
    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    {
      std::cout << "Fail to listen" << std::endl;
      exit(-1);
    }
  }

  int Receive(char *buff, int buff_len)
  {
    int bytes_read;
    bytes_read = recv(m_socket, buff, buff_len, 0);
    return bytes_read;
  }

  int Receive()
  {
    int bytes_read = recv(m_socket, buffer, BUFSIZ, 0);
    if (bytes_read == SOCKET_ERROR)
    {
      // If the server disconnected, inform the user and exit the program
      std::cout << "Error in receiving " << WSAGetLastError() << std::endl;
    }
    else
    {
      bytesInBuffer += bytes_read;

    }
    return bytes_read;
  }

  bool Send(const char* buff, int buff_len) const
  {
    if (send(m_socket, buff, buff_len, 0) == SOCKET_ERROR)
    {
      return true;
    }
    return false;
  }

  SOCKET m_socket = INVALID_SOCKET;
  int bytesInBuffer = 0;
  char buffer[BUFSIZ];
  

};

/*struct ThreadDataObj
{
  ThreadDataObj(CriticalSection &cs_, CS260_Socket& socket) : m_cs(cs_), m_socket(socket){}
  CriticalSection &m_cs;
  CS260_Socket& m_socket;

  ThreadDataObj& operator=(const ThreadDataObj&) = delete;
};*/

#endif