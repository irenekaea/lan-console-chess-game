#include "Player.h"
#include "Lobby.h"
#include "UserCommands.h"
#include <regex>

extern std::string commands[];
extern std::mutex lobby_mutex;
extern std::string helpCommand;

Player::Player(SOCKET s_, std::string name_, STATE state)
  : s(s_), name(name_), currState(state){}

std::string Player::GetStateString(STATE state) const
{
  {
    switch (state)
    {
    case IDLE:
      return " (available)";
    case IN_GAME:
      return " (in-game)";
    case WAITING_OPP_ACCEPT:
      return " (pending request)";
    case WAITING:
      return " (pending request)";
    case SPECTATE:
      return " (spectating)";
    }
    return "";
  }
}

PromptUserName::PromptUserName(SOCKET s) : m_socket(s){}
void PromptUserName::operator()()
{
  std::cout << "Prompting user name for socket " << m_socket << std::endl;
  std::string str = "Please enter username:\n";
  threadpool.AddToQueue(SendSocketMsg(m_socket, str));
}

SendLobbyInfo::SendLobbyInfo(SOCKET s) : m_socket(s){}
void SendLobbyInfo::operator()()
{
  std::cout << "Printing lobby for socket " << m_socket << std::endl;
  std::string msg = lobby.DisplayLobby();
  threadpool.AddToQueue(SendSocketMsg(m_socket, msg));
}

AddNewPlayer::AddNewPlayer(Player player) : m_player(player){}
void AddNewPlayer::operator()()
{
  std::cout << "Adding user " << m_player.name << " ..." << std::endl;
  lobby.AddUser(m_player);
  threadpool.AddToQueue(SendLobbyInfo(m_player.s));
}

SetupNewGame::SetupNewGame(Player white, Player black) : m_white(white), m_black(black){}
void SetupNewGame::operator()()
{
  lobby.SetupGame(m_white, m_black);
}

MakeGameMove::MakeGameMove(SOCKET s_, std::string move) : s(s_), m(move){}
void MakeGameMove::operator()()
{
  int val[4];
  int i = 0;

  // Loop through each string character to generate moves
  for (const char &elem : m)
  {
    std::regex tmp{ "[()\\-]" };
    if (std::regex_match(std::string{ elem }, tmp))
      continue;
    if (elem >= 'A' && elem <= 'Z')
      val[i++] = elem - 'A';
    else if (elem >= 'a' && elem <= 'z')
      val[i++] = elem - 'a';
    else
      val[i++] = elem - '0';
  }
  Position orig{ val[0], val[1] };
  Position dest{ val[2], val[3] };
  Move move_pos{ orig, dest };
  
  lobby.MakeMove(s, move_pos);
}

GetUserCommand::GetUserCommand(std::string ui, SOCKET s) 
  : userInput(ui), socket(s) {}
void GetUserCommand::operator()()
{
  size_t pos = userInput.find(" ");
  std::string command(userInput, 0, pos);

  if (pos != std::string::npos)
    userInput.erase(userInput.begin(), userInput.begin() + pos + 1);
  else
    userInput.clear();

  // For debugging purporse
  std::cout << "Socket " << socket << " command: " << command << std::endl;
  std::cout << "Socket " << socket << " input: " << userInput << std::endl;

  if (command == commands[REFRESH])
    threadpool.AddToQueue(UpdateUserRefresh(socket));
  else if (command == commands[PLAY])
    threadpool.AddToQueue(UpdateUserPlay(socket, userInput));
  else if (command == commands[ACCEPT])
    threadpool.AddToQueue(UpdateUserAccept(socket));
  else if(command == commands[REJECT])
    threadpool.AddToQueue(UpdateUserReject(socket));
  else if(command == commands[CANCEL])
    threadpool.AddToQueue(UpdateUserCancel(socket));
  else if(command == commands[MOVE])
    threadpool.AddToQueue(UpdateUserMove(socket, userInput));
  else if(command == commands[LEAVE])
    threadpool.AddToQueue(UpdateUserLeave(socket));
  else if(command == commands[HELP])
    threadpool.AddToQueue(SendSocketMsg(socket, helpCommand));
  else if(command == commands[WATCH])
    threadpool.AddToQueue(UpdateUserWatch(socket, userInput));
  else
    threadpool.AddToQueue(SendSocketMsg(socket, "Invalid command\n"));
}

UpdateUserRefresh::UpdateUserRefresh(SOCKET s) : socket(s) {}
void UpdateUserRefresh::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // check if player is in lobby
    Player& player = lobby.FindPlayer(socket);
    if (player.currState != IN_GAME && player.currState != SPECTATE)
      threadpool.AddToQueue(SendLobbyInfo(socket));
    else if (player.currState == IN_GAME)
      lobby.FindGame(player.s).PrintBoard(player.s);
    else
      lobby.FindGame(lobby.FindPlayer(player.watch).s).PrintBoardSpec(player.s);
  }
}

