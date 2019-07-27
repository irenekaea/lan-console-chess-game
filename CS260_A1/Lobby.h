#ifndef LOBBY_H
#define LOBBY_H

#include <utility>
#include <mutex>
#include <list>
#include <vector>
#include "..\NetworkHandle.h"
#include "Player.h"
#include "NewGame.h"

class Lobby
{
public:
  Lobby() : invalid_player(Player(INVALID_SOCKET, "", IDLE)){}
  // For add/del/find user
  void AddUser(Player p);
  void DeleteUser(SOCKET s);
  bool CheckDuplicateName(std::string name);

  Player& FindPlayer(SOCKET s);
  Player& FindPlayer(std::string name);

  std::string DisplayLobby();
  
  // For game
  void SetupGame(Player w, Player b);
  NewGame& FindGame(SOCKET s);
  NewGame& FindSpecGame(SOCKET s);
  void DeleteGame(NewGame game);
  void MakeMove(SOCKET s, Move move);

  void PrintBoard(SOCKET s);
  void PrintBoardSpec(SOCKET s);
  void AddSpectator(SOCKET socket, SOCKET watcher);
  void DeleteSpectator(SOCKET watcher);

private:
  // Socket, and name
  using LobbyIterator = std::list<Player>::iterator;

  std::list<Player> userList;
  std::mutex lobby_mutex;
  std::list<NewGame> GameList;
  std::mutex game_mutex;

  // Invalid player
  Player invalid_player;
};

#endif