#ifndef FUNC_H
#define FUNC_H
#include <thread>
#include <vector>
#include "..\NetworkHandle.h"
#include "CS260_A1\Player.h"
#include "CS260_A1\UserCommands.h"
#include "CS260_A1\Lobby.h"
#include <regex>

extern std::list<CS260::Socket> SocketList;
extern ThreadPool threadpool;
extern std::string commands[];
extern std::string helpCommand;

void SetupFDSets(fd_set& ReadFDs, fd_set& WriteFDs, fd_set& ExceptFDs, SOCKET Listener = INVALID_SOCKET)
{
  FD_ZERO(&ReadFDs);
  FD_ZERO(&WriteFDs);
  FD_ZERO(&ExceptFDs);

  // Add the listener socket to the read and except FD sets, if there
  // is one.
  if (Listener != INVALID_SOCKET) 
  {
    FD_SET(Listener, &ReadFDs);
    FD_SET(Listener, &ExceptFDs);
  }

  // Add client connections
  for (const auto &elem : SocketList)
  {
    if (elem.bytesInBuffer < BUFSIZ)
    {
      // There is something in the this socket buffer
      FD_SET(elem.m_socket, &ReadFDs);
    }
 
    if (elem.bytesInBuffer > 0)
    {
      FD_SET(elem.m_socket, &WriteFDs);
    }

    FD_SET(elem.m_socket, &ExceptFDs);
  }

}

void ExecuteCommand(std::list<CS260::Socket>::iterator itr)
{
  // do command
  std::string userInput(itr->buffer);
  SecureZeroMemory(itr->buffer, sizeof(itr->buffer));
  itr->bytesInBuffer = 0;

  threadpool.AddToQueue(GetUserCommand(userInput, itr->m_socket));
}

#endif
