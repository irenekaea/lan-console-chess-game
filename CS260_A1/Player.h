#ifndef PLAYER_H
#define PLAYER_H
#include "..\NetworkHandle.h"
#include "SendMsg.h"
#include "ThreadPool.h"
#include "PositionAndMove.h"

class NewGame;
class Lobby;
extern ThreadPool threadpool;
extern Lobby lobby;

enum STATE
{
  IDLE,
  IN_GAME,
  WAITING_OPP_ACCEPT,
  WAITING,
  SPECTATE,
};

struct Player
{
  Player(SOCKET s_ = INVALID_SOCKET, std::string name_ = "", STATE state = IDLE);
  std::string GetStateString(STATE s) const;

  SOCKET s = INVALID_SOCKET;
  SOCKET opp = INVALID_SOCKET;
  SOCKET watch = INVALID_SOCKET;
  std::string name;
  STATE currState = IDLE;
  Colour colour;
};

struct PromptUserName
{
  PromptUserName(SOCKET s);
  void operator()();

  SOCKET m_socket;
};

struct SendLobbyInfo
{
  SendLobbyInfo(SOCKET s);
  void operator()();
  SOCKET m_socket;
};

struct AddNewPlayer
{
  AddNewPlayer(Player player);
  void operator()();
  Player m_player;
};

struct SetupNewGame
{
  SetupNewGame(Player white, Player black);
  void operator()();
  Player m_white, m_black;
};

struct MakeGameMove
{
  MakeGameMove(SOCKET s_, std::string move);
  void operator()();
  MakeGameMove& operator=(const MakeGameMove&) = delete;
  SOCKET s;
  std::string m;
};

struct GetUserCommand
{
  GetUserCommand(std::string ui, SOCKET s);
  void operator()();
  std::string userInput;
  SOCKET socket;
};

struct UpdateUserRefresh
{
  UpdateUserRefresh(SOCKET s);
  void operator()();
  SOCKET socket;
};

struct UpdateUserPlay
{
  UpdateUserPlay(SOCKET s, std::string input);
  void operator()();
  SOCKET socket;
  std::string userInput;
};

struct UpdateUserAccept
{
  UpdateUserAccept(SOCKET s);
  void operator()();
  SOCKET socket;
};

struct UpdateUserReject
{
  UpdateUserReject(SOCKET s);
  void operator()();
  SOCKET socket;
};

struct UpdateUserCancel
{
  UpdateUserCancel(SOCKET s);
  void operator()();
  SOCKET socket;
};

struct UpdateUserMove
{
  UpdateUserMove(SOCKET s, std::string input);
  void operator()();
  SOCKET socket;
  std::string userInput;
};

struct UpdateUserLeave
{
  UpdateUserLeave(SOCKET s);
  void operator()();
  SOCKET socket;
};

struct UpdateUserWatch
{
  UpdateUserWatch(SOCKET s, std::string input);
  void operator()();
  SOCKET socket;
  std::string userInput;
};

#endif
