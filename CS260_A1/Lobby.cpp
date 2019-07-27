#include "Lobby.h"
#include "Player.h"
#include "NewGame.h"
#include <algorithm>
#include <regex>

void Lobby::AddUser(Player p)
{
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);
    userList.push_back(p);
  }
}

void Lobby::DeleteUser(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);

    LobbyIterator itr = 
    std::find_if(userList.begin(), userList.end(), [&](const Player& p)
    {
      return p.s == s;
    });


    if (itr != userList.end())
    {
      // Remove game
      if (itr->currState == IN_GAME)
      {
        std::unique_lock<std::mutex> lck_lck(game_mutex);
        auto gameitr = std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
        {
          return game.CheckPlayer(s);
        });

        if (gameitr != GameList.end())
        {
          SOCKET socket = s == gameitr->white.s ? gameitr->black.s : gameitr->white.s;

          Player& opp = *std::find_if(userList.begin(), userList.end(), [&](const Player& p)
          {
            return p.s == socket;
          });
          if (opp.s != INVALID_SOCKET)
          {
            opp.currState = IDLE;
            opp.opp = INVALID_SOCKET;
            threadpool.AddToQueue(SendSocketMsg(opp.s, "Opponent disconnected. Type 'refresh' to display lobby.\n"));
          }

          for (auto &elem : gameitr->spec)
          {
            Player& spec = *std::find_if(userList.begin(), userList.end(), [&](const Player& p)
            {
              return p.s == elem;
            });

            if (spec.s != INVALID_SOCKET)
            {
              spec.currState = IDLE;
              spec.watch = INVALID_SOCKET;
              threadpool.AddToQueue(SendSocketMsg(spec.s, "Game session closed. Type 'refresh' to display lobby.\n"));
            }
          }

          GameList.erase(gameitr);
        }
      }
      else if (itr->currState == SPECTATE)
      {
        std::unique_lock<std::mutex> lck_lck(game_mutex);
        auto gameitr = std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
        {
          return game.CheckSpectators(s);
        });
        if(gameitr != GameList.end())
          gameitr->DeleteSpectators(itr->s);
      }
      // Find the opponent;
      LobbyIterator oppitr =
        std::find_if(userList.begin(), userList.end(), [&](const Player& p)
      {
        return p.opp == itr->s;
      });
      if (oppitr != userList.end())
      {
        oppitr->opp = INVALID_SOCKET;
        oppitr->currState = IDLE;
        threadpool.AddToQueue(SendSocketMsg(oppitr->s, "Opponent left.\n"));
        threadpool.AddToQueue(SendLobbyInfo(oppitr->s));
      }

      userList.erase(itr);
    }
  }
}

Player& Lobby::FindPlayer(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);

    LobbyIterator itr = 
    std::find_if(userList.begin(), userList.end(), [&](const Player& p)
    {
      return p.s == s;
    });

    if (itr != userList.end())
      return *itr;
    
    return invalid_player;
  }
}

Player& Lobby::FindPlayer(std::string name)
{
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);

    LobbyIterator itr =
      std::find_if(userList.begin(), userList.end(), [&](const Player& p)
    {
      return p.name == name;
    });

    if (itr != userList.end())
      return *itr;

    return invalid_player;
  }
}

bool Lobby::CheckDuplicateName(std::string name)
{
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);
    LobbyIterator itr =
      std::find_if(userList.begin(), userList.end(), [&](const Player& p)
    {
      return p.name == name;
    });

    if (itr != userList.end())
      return true;
    return false;
  }
}

std::string Lobby::DisplayLobby() 
{
  std::string lobbyInfo;
  lobbyInfo.append("Type 'help' to show list of commands\n");
  lobbyInfo.append("Players online: ");
  {
    std::unique_lock<std::mutex> lck(lobby_mutex);
    lobbyInfo.append(std::to_string(userList.size()));
    lobbyInfo.append("\n");

    int i = 0;
    for(auto &elem : userList)
    {
      lobbyInfo.append(std::to_string(i + 1));
      lobbyInfo.append(". ");
      lobbyInfo.append(elem.name);
      lobbyInfo.append(elem.GetStateString(elem.currState));
      lobbyInfo.append("\n");
      ++i;
    }
  }
  return lobbyInfo;
}

void Lobby::SetupGame(Player w, Player b)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    GameList.push_back(NewGame(w, b));
    GameList.back().PrintBoard();
  }
}

NewGame& Lobby::FindGame(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    return *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckPlayer(s);
    });
  }
}

NewGame& Lobby::FindSpecGame(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    return *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckSpectators(s);
    });
  }
}

void Lobby::DeleteGame(NewGame game)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    auto itr = std::find(GameList.begin(), GameList.end(), game);
    if (itr != GameList.end())
    {
      for (size_t i = 0; i < itr->SpecSize(); ++i)
      {
        Player& player = lobby.FindPlayer(itr->spec[i]);
        if (player.s != INVALID_SOCKET)
        {
          threadpool.AddToQueue(SendSocketMsg(player.s, "Game session ended, type 'refresh' to display lobby\n"));
          player.currState = IDLE;
        }
      }
      GameList.erase(itr);
    }
  }
}

void Lobby::MakeMove(SOCKET s, Move move)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    NewGame& g = *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckPlayer(s);
    });

    g.MakeMove(move);
  }
}

void Lobby::PrintBoardSpec(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    NewGame& game = *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckSpectators(s);
    });

    game.PrintBoardSpec(s);
  }
}

void Lobby::PrintBoard(SOCKET s)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    NewGame& game = *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckPlayer(s);
    });
    game.PrintBoard(s);
  }
}

void Lobby::AddSpectator(SOCKET socket, SOCKET watcher)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    NewGame& game = *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckPlayer(socket);
    });
    game.AddSpectators(watcher);
    game.PrintBoardSpec(watcher);
  }
}
void Lobby::DeleteSpectator(SOCKET watcher)
{
  {
    std::unique_lock<std::mutex> lck(game_mutex);
    NewGame& game = *std::find_if(GameList.begin(), GameList.end(), [&](const NewGame& game)
    {
      return game.CheckSpectators(watcher);
    });
    game.DeleteSpectators(watcher);
  }
}
