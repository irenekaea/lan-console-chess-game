#include "CS260_Network.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

std::atomic<bool> ShutDown(false);
std::mutex shut_mutex;
struct Connection
{
  Connection(CS260_Socket& s_) : s(s_){}
  CS260_Socket& s;

  void operator()()
  {
    std::string msg;
    while (!ShutDown.load() && std::getline(std::cin, msg))
    {
      if (msg == "clear")
        system("CLS");
      else if (msg == "quit")
      {
        std::unique_lock<std::mutex>(shut_mutex);
        ShutDown.store(true);
      }
      else
        s.Send(msg.c_str(), msg.length() + 1);
    }
  }
};

void SetupFDSets(fd_set& ReadFDs, fd_set& ExceptFDs, Connection &c)
{
  FD_ZERO(&ReadFDs);
  FD_ZERO(&ExceptFDs);

  if (c.s.m_socket != INVALID_SOCKET)
  {
    FD_SET(c.s.m_socket, &ReadFDs);
    FD_SET(c.s.m_socket, &ExceptFDs);
  }

}
int main()
{
  // Prompt user for port to connect to
  std::string port_string;
  std::cout << "Please enter port: ";
  std::getline(std::cin, port_string);

  // Prompt user for ip address to connect to
  std::string ip_addr;
  std::cout << "Please enter ip: ";
  std::getline(std::cin, ip_addr);

  // Initialize wsadata
  CS260_WSAData wsaData;
  wsaData.Init();
  
  CS260_AddrInfo hints;
  hints.Setup(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Set the address family as AF_INET which is for IPv4
  // Get the address information of the specified port and ip
  CS260_AddrInfo info;
  hints.GetAddressInfo(ip_addr.c_str(), port_string.c_str(), info);

  // Create a connect socket
  CS260_Socket connectSocket(info);
  // To connect to the server
  connectSocket.ConnectSocket(info);
  // Mark the socket as non-blocking
  u_long nNoBlock = 1;
  if (ioctlsocket(connectSocket.m_socket, FIONBIO, &nNoBlock))
  {
    int err = WSAGetLastError();
    std::cout << "Non block listener error: " << err;
    system("Pause");
  }
  std::cout << "Connected!\n";
  Connection connection(connectSocket);

  // Create a worker thread just to wait for cin
  std::thread worker(connection);

  for (;;)
  {
    {
      std::unique_lock<std::mutex>(shut_mutex);
      if (ShutDown.load())
        break; 
    }
    char buff[BUFSIZ];
    int bytes_read;
    SecureZeroMemory(buff, BUFSIZ);
    std::string s;
    std::string msg;

    fd_set ReadFDs, WriteFDs, ExceptFDs;
    FD_ZERO(&WriteFDs);
    SetupFDSets(ReadFDs, ExceptFDs, connection);

    int result = select(0, &ReadFDs, &ExceptFDs, &WriteFDs, nullptr);

    if (result == SOCKET_ERROR)
    {
      std::cout << "ERROR IN SELECT: " << WSAGetLastError();
      system("Pause");
    }

    if (result > 0)
    {
      if (FD_ISSET(connectSocket.m_socket, &ReadFDs))
      {
        bytes_read = connectSocket.Receive(buff, BUFSIZ);

        if (bytes_read == 0 || bytes_read == SOCKET_ERROR)
          break; // Server closed

        s.append(buff, bytes_read - 1);
        std::cout << s;
        FD_CLR(connectSocket.m_socket, &ReadFDs);
      }
    }
  }
  ShutDown.store(true);
  worker.join();
  std::cout << "Server disconnected.\n";
  std::string lala;
  std::getline(std::cin, lala);
  return 0;
}