UpdateUserPlay::UpdateUserPlay(SOCKET s, std::string input) : socket(s), userInput(input) {}
void UpdateUserPlay::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Check if player in game
    Player &w = lobby.FindPlayer(socket);
    if (w.currState == IN_GAME)
      threadpool.AddToQueue(SendSocketMsg(socket, "You are currently in game.\n"));
    else if (userInput != "")
    {
      // Check if opponent in game
      Player &b = lobby.FindPlayer(userInput);
      if (b.s == INVALID_SOCKET)
        threadpool.AddToQueue(SendSocketMsg(socket, "No such player.\n"));
      else if (b.name == w.name)
        threadpool.AddToQueue(SendSocketMsg(socket, "You can't play with yourself.\n"));
      else
      {
        if (b.currState == IN_GAME)
        {
          std::string msg = b.name + " currently in game.\n";
          threadpool.AddToQueue(SendSocketMsg(socket, msg));
        }
        else if (b.currState == SPECTATE)
          threadpool.AddToQueue(SendSocketMsg(socket, b.name + " currently watching game.\n"));
        else // Set up game
        {
          switch (w.currState)
          {
          case IDLE:
          {
            w.currState = WAITING_OPP_ACCEPT;
            b.currState = WAITING;
            w.opp = b.s;
            b.opp = w.s;
            std::string msg = "Waiting for " + b.name + " to accept. Type 'cancel' to cancel\n";
            threadpool.AddToQueue(SendSocketMsg(socket, msg));
            std::string m = w.name + " wants to play with you, type 'accept' or 'reject'\n";
            threadpool.AddToQueue(SendSocketMsg(b.s, m));
          }
          break;
          case WAITING_OPP_ACCEPT:
          {
            std::string msg = "You are waiting for " + lobby.FindPlayer(w.opp).name + " to accept. Type 'cancel' to cancel.\n";
            threadpool.AddToQueue(SendSocketMsg(socket, msg));
          }
          break;
          case WAITING:
            threadpool.AddToQueue(SendSocketMsg(socket, "You have a pending game invitation, type 'accept' or 'reject'\n"));
            break;
          case SPECTATE:
            threadpool.AddToQueue(SendSocketMsg(socket, "You are currently watching game. Type 'leave' to stop watching.\n"));
          } // end switch case
        }
      }
    }
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "Please enter the username of the person you want to play with. play <username>\n"));
  }
}

UpdateUserAccept::UpdateUserAccept(SOCKET s) : socket(s){}
void UpdateUserAccept::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Only can accept game if the state is WAITING
    Player& b = lobby.FindPlayer(socket);
    if (b.currState == WAITING)
    {
      Player& w = lobby.FindPlayer(b.opp);
      // opponent have left game
      if (w.s == INVALID_SOCKET)
      {
        b.opp = INVALID_SOCKET;
        b.currState = IDLE;
        threadpool.AddToQueue(SendSocketMsg(socket, "Opponent has left game.\n"));
      }
      else
      {
        // Setup game
        b.currState = IN_GAME;
        b.colour = BLACK;
        w.currState = IN_GAME;
        w.colour = WHITE;
        threadpool.AddToQueue(SetupNewGame(w, b));
      }
    }
    else if (b.currState == SPECTATE)
      threadpool.AddToQueue(SendSocketMsg(socket, "You are currently watching game, type 'leave' to leave game\n"));
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "You have no game pending to accept\n"));
  }
}

UpdateUserReject::UpdateUserReject(SOCKET s) : socket(s) {}
void UpdateUserReject::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Only can reject game if the state is waiting
    Player& b = lobby.FindPlayer(socket);
    if (b.currState == WAITING)
    {
      Player& w = lobby.FindPlayer(b.opp);
      // opponent left
      if (w.s == INVALID_SOCKET)
      {
        b.opp = INVALID_SOCKET;
        b.currState = IDLE;
        threadpool.AddToQueue(SendSocketMsg(socket, "Opponent has left game.\n"));
      }
      else
      {
        w.currState = IDLE;
        b.currState = IDLE;

        w.opp = INVALID_SOCKET;
        b.opp = INVALID_SOCKET;
        std::string msg = b.name + " does not wish to play.\n";
        threadpool.AddToQueue(SendSocketMsg(w.s, msg));
        threadpool.AddToQueue(SendSocketMsg(socket, "Game invitation rejected.\n"));
      }
    }
    else if (b.currState == SPECTATE)
      threadpool.AddToQueue(SendSocketMsg(socket, "You are currently watching game, type 'leave' to leave game\n"));
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "You have no game pending to reject\n"));
  }
}

