#include "Board.h"
#include "Piece.h"
#include <iterator>
#include <cctype>
#include <map>
#include <utility>

const char INTERSECTION = '+';
const char HORIZONTAL_WALL = '-';
const char VERTICAL_WALL = '|';

bool IsBlack(SquareState s)
{
  return (static_cast<int>(s)& 0x8) != 0 && s != SquareState::EMPTY;
}

bool IsWhite(SquareState s)
{
  return !IsBlack(s) && s != SquareState::EMPTY;
}

bool IsEmpty(SquareState s)
{
  return s == SquareState::EMPTY;
}

bool IsIllegal(SquareState s)
{
  return s == SquareState::ILLEGAL;
}

Board::Board(int row, int col)
  : rows_(row),
  cols_(col),
  default_board("RNBQKBNRPPPPPPPP\neeeeeeee\neeeeeeee\neeeeeeee\neeeeeeee\npppppppp\nrnbqkbnr"),
  states_((rows_ + EXTRA_ROWS) * (cols_ + EXTRA_COLS), SquareState::ILLEGAL),
  all_pieces_((rows_ + EXTRA_ROWS) * (cols_ + EXTRA_COLS), nullptr)
{
  static std::map<PieceChar, SquareState>
    CharToSquareState
  {
    { PieceChar::WHITE_PAWN, SquareState::WHITE_PAWN },
    { PieceChar::WHITE_KNIGHT, SquareState::WHITE_KNIGHT },
    { PieceChar::WHITE_BISHOP, SquareState::WHITE_BISHOP },
    { PieceChar::WHITE_QUEEN, SquareState::WHITE_QUEEN },
    { PieceChar::WHITE_ROOK, SquareState::WHITE_ROOK },
    { PieceChar::WHITE_KING, SquareState::WHITE_KING },
    { PieceChar::BLACK_PAWN, SquareState::BLACK_PAWN },
    { PieceChar::BLACK_KNIGHT, SquareState::BLACK_KNIGHT },
    { PieceChar::BLACK_BISHOP, SquareState::BLACK_BISHOP },
    { PieceChar::BLACK_QUEEN, SquareState::BLACK_QUEEN },
    { PieceChar::BLACK_ROOK, SquareState::BLACK_ROOK },
    { PieceChar::BLACK_KING, SquareState::BLACK_KING },
    { PieceChar::EMPTY, SquareState::EMPTY }
  };
  int row_index = 0;
  int col_index = 0;

  auto c = default_board.begin();
  while (c != default_board.end())
  {
    if (std::isalpha(*c))
    {
      Position new_position(col_index, row_index);
      int new_position_index = ConvertPositionIntoIndex(new_position);
      PieceChar pc = static_cast<PieceChar>(*c);
      auto state = CharToSquareState[pc];
      states_[new_position_index] = state;
      if (!IsEmpty(state))
      {
        auto& pieces = IsBlack(state) ? black_pieces_ : white_pieces_;
        auto piece_ptr
          = Piece::Factory(static_cast<PieceChar>(*c),
          Position(col_index, row_index));
        pieces.insert(piece_ptr);
        all_pieces_[new_position_index] = piece_ptr;
      }
      col_index = (col_index + 1) % cols_;
      row_index += (col_index == 0);
    }
    ++c;
  }
  std::cout << std::endl;
}

SquareState Board::GetState(const Position &p) const
{
  return states_[ConvertPositionIntoIndex(p)];
}


std::vector<Move> Board::GetAllLegalMoves(Colour col) const
{
  std::vector<Move> all_legal_moves;
  auto& pieces = col == BLACK ? black_pieces_ : white_pieces_;
  for (const auto & elem : pieces)
  {
    elem->GenMoves(*this, all_legal_moves);
  }
  return all_legal_moves;
}

Board::PiecePtr & Board::GetPiecePtr(const Position& p)
{
  return all_pieces_[ConvertPositionIntoIndex(p)];
}


SquareState& Board::GetState(const Position &p)
{
  return states_[ConvertPositionIntoIndex(p)];
}

