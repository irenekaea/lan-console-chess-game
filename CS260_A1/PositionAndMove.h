#ifndef POSITION_AND_MOVE_H
#define POSITION_AND_MOVE_H
#include <complex>

enum Colour
{
    BLACK = 0,
    WHITE = 1
};

struct Position : std::complex<int>
{
    Position(int x=0, int y=0) : std::complex<int>(x, y) {}
    Position(const std::complex<int> c) :std::complex<int>(c.real(), c.imag()) {}
    int row() const {
        return imag();
    };
    int col() const {
        return real();
    }
};

struct Move
{
    Position orig_pos, dest_pos;
};
#endif
