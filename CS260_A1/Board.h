#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <complex>
#include <iostream>
#include <fstream>
#include <set>
#include "Piece.h"

enum PieceType
{
  EMPTY = 0,
  PAWN = 1,
  KNIGHT = 2,
  BISHOP = 3,
  ROOK = 4,
  QUEEN = 5,
  KING = 6
};

enum class SquareState
{
  EMPTY         = PieceType::EMPTY,
  WHITE_PAWN    = PieceType::PAWN,
  WHITE_KNIGHT  = PieceType::KNIGHT,
  WHITE_BISHOP  = PieceType::BISHOP,
  WHITE_ROOK    = PieceType::ROOK,
  WHITE_QUEEN   = PieceType::QUEEN,
  WHITE_KING    = PieceType::KING,

  BLACK_PIECE    = 8,
  BLACK_PAWN     = PieceType::PAWN + BLACK_PIECE,
  BLACK_KNIGHT   = PieceType::KNIGHT + BLACK_PIECE,
  BLACK_BISHOP   = PieceType::BISHOP + BLACK_PIECE,
  BLACK_ROOK     = PieceType::ROOK + BLACK_PIECE,
  BLACK_QUEEN    = PieceType::QUEEN + BLACK_PIECE,
  BLACK_KING     = PieceType::KING + BLACK_PIECE,
  ILLEGAL = 0xF,

};

bool IsBlack(SquareState s);
bool IsWhite(SquareState s);
bool IsEmpty(SquareState s);
bool IsIllegal(SquareState s);

const int EXTRA_ROWS = 4;
const int EXTRA_COLS = 4;
const int EXTRA_COLS_AT_FRONT = 2;
const int EXTRA_ROWS_ON_TOP = 2;

class Board
{
public:
  Board(int row = 8, int col = 8);
  Board(const Board&);
  ~Board();

  SquareState       GetState(const Position&) const;
  std::vector<Move> GetAllLegalMoves(Colour col) const;
  void              MakeMove(const Move &);
  std::string       GetBoard() const;
  char              GetSquareStateToChar(const SquareState&) const;


  int rows() const {
    return rows_;
  }
  int cols() const {
    return cols_;
  }



private:
  int ConvertPositionIntoIndex(const Position &pos) const
  {
    return  (EXTRA_COLS + cols_)*(EXTRA_ROWS_ON_TOP + pos.row())
      + pos.col() + EXTRA_COLS_AT_FRONT;
  }
  using PiecePtr = Piece *;
  SquareState& GetState(const Position&);
  PiecePtr & GetPiecePtr(const Position&);
  int rows_, cols_;
  std::vector<SquareState> states_;
  std::vector<PiecePtr> all_pieces_;
  std::set<PiecePtr> white_pieces_;
  std::set<PiecePtr> black_pieces_;

  std::string default_board;
};


#endif
