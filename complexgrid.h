#pragma once

//#ifndef COMPLEXGRID_H
//#define COMPLEXGRID_H

#include <vector>
#include <complex>
#include <iostream>
#include <algorithm>

/**
 * @brief A regular 2D grid of complex numbers covering a rectangular region.
 *
 * The grid spans a rectangular region in the complex plane defined by two
 * corner values.  Given a target resolution (total node count), the grid
 * dimensions are chosen to preserve the aspect ratio of the region.  Nodes
 * are stored in row-major order from top-left to bottom-right (i.e. the
 * imaginary component decreases as the row index increases).
 *
 * Typical use: pre-computing the complex sample points for a Mandelbrot or
 * Julia set renderer.
 */
class ComplexGrid {

public:

  /** @brief Size / index type. */
  typedef size_t                               size_type;

  /** @brief Complex number type used for grid nodes. */
  typedef std::complex<double>                 complex;

  /** @brief Const iterator over grid nodes in row-major order. */
  typedef std::vector<complex>::const_iterator const_iterator;

private:

  complex              bl;    ///< Bottom-left corner of the region.
  complex              tr;    ///< Top-right corner of the region.
  unsigned long        res;   ///< Requested resolution (approximate total node count).
  size_type            w;     ///< Actual grid width (columns).
  size_type            h;     ///< Actual grid height (rows).
  std::vector<complex> nodes; ///< Grid nodes in row-major order (top-left first).

public:

  ComplexGrid() = delete;

  /**
   * @brief Construct a grid over a complex rectangle.
   *
   * The two corner arguments may be given in any order; the constructor
   * normalises them so that @c bl holds the minimum real and imaginary parts
   * and @c tr holds the maximum.  Grid dimensions are calculated to match the
   * aspect ratio while keeping the total node count close to @p resolution.
   *
   * Nodes are filled row by row from the top-left corner, with the real part
   * increasing and the imaginary part decreasing.
   *
   * @param corner1    One corner of the complex rectangular region.
   * @param corner2    The opposite corner.
   * @param resolution Approximate desired total number of grid nodes.
   */
  ComplexGrid(
    const complex & corner1,
    const complex & corner2,
    unsigned long   resolution
  ) :

  bl(complex{ std::min(corner1.real(), corner2.real()), std::min(corner1.imag(), corner2.imag()) }),
  tr(complex{ std::max(corner1.real(), corner2.real()), std::max(corner1.imag(), corner2.imag()) }),

  res(resolution),

  w(static_cast<size_type>(sqrt(res * (tr.real()-bl.real()) / (tr.imag()-bl.imag())))),
  h(static_cast<size_type>(sqrt(res * (tr.imag()-bl.imag()) / (tr.real()-bl.real())))),

  nodes(w*h)

  {

    complex             current(bl.real(), tr.imag());
    complex::value_type dx = (tr.real()-bl.real()) / w;
    complex::value_type dy = (tr.imag()-bl.imag()) / h;
    size_type           n  = 0;

    for (size_type y = 0; y < h; ++y) {
      for (size_type x = 0; x < w; ++x) {
        nodes[n] = current;
        ++n;
        current.real(current.real() + dx);
      }
      current.real(bl.real());
      current.imag(current.imag() - dy);
    }
  }

  ComplexGrid(const ComplexGrid &) = delete;
  ComplexGrid(ComplexGrid &&) = default;
  ComplexGrid & operator = (const ComplexGrid &) = delete;
  ComplexGrid & operator = (ComplexGrid &&) = default;

  /**
   * @brief Return the grid width (number of columns).
   * @return Column count.
   */
  size_type width() const {
    return w;
  }

  /**
   * @brief Return the grid height (number of rows).
   * @return Row count.
   */
  size_type height() const {
    return h;
  }

  /**
   * @brief Return the bottom-left corner of the covered region.
   * @return Bottom-left complex value.
   */
  complex bottom_left() const {
    return bl;
  }

  /**
   * @brief Return the top-right corner of the covered region.
   * @return Top-right complex value.
   */
  complex top_right() const {
    return tr;
  }

  /**
   * @brief Return the requested resolution.
   * @return Approximate total node count passed to the constructor.
   */
  unsigned long resolution() const {
    return res;
  }

  /**
   * @brief Return the total number of grid nodes.
   * @return width() * height().
   */
  size_type size() const {
    return nodes.size();
  }

  /**
   * @brief Access a node by (column, row) index.
   * @param x Column index (0-based).
   * @param y Row index (0-based, 0 = top row).
   * @return Const reference to the complex node value.
   */
  const complex & operator() (size_type x, size_type y) const {
    return nodes[w*y+x];
  }

  /**
   * @brief Access a node by linear index in row-major order.
   * @param n Linear index (0-based).
   * @return Const reference to the complex node value.
   */
  const complex & operator [] (size_type n) const {
    return nodes[n];
  }

  /**
   * @brief Return a const iterator to the first node.
   * @return Begin iterator.
   */
  const_iterator begin() const {
    return nodes.begin();
  }

  /**
   * @brief Return a const iterator past the last node.
   * @return End iterator.
   */
  const_iterator end() const {
    return nodes.end();
  }
};

/**
 * @brief Stream output operator for ComplexGrid.
 *
 * Prints all nodes in row-major order, space-separated within each row,
 * with rows separated by newlines.
 *
 * @param os   Output stream.
 * @param grid Grid to print.
 * @return Reference to @p os.
 */
inline std::ostream & operator << (std::ostream & os, const ComplexGrid & grid) {

  for (ComplexGrid::size_type n = 0; n < grid.size(); ++n) {
    auto node = grid[n];
    auto x    = n % grid.width();
    if (n > 0 && x == 0) {
      os << "\n";
    }
    if (x > 0) {
      os << " ";
    }
    os << node;
  }

  return os;
}

//#endif
