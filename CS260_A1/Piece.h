#ifndef PIECE_H
#define PIECE_H
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include "PositionAndMove.h"

class Board;

enum class PieceChar : char
{
  WHITE_PAWN = 'P',
  WHITE_KNIGHT = 'N',
  WHITE_BISHOP = 'B',
  WHITE_ROOK = 'R',
  WHITE_QUEEN = 'Q',
  WHITE_KING = 'K',

  BLACK_PAWN = 'p',
  BLACK_KNIGHT = 'n',
  BLACK_BISHOP = 'b',
  BLACK_ROOK = 'r',
  BLACK_QUEEN = 'q',
  BLACK_KING = 'k',
  EMPTY = 'e',
};

// Base class of each individual piece on the board
class Piece
{
public:
  Piece(Colour c, const Position& pos);
  virtual ~Piece();
  
  // Generation of all legal moves
  virtual void    GenMoves(const Board& board, std::vector<Move>& moves) const = 0;
  // Create of each piece
  virtual Piece*  Create(const Position& pos) const = 0;
  // Cloning of each piece
  virtual Piece*  Clone() const = 0;
  // Returns the colour of each piece
  Colour          colour() const;
  // Set the position of each piece
  void            position(const Position& pos);
  // Return the position of each piece
  const Position& position() const;
  // A factory to create all types of the pieces
  static Piece*  Factory(PieceChar piece, const Position& pos);

protected:
  Colour m_colour;
  Position m_pos;

  // Helper function to generate movements
  void BishopRookMoves(const Board& board, std::vector<Move>& moves, int* case_1, int* case_2, int* case_3, int* case_4) const;
  void KnightKingMoves(const Board& board, std::vector<Move>& moves, Position* movements) const;
};

class Pawn : public Piece
{
public:
  Pawn(Colour c, const Position& pos);

  // Overriding base class functions
  void  GenMoves(const Board& board, std::vector<Move>& moves) const;
  Pawn* Create(const Position& pos) const;
  Pawn* Clone() const;
};

class Knight : public Piece
{
public:
  Knight(Colour c, const Position& pos);

  // Overriding base class functions
  void    GenMoves(const Board& board, std::vector<Move>& moves) const;
  Knight* Create(const Position& pos) const;
  Knight* Clone() const;
};

class Bishop : public virtual Piece
{
public:
  Bishop(Colour c, const Position& pos);

  // Overriding base class functions
  void    GenMoves(const Board& board, std::vector<Move>& moves) const;
  Bishop* Create(const Position& pos) const;
  Bishop* Clone() const;
};

class Rook : public virtual Piece
{
public:
  Rook(Colour c, const Position& pos);

  // Overriding base class functions
  void  GenMoves(const Board& board, std::vector<Move>& moves) const;
  Rook* Create(const Position& pos) const;
  Rook* Clone() const;
};

class King : public Piece
{
public:
  King(Colour c, const Position& pos);

  // Overriding base class functions
  void  GenMoves(const Board& board, std::vector<Move>& moves) const;
  King* Create(const Position& pos) const override;
  King* Clone() const;
};

// Defines to fix virtual inheritance bug for cl compiler
#ifdef _WIN32
class Queen : public virtual Piece, public Bishop, public Rook
#else
class Queen : public Bishop, public Rook
#endif
{
public:
  Queen(Colour c, const Position& pos);

  // Overriding base class functions
  void   GenMoves(const Board& board, std::vector<Move>& moves) const;
  Queen* Create(const Position& pos) const;
  Queen* Clone() const;
};
#endif