UpdateUserCancel::UpdateUserCancel(SOCKET s) : socket(s) {}
void UpdateUserCancel::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Only can reject game if the state is WAITING_OPP_ACCEPT
    Player& w = lobby.FindPlayer(socket);
    if (w.currState == WAITING_OPP_ACCEPT)
    {
      Player& b = lobby.FindPlayer(w.opp);
      if (b.s == INVALID_SOCKET)
      {
        w.opp = INVALID_SOCKET;
        w.currState = IDLE;
        threadpool.AddToQueue(SendSocketMsg(socket, "Opponent has left game.\n"));
      }
      else
      {
        w.currState = IDLE;
        b.currState = IDLE;

        w.opp = INVALID_SOCKET;
        b.opp = INVALID_SOCKET;
        std::string msg = w.name + " has cancel the invitation.\n";
        threadpool.AddToQueue(SendSocketMsg(b.s, msg));
        threadpool.AddToQueue(SendSocketMsg(socket, "Game invitation cancelled.\n"));
      }
    }
    else if (w.currState == SPECTATE)
      threadpool.AddToQueue(SendSocketMsg(socket, "You are currently watching game, type 'leave' to leave game\n"));
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "You have no game invitation to cancel\n"));
  }
}

UpdateUserMove::UpdateUserMove(SOCKET s, std::string input) : socket(s), userInput(input) {}
void UpdateUserMove::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Only can make a move if the state is IN_GAME
    Player& player = lobby.FindPlayer(socket);
    if (player.currState == IN_GAME)
    {
      // Check if it is a valid move
      std::regex expr{ "\\([A-Ha-h][0-7]\\-[A-Ha-h][0-7]\\)" };
      if (!std::regex_match(userInput, expr))
        threadpool.AddToQueue(SendSocketMsg(socket, "Invalid move.\n"));
      else
      {
        // Check if it's the player turn
        NewGame game = lobby.FindGame(player.s);
        if (game.turn == player.colour)
        {
          // Play move
          threadpool.AddToQueue(MakeGameMove(socket, userInput));
        }
        else
          threadpool.AddToQueue(SendSocketMsg(socket, "It's not your turn.\n"));
      }
    }
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "You are not in a game.\n"));
  }
}

UpdateUserLeave::UpdateUserLeave(SOCKET s) : socket(s) {}
void UpdateUserLeave::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    // Only can leave game if you are in game
    Player& player = lobby.FindPlayer(socket);
    if (player.currState == IN_GAME)
    {
      NewGame& game = lobby.FindGame(player.s);
      Player& opponent = lobby.FindPlayer(game.GetOpponentSocket(player));

      opponent.opp = INVALID_SOCKET;
      player.opp = INVALID_SOCKET;
      opponent.currState = IDLE;
      player.currState = IDLE;

      lobby.DeleteGame(game);

      threadpool.AddToQueue(SendSocketMsg(socket, "You have left the game. Type 'refresh' to display lobby\n"));
      threadpool.AddToQueue(SendSocketMsg(opponent.s, "Opponent has left the game. Type 'refresh' to display lobby\n"));
    }
    else if (player.currState == SPECTATE)
    {
      player.currState = IDLE;
      lobby.DeleteSpectator(player.s);
      threadpool.AddToQueue(SendSocketMsg(socket, "You have left the game. Type 'refresh' to display lobby\n"));
    }
    else
      threadpool.AddToQueue(SendSocketMsg(socket, "You are not in a game.\n"));
  }
}

UpdateUserWatch::UpdateUserWatch(SOCKET s, std::string input) : socket(s), userInput(input) {}
void UpdateUserWatch::operator()()
{
  {
    std::unique_lock<std::mutex> lock(lobby_mutex);
    if (userInput == "")
      threadpool.AddToQueue(SendSocketMsg(socket, "Please specify which game to watch. 'watch <username>'\n"));
    else
    {
      Player& player = lobby.FindPlayer(socket);

      if (player.currState == IDLE)
      {
        Player toWatch = lobby.FindPlayer(userInput);
        if (toWatch.currState == IN_GAME)
        {
          player.currState = SPECTATE;
          player.watch = toWatch.s;

          SOCKET s = lobby.FindPlayer(userInput).s;
          lobby.AddSpectator(s, player.s);

        }
        else
        {
          std::string msg = toWatch.name + " is not in game.\n";
          threadpool.AddToQueue(SendSocketMsg(socket, msg));
        }
      }
      else
        threadpool.AddToQueue(SendSocketMsg(socket, "You can't watch game.\n"));
    }
  }
}