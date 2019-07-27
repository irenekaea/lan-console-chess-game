#include "Piece.h"
#include "Board.h"
#include <iostream>
#define LEFT -1
#define RIGHT 1
#define UP -1
#define DOWN 1
#define ROOKBISHOP_DIR 4
#define KNIGHTKING_DIR 8
#define X 0
#define Y 1
#define TOTAL_AXIS 2

Piece::Piece(Colour c, const Position& pos)
  : m_colour(c)
  , m_pos(pos)
{

}

Piece::~Piece()
{
  
}

Colour Piece::colour() const
{
  return m_colour;
}

void Piece::position(const Position& pos)
{
  m_pos = pos;
}

const Position& Piece::position() const
{
  return m_pos;
}

Piece* Piece::Factory(PieceChar piece, const Position& pos)
{
  // Create a map in order to call the correct create function.
  static std::map<PieceChar, std::shared_ptr<Piece> > table
  {
    std::make_pair(PieceChar::WHITE_PAWN,   
      std::shared_ptr<Pawn>(new Pawn(WHITE, pos))),
    std::make_pair(PieceChar::WHITE_KNIGHT, 
      std::shared_ptr<Knight>(new Knight(WHITE, pos))),
    std::make_pair(PieceChar::WHITE_BISHOP, 
      std::shared_ptr<Bishop>(new Bishop(WHITE, pos))),
    std::make_pair(PieceChar::WHITE_ROOK,   
      std::shared_ptr<Rook>(new Rook(WHITE, pos))),
    std::make_pair(PieceChar::WHITE_QUEEN,  
      std::shared_ptr<Queen>(new Queen(WHITE, pos))),
    std::make_pair(PieceChar::WHITE_KING,   
      std::shared_ptr<King>(new King(WHITE, pos))),
    std::make_pair(PieceChar::BLACK_PAWN,   
      std::shared_ptr<Pawn>(new Pawn(BLACK, pos))),
    std::make_pair(PieceChar::BLACK_KNIGHT, 
      std::shared_ptr<Knight>(new Knight(BLACK, pos))),
    std::make_pair(PieceChar::BLACK_BISHOP, 
      std::shared_ptr<Bishop>(new Bishop(BLACK, pos))),
    std::make_pair(PieceChar::BLACK_ROOK,   
      std::shared_ptr<Rook>(new Rook(BLACK, pos))),
    std::make_pair(PieceChar::BLACK_QUEEN,  
      std::shared_ptr<Queen>(new Queen(BLACK, pos))),
    std::make_pair(PieceChar::BLACK_KING,   
      std::shared_ptr<King>(new King(BLACK, pos)))
  };
  // Return the appropraite piece created
  return table[piece]->Create(pos);
}

void Piece::BishopRookMoves(const Board& board, std::vector<Move>&moves, 
  int* case_1, int* case_2, int* case_3, int* case_4) const
{
  // Initialize first direction to check as LEFT and UP (diagonal)
  int dirX = case_4[X], dirY = case_4[Y];
  Position currPos = { m_pos.col() + dirX, m_pos.row() + dirY };
  SquareState nextTile = board.GetState(currPos);
  bool(*isOpponent)(SquareState) = (m_colour == WHITE) ? IsBlack : IsWhite;
  int noDir = ROOKBISHOP_DIR;
  
  // While loop through until all 4 direction were checked.
  while (noDir)
  {
    // If next tile is not own colour & is legal, add moves
    if (!IsIllegal(nextTile) && (IsEmpty(nextTile) || isOpponent(nextTile)))
      moves.push_back(Move{ m_pos, currPos });

    // If the tile contains an enemy / is an ally, change direction
    if (!IsEmpty(nextTile))
    {
      currPos = { m_pos };
      --noDir;
      switch (noDir)
      {
      case 1:
        dirX = case_1[X];
        dirY = case_1[Y];
        break;
      case 2:
        dirX = case_2[X];
        dirY = case_2[Y];
        break;
      case 3:
        dirX = case_3[X];
        dirY = case_3[Y];
        break;
      }
    }
    // Update current position we are checking
    currPos = { currPos.col() + dirX, currPos.row() + dirY };
    nextTile = board.GetState(currPos);
  }
}

void Piece::KnightKingMoves(const Board& board, std::vector<Move>& moves, 
  Position* movements) const
{
  SquareState nextTile;
  bool(*isOpponent)(SquareState) = (m_colour == WHITE) ? IsBlack : IsWhite;

  // For loop through to check if each square is empty / can eat an enemy
  for (int i = 0; i < KNIGHTKING_DIR; ++i)
  {
    // Get the state of next square
    nextTile = board.GetState(m_pos + movements[i]);
    if (!IsIllegal(nextTile) && (IsEmpty(nextTile) || isOpponent(nextTile)))
      moves.push_back(Move{ m_pos, m_pos + movements[i] });
  }

}