void Board::MakeMove(const Move& move)
{
  auto& src_piece_ptr = GetPiecePtr(move.orig_pos);
  auto& dest_piece_ptr = GetPiecePtr(move.dest_pos);
  if (dest_piece_ptr)
  {
    auto& piece_set = (dest_piece_ptr->colour() == BLACK)
      ? black_pieces_ : white_pieces_;
    piece_set.erase(dest_piece_ptr);
  }
  dest_piece_ptr = src_piece_ptr;
  src_piece_ptr->position(move.dest_pos);
  src_piece_ptr = nullptr;
  GetState(move.dest_pos) = GetState(move.orig_pos);
  GetState(move.orig_pos) = SquareState::EMPTY;
}

std::string Board::GetBoard() const
{
  std::string currentBoard;
  int h = rows_;
  int w = cols_;
  currentBoard.push_back(' ');
  for (char c = 'A'; c <= 'H'; ++c)
  {
    currentBoard.push_back(' ');
    currentBoard.push_back(c);
  }
  currentBoard.push_back('\n');
  currentBoard.push_back(' ');
  currentBoard.push_back(INTERSECTION);
  for (int j = 0; j < w; ++j)
  {
    currentBoard.push_back(HORIZONTAL_WALL);
    currentBoard.push_back(INTERSECTION);
  }
  currentBoard.push_back('\n');
  for (int i = 0; i<h; ++i)
  {
    currentBoard.push_back( (char)i + '0');
    currentBoard.push_back(VERTICAL_WALL);
    for (int j = 0; j<w; ++j)
    {
      char c = GetSquareStateToChar(GetState(Position(j, i)));
      currentBoard.push_back(c);
      currentBoard.push_back(VERTICAL_WALL);
    }
    currentBoard.push_back('\n');
    currentBoard.push_back(' ');
    currentBoard.push_back(INTERSECTION);
    for (int j = 0; j < w; ++j)
    {
      currentBoard.push_back(HORIZONTAL_WALL);
      currentBoard.push_back(INTERSECTION);
    }
    currentBoard.push_back('\n');
  }

  return currentBoard;
}

char Board::GetSquareStateToChar(const SquareState& s) const
{
  static std::map<SquareState, PieceChar>
    SquareStateToChar
  {
    { SquareState::WHITE_PAWN,   PieceChar::WHITE_PAWN },
    { SquareState::WHITE_KNIGHT, PieceChar::WHITE_KNIGHT, },
    { SquareState::WHITE_BISHOP, PieceChar::WHITE_BISHOP },
    { SquareState::WHITE_QUEEN,  PieceChar::WHITE_QUEEN },
    { SquareState::WHITE_ROOK,   PieceChar::WHITE_ROOK },
    { SquareState::WHITE_KING,   PieceChar::WHITE_KING },
    { SquareState::BLACK_PAWN,   PieceChar::BLACK_PAWN },
    { SquareState::BLACK_KNIGHT, PieceChar::BLACK_KNIGHT },
    { SquareState::BLACK_BISHOP, PieceChar::BLACK_BISHOP },
    { SquareState::BLACK_QUEEN,  PieceChar::BLACK_QUEEN },
    { SquareState::BLACK_ROOK,   PieceChar::BLACK_ROOK },
    { SquareState::BLACK_KING,   PieceChar::BLACK_KING },
    { SquareState::EMPTY,        PieceChar::EMPTY }
  };
 return ((s == SquareState::EMPTY) ? ' ' :
    static_cast<char>(SquareStateToChar[s]));
}

Board::~Board()
{
  for (auto& elem : white_pieces_)
  {
    delete elem;
  }
  for (auto& elem : black_pieces_)
  {
    delete elem;
  }

}

Board::Board(const Board& rhs)
  : rows_(rhs.rows_),
  cols_(rhs.cols_),
  default_board(rhs.default_board),
  states_(rhs.states_),
  all_pieces_(rhs.all_pieces_.size(), nullptr)
{
  std::set<PiecePtr> const * pieces_set_ptrs[] = { &rhs.white_pieces_,
    &rhs.black_pieces_
  };

  for (const auto & piece_set : pieces_set_ptrs)
  {
    for (const auto & piece_ptr : *piece_set)
    {
      auto ptr = piece_ptr->Clone();
      auto& pieces = ptr->colour() == BLACK ?
      black_pieces_ : white_pieces_;
      pieces.insert(ptr);
      auto index = ConvertPositionIntoIndex(ptr->position());
      all_pieces_[index] = ptr;
    }
  }
}
