#ifndef SENDMSG_H
#define SENDMSG_H
#include "..\NetworkHandle.h"

struct SendMsg
{
  SendMsg(const CS260::Socket& socket, std::string m)
  : m_socket(socket)
  , msg(m)
  {}

  void operator()()
  {
    if (m_socket.Send(msg))
    {
      std::cout << "Error in sending " << WSAGetLastError() << std::endl;
      std::cout << "Client disconnected!" << std::endl;
    }
  }
  
  SendMsg& operator=(const SendMsg&) = delete;

  const CS260::Socket& m_socket;
  std::string msg;
};

struct SendSocketMsg
{
  SendSocketMsg(SOCKET socket, std::string msg) : s(socket), m(msg){}
  
  void operator()()
  {
    int result = send(s, m.c_str(), m.length() + 1, 0);
    if (result == SOCKET_ERROR)
    {
      std::cout << "Error in sending " << WSAGetLastError() << std::endl;
      std::cout << "Client disconnected!" << std::endl;
    }
  }

  SOCKET s;
  std::string m;
};

#endif
