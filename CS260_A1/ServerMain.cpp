#if !defined(CLIENT_RELEASE) && !defined(CLIENT_DEBUG)
#include <iostream>
#include "ThreadPool.h"
#include "..\Functions.h"
#include "..\NetworkHandle.h"
#include "Jobs.h"
#include "Lobby.h"

#define NUM_THREADS 10

// Global
Lobby lobby;
std::string commands[] = { "refresh", "play", "accept", "reject", "cancel", "move", "leave", "help", "watch" };
std::string helpCommand = 
"__________________________\nrefresh\n  - redisplay lobby/board\nplay <username>\n  - send game invitation to <username>\naccept\n  - accept a game invitation\nreject\n  - reject a game invitation\ncancel\n  - cancel a game invitation\nmove (<LETTER><NUMBER>-<LETTER><NUMBER>)\n  - move (A0-A1) to move piece from A0 to A1\nleave\n  - leave current game\nclear\n  - clear console\nwatch <username>\n  - watch <username>'s game\nquit\n  - quit game\n__________________________\n";
ThreadPool threadpool(NUM_THREADS);
std::list<CS260::Socket> SocketList;
std::mutex lobby_mutex;

int main()
{
  // Setup listener
  // Prompt the user for the port to connect to
  std::string port_string;
  std::cout << "Please enter port: ";
  std::cin >> port_string;

  std::cout << "Starting up" << std::endl;
  CS260::WSAData wsaData;
  wsaData.Init(); // Initialize wsadata

  // Set the address family as AF_INET which is for IPv4
  // Set socktype as SOCK_STREAM which stands for TCP
  // Set as AI_PASSIVE such that the return address is
  // suitable in binding a socket for accepting incoming 
  // connections.
  addrinfo hints;
  SecureZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  addrinfo *result_info;
  int result = getaddrinfo(NULL, port_string.c_str(), &hints, &result_info);
  if (result)
  {
    std::cerr << "getaddrinfo failed\n" << result;
    return -1;
  }

  // Create a listener to listen for connection
  CS260::Socket listenerSocket;
  listenerSocket.m_socket = socket(result_info->ai_family, result_info->ai_socktype, result_info->ai_protocol);

  // Bind the listener socket
  listenerSocket.BindSocket(result_info);

  // Get infomation of the server
  sockaddr_in port_info;
  int size = sizeof(port_info);
  SecureZeroMemory(&port_info, size);

  if (getsockname(listenerSocket.m_socket, (sockaddr*)(&port_info), &size))
  {
    std::cout << "Fail to get sockname\n";
    return -1;
  }
  freeaddrinfo(result_info);
  // Get port and ip address
  CS260::ServerInfo server_info = listenerSocket.GetServerInfo(hints);
  std::cout << "Server Port: " << server_info.port << std::endl;
  std::cout << "Server IP: " << server_info.ip_addr << std::endl;

  listenerSocket.Listen();
  // Mark the socket as non-blocking
  u_long nNoBlock = 1;
  if (ioctlsocket(listenerSocket.m_socket, FIONBIO, &nNoBlock))
  {
    int err = WSAGetLastError();
    std::cout << "Non block listener error: " << err;
    system("Pause");
  }
  
  for (;;)
  {
    std::cout << "Waiting for connection\n";
    fd_set ReadFDs, WriteFDs, ExceptFDs;
    SetupFDSets(ReadFDs, WriteFDs, ExceptFDs, listenerSocket.m_socket);

    result = select(0, &ReadFDs, &WriteFDs, &ExceptFDs, nullptr);

    if (result == SOCKET_ERROR)
    {
      int err = WSAGetLastError();
      std::cout << "select error " << err;
    }

    if (result > 0)
    {
      // Something happened on one of the sockets
      // Check if it's the listening socket
      if (FD_ISSET(listenerSocket.m_socket, &ReadFDs))
      {
        // Accept incoming connection
        CS260::Socket clientSocket;
        result = clientSocket.Accept(listenerSocket.m_socket);

        if (result != INVALID_SOCKET)
        {
          // Mark the socket as non blocking
          u_long non_block = 1;
          if (ioctlsocket(clientSocket.m_socket, FIONBIO, &non_block))
          {
            int err = WSAGetLastError();
            std::cout << "Non block error: " << err;
            system("Pause");
          }
          // Add into socket list & prompt user name
          std::cout << "Connected: " << clientSocket.m_socket << std::endl;
          SOCKET s = clientSocket.m_socket;
          SocketList.push_back(std::move(clientSocket));
          
          threadpool.AddToQueue(PromptUserName(s));

          FD_CLR(listenerSocket.m_socket, &ReadFDs);
        }
      }
      else if (FD_ISSET(listenerSocket.m_socket, &ExceptFDs))
      {
        std::cout << "EXCEPTION ON LISTENER SOCKET \n";
        return -1;
      }
    }

    std::list<CS260::Socket>::iterator itr = SocketList.begin();

    // Check if anything happen on one of the client sockets
    while (itr != SocketList.end())
    {
      bool closeSocket = false;
      // Exception happen on one of the client sockets
      if (FD_ISSET(itr->m_socket, &ExceptFDs))
      {
        // Close socket
        closeSocket = true;
        FD_CLR(itr->m_socket, &ExceptFDs);
      }
      else
      {
        if (FD_ISSET(itr->m_socket, &ReadFDs))
        {
          int bytes_read = itr->Receive();
          if (bytes_read == 0 || bytes_read == INVALID_SOCKET)
            closeSocket = true;
          else
          {
            std::cout << "From " << itr->m_socket << ": " << itr->buffer << std::endl;
            std::cout << "Checking state" << std::endl;
            Player player = lobby.FindPlayer(itr->m_socket);
            
            // Player does not exist, add the player
            if (player.s == INVALID_SOCKET)
            {
              // Socket have not been added as a player yet
              // Check if username exist
              std::string name(itr->buffer);
              SecureZeroMemory(itr->buffer, sizeof(itr->buffer));
              itr->bytesInBuffer = 0;

              if (lobby.CheckDuplicateName(name)) // Duplicated name
                threadpool.AddToQueue(PromptUserName(itr->m_socket));
              else
              {
                Player p(itr->m_socket, name, IDLE);
                threadpool.AddToQueue(AddNewPlayer(p));
              }
            }
            else
              ExecuteCommand(itr);
          }

          FD_CLR(itr->m_socket, &ReadFDs);
        }
        if (FD_ISSET(itr->m_socket, &WriteFDs))
        {
          // Handle the writing of data
          FD_CLR(itr->m_socket, &WriteFDs);
        }
      }

      if (closeSocket)
      {
        // One of the client disconnected, remove it from the connection
        // remove it from the list
        std::cout << "closing connection " << itr->m_socket << " \n";
        lobby.DeleteUser(itr->m_socket);
        SocketList.erase(itr);
        itr = SocketList.begin();
      }
      else
        ++itr;
    }

  } // End for(;;)

  return 0;
}

#endif