Pawn::Pawn(Colour c, const Position& pos)
  : Piece(c, pos)
{

}
/******************************************************************************/
/*! 
\brief
  Generation of all possible moves the pawn can make
\param board
  The board to check for possible moves
\param moves
  The vector container to store the list of possible moves
*/
/******************************************************************************/
void Pawn::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  SquareState nextTile;
  int dirY = (m_colour == WHITE) ? DOWN : UP;
  Position newPos = { m_pos.col(), m_pos.row() + dirY};
  bool(*isOpponent)(SquareState) = (m_colour == WHITE) ? IsBlack : IsWhite;

  // Moving up
  // Get the square state
  nextTile = board.GetState(newPos);
  // If the square is empty, pawn can move
  if (!IsIllegal(nextTile) && IsEmpty(nextTile))
    moves.push_back(Move{ m_pos, newPos });

  // Check left
  nextTile = board.GetState(Position{ newPos.col() + LEFT, newPos.row() });

  // If the diagonal square contains an enemy, the pawn can eat the opponent
  if (!IsIllegal(nextTile) && isOpponent(nextTile))
    moves.push_back(Move{ m_pos, Position{newPos.col() + LEFT, newPos.row()} });

  // Check right
  nextTile = board.GetState(Position{ newPos.col() + RIGHT, newPos.row() });
  // If the diagonal square contains an enemy, the pawn can eat the opponent
  if (!IsIllegal(nextTile) && isOpponent(nextTile))
    moves.push_back(Move{ m_pos,  Position{newPos.col() + RIGHT, newPos.row()} });
}


Pawn* Pawn::Create(const Position& pos) const
{
  return new Pawn{ colour(), pos };
}

Pawn* Pawn::Clone() const
{
  return new Pawn{ colour(), position() };
}


Knight::Knight(Colour c, const Position& pos)
  : Piece(c, pos)
{

}
/******************************************************************************/
/*! 
\brief
  Generation of all possible moves the knight can make
\param board
  The board to check for possible moves
\param moves
  The vector container to store the list of possible moves
*/
/******************************************************************************/
void Knight::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  // Stores an array of Knight movements
  // In all 8 direction the knight can move in a 'L' shape.
  Position knightMove[KNIGHTKING_DIR] = 
  {
    Position{ (LEFT  * 2) , UP},
    Position{ (RIGHT * 2) , UP},
    Position{ (LEFT  * 2) , DOWN},
    Position{ (RIGHT * 2) , DOWN},
    Position{ LEFT, (UP * 2)},
    Position{ LEFT, (DOWN * 2)},
    Position{ RIGHT,(UP * 2)},
    Position{ RIGHT,(DOWN * 2)}
  };
  KnightKingMoves(board, moves, knightMove);
}

Knight* Knight::Create(const Position& pos) const
{
  return new Knight{ colour(), pos };
}

Knight* Knight::Clone() const
{
  return new Knight{ colour(), position() };
}


Bishop::Bishop(Colour c, const Position& pos)
  : Piece(c, pos)
{

}
/******************************************************************************/
/*! 
\brief
  Generation of all possible moves the Bishop can make
\param board
  The board to check for possible moves
\param moves
  The vector container to store the list of possible moves
*/
/******************************************************************************/
void Bishop::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  // stores the direction of the bishop movements
  int case_1[TOTAL_AXIS] = { RIGHT, UP };
  int case_2[TOTAL_AXIS] = { RIGHT, DOWN };
  int case_3[TOTAL_AXIS] = { LEFT, DOWN };
  int case_4[TOTAL_AXIS] = { LEFT, UP };
  BishopRookMoves(board, moves, case_1, case_2, case_3, case_4);
}

Bishop* Bishop::Create(const Position& pos) const
{
  return new Bishop{ colour(), pos };
}

Bishop* Bishop::Clone() const
{
  return new Bishop{ colour(), position() };
}


Rook::Rook(Colour c, const Position& pos)
  : Piece(c, pos)
{

}

void Rook::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  // Store the direction of rook movements
  int case_1[TOTAL_AXIS] = { 0, UP };
  int case_2[TOTAL_AXIS] = { 0, DOWN };
  int case_3[TOTAL_AXIS] = { RIGHT, 0 };
  int case_4[TOTAL_AXIS] = { LEFT, 0 };
  BishopRookMoves(board, moves, case_1, case_2, case_3, case_4);
}

Rook* Rook::Create(const Position& pos) const
{
  return new Rook{ colour(), pos };
}

Rook* Rook::Clone() const
{
  return new Rook{ colour(), position() };
}


King::King(Colour c, const Position& pos)
  : Piece(c, pos)
{

}
/******************************************************************************/
/*! 
\brief
  Generation of all possible moves the King can make
\param board
  The board to check for possible moves
\param moves
  The vector container to store the list of possible moves
*/
/******************************************************************************/
void King::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  Position kingMove[KNIGHTKING_DIR] =
  {
    Position{ LEFT , 0},
    Position{ RIGHT, 0},
    Position{ 0    , UP},
    Position{ 0    , DOWN},
    Position{ LEFT, UP},
    Position{ LEFT, DOWN},
    Position{ RIGHT, UP},
    Position{ RIGHT, DOWN}
  };
  KnightKingMoves(board, moves, kingMove);
}

King* King::Create(const Position& pos) const
{
  return new King{ colour(), pos };
}

King* King::Clone() const
{
  return new King{ colour(), position() };
}


Queen::Queen(Colour c, const Position& pos)
  : Piece(c, pos)
  , Bishop(c, pos)
  , Rook(c, pos)
{

}
/******************************************************************************/
/*! 
\brief
  Generation of all possible moves the Queen can make
\param board
  The board to check for possible moves
\param moves
  The vector container to store the list of possible moves
*/
/******************************************************************************/
void Queen::GenMoves(const Board& board, std::vector<Move>& moves) const
{
  Rook::GenMoves(board, moves);
  Bishop::GenMoves(board, moves);
}

Queen* Queen::Create(const Position& pos) const
{
  return new Queen{ colour(), pos };
}

Queen* Queen::Clone() const
{
  return new Queen{ colour(), position() };
}

