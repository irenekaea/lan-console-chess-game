#include "NewGame.h"
#include <algorithm>

NewGame::NewGame(Player white_player, Player black_player)
  : white(white_player), black(black_player), turn(WHITE)
{

}

NewGame::NewGame() 
  : white(Player(INVALID_SOCKET, "", IDLE))
  , black(Player(INVALID_SOCKET, "", IDLE))
  , turn(WHITE)
{

}

NewGame& NewGame::operator=(const NewGame& game)
{
  board = game.board;
  white = game.white;
  black = game.black;
  turn = game.turn;
  std::copy(game.spec.begin(), game.spec.end(), std::back_inserter(spec));
  return *this;
}

bool NewGame::operator==(const NewGame& game) const
{
  if (game.white.s != white.s || game.black.s != game.black.s)
    return false;
  return true;
}

void NewGame::PrintBoard() const
{
  std::string currBoard = board.GetBoard();

  threadpool.AddToQueue(SendSocketMsg(white.s, currBoard));
  threadpool.AddToQueue(SendSocketMsg(black.s, currBoard));

  if (turn == WHITE)
  {
    threadpool.AddToQueue(SendSocketMsg(white.s, "Your turn: \n"));
    threadpool.AddToQueue(SendSocketMsg(black.s, "Waiting for opponent moves...\n"));
  }
  else
  {
    threadpool.AddToQueue(SendSocketMsg(black.s, "Your turn: \n"));
    threadpool.AddToQueue(SendSocketMsg(white.s, "Waiting for opponent moves...\n"));
  }

  for (SOCKET s : spec)
  {
    Player player = turn == WHITE ? white : black;
    std::string msg = currBoard + "It's " + player.name + " turn.\n";
    threadpool.AddToQueue(SendSocketMsg(s, msg));
  }
}

void NewGame::PrintBoard(SOCKET s) const
{
  std::string currBoard = board.GetBoard();
  threadpool.AddToQueue(SendSocketMsg(s, currBoard));
  
  if (s == white.s && turn == WHITE || s == black.s && turn == BLACK)
    threadpool.AddToQueue(SendSocketMsg(s, "Your turn:\n"));
  else
    threadpool.AddToQueue(SendSocketMsg(s, "Waiting for opponent moves...\n"));
}

void NewGame::PrintBoardSpec(SOCKET s) const
{
  std::string currBoard = board.GetBoard();
  Player player = turn == WHITE ? white : black;
  std::string msg = currBoard + "It's " + player.name + " turn.\n";
  threadpool.AddToQueue(SendSocketMsg(s, msg));
}

SOCKET NewGame::GetOpponentSocket(Player p)
{
  if (p.s == white.s)
    return black.s;
  return white.s;
}

bool NewGame::CheckPlayer(SOCKET s) const
{
  if (s == white.s || s == black.s)
    return true;
  return false;
}

bool NewGame::CheckSpectators(SOCKET s) const
{
  for (SOCKET socket : spec)
  {
    if (socket == s)
      return true;
  }
  return false;
}

void NewGame::AddSpectators(SOCKET s)
{
  spec.push_back(s);
}
void NewGame::DeleteSpectators(SOCKET s)
{
  auto itr = std::find(spec.begin(), spec.end(), s);
  if (itr != spec.end())
    spec.erase(itr);
}

void NewGame::MakeMove(const Move& move)
{
  board.MakeMove(move);
  turn = turn == WHITE ? BLACK : WHITE;
  PrintBoard();
}

size_t NewGame::SpecSize() const
{
  return spec.size();
}