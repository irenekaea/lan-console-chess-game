#include "NetworkHandle.h"
#include <algorithm>
namespace CS260
{
  WSAData::WSAData() : tableSize(0)
  {
    SecureZeroMemory(&wsaData, sizeof(wsaData));
  }

  WSAData::~WSAData()
  {
    WSACleanup();
  }

  void WSAData::Init()
  {
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
      std::cout << "WSAStartup fail" << std::endl;
      exit(-1);
    }
  }

  int WSAData::GetMajorVersion() const
  {
    return LOBYTE(wsaData.wVersion);
  }

  int WSAData::GetMinorVersion() const
  {
    return HIBYTE(wsaData.wVersion);
  }

  Socket::Socket(addrinfo *info) : m_socket(INVALID_SOCKET), bytesInBuffer(0)
  {
    if (info)
      m_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (m_socket == INVALID_SOCKET && info)
    {
      std::cout << "Error in creating socket (connect)" << std::endl;
      exit(-1);
    }
  }

  Socket::Socket(Socket&& socket) : m_socket(INVALID_SOCKET), bytesInBuffer(0)
  {
    std::swap(m_socket, socket.m_socket);
    bytesInBuffer = socket.bytesInBuffer;
    std::swap(buffer, socket.buffer);
  }

  Socket::~Socket()
  {
    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);
  }

  Socket& Socket::operator=(Socket&& rhs)
  {
    std::swap(m_socket, rhs.m_socket);
    bytesInBuffer = rhs.bytesInBuffer;
    std::swap(buffer, rhs.buffer);
    return *this;
  }

  ServerInfo Socket::GetServerInfo(addrinfo& hints)
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
    char somebuff[BUFSIZ];
    if (getnameinfo((sockaddr*)&port_info, size, host, NI_MAXHOST, somebuff, NI_MAXSERV, NI_NUMERICSERV))
    {
      std::cout << "Fail to get host name\n";
      exit(-1);
    }
    addrinfo* hostAddr;
    if (getaddrinfo(host, NULL, &hints, &hostAddr))
    {
      std::cout << "Fail to get host addr info\n";
      exit(-1);
    }

    sockaddr_in* addr = (sockaddr_in*)hostAddr->ai_addr;
    inet_ntop(hostAddr->ai_family, &(addr->sin_addr), somebuff, INET_ADDRSTRLEN);
    ServerInfo servinfo;
    std::string str{ somebuff };
    servinfo.ip_addr = str;
    servinfo.port = ntohs(port_info.sin_port);
    return servinfo;
  }

  void Socket::ConnectSocket(const addrinfo* info)
  {
    if (connect(m_socket, info->ai_addr, (int)info->ai_addrlen) == SOCKET_ERROR)
    {
      std::cout << "Unable to connect to server!" << std::endl;
      std::cout << "Press enter to exit the program\n";
      std::string str;
      std::getline(std::cin, str);
      exit(-1);
    }
  }

  void Socket::BindSocket(const addrinfo* info) const
  {
    if (bind(m_socket, info->ai_addr, (int)info->ai_addrlen))
    {
      std::cout << "Error in bind" << std::endl;
      exit(-1);
    }
  }


  SOCKET Socket::Accept(SOCKET oppSocket)
  {
    if (m_socket != INVALID_SOCKET)
    {
      shutdown(m_socket, SD_BOTH);
      closesocket(m_socket);
    }

    sockaddr addr;
    SecureZeroMemory(&addr, sizeof(addr));
    int size = sizeof(addr);
    m_socket = accept(oppSocket, &addr, &size);
    return m_socket;
  }

  void Socket::Listen() const
  {
    if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    {
      std::cout << "Fail to listen" << std::endl;
      exit(-1);
    }
  }

  int Socket::Receive()
  {
    int bytes_read = recv(m_socket, buffer, BUFSIZ, 0);
    if (bytes_read != SOCKET_ERROR)
    {
      bytesInBuffer += bytes_read;
    }
    return bytes_read;
  }

  bool Socket::Send(const std::string &data) const
  {
    int result = send(m_socket, data.c_str(), data.length() + 1, 0);
    if (result == SOCKET_ERROR)
    {
      std::cout << "Fail to send message: " << WSAGetLastError() << std::endl;
      std::string str;
      return true;
    }
    return false;
  }

}