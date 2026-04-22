#pragma once

//#ifndef BOUNDINGBOX_H
//#define BOUNDINGBOX_H

#include "position.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>

/**
 * @brief Describes a 2D screen resolution in pixels.
 */
struct ScreenSize {
  unsigned long width;   ///< Width in pixels.
  unsigned long height;  ///< Height in pixels.
};

/**
 * @brief Axis-aligned bounding box in 2D space.
 *
 * Tracks the minimum and maximum extents of a set of 2D positions.
 * An empty bounding box is valid and represents the absence of any geometry.
 * Supports incremental expansion via operator+=, scaling via operator*=,
 * and conversion to a pixel-based ScreenSize.
 */
class BoundingBox {

private:

  bool     is_empty;
  Position corner1; ///< Bottom-left corner (min_x, min_y).
  Position corner2; ///< Top-right corner (max_x, max_y).

public:

  /**
   * @brief Default constructor. Creates an empty bounding box.
   */
  BoundingBox() : is_empty(true) {
  }

  /**
   * @brief Construct a degenerate bounding box from a single position.
   * @param p The single point; both corners are set to this position.
   */
  BoundingBox(const Position & p)
  : is_empty(false), corner1(p), corner2(p) {
  }

  /**
   * @brief Construct from two corner positions.
   *
   * The constructor normalizes the corners so that corner1 is always the
   * bottom-left (minimum) and corner2 is the top-right (maximum).
   *
   * @param p1 First corner.
   * @param p2 Second corner (opposite diagonal).
   */
  BoundingBox(const Position & p1, const Position & p2) :
  is_empty(false),
  corner1(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
  corner2(std::max(p1.x, p2.x), std::max(p1.y, p2.y)) {
  }

  /**
   * @brief Construct as the bounding box of an initializer list of positions.
   * @param poslist List of positions to enclose. An empty list yields an empty box.
   */
  BoundingBox(std::initializer_list<Position> poslist) : is_empty(true) {
    for (auto & pos : poslist) {
      *this += pos;
    }
  }

  /**
   * @brief Construct as the bounding box of a vector of positions.
   * @param poslist Vector of positions to enclose. An empty vector yields an empty box.
   */
  BoundingBox(const std::vector<Position> & poslist) : is_empty(true) {
    for (auto & pos : poslist) {
      *this += pos;
    }
  }

  /**
   * @brief Check whether the bounding box contains no geometry.
   * @return true if empty, false otherwise.
   */
  bool empty() const {
    return is_empty;
  }

  /**
   * @brief Return the width of the bounding box.
   * @return Absolute difference between the x coordinates of the two corners.
   */
  Position::value_type width() const {
    return std::abs(corner2.x - corner1.x);
  }

  /**
   * @brief Return the height of the bounding box.
   * @return Absolute difference between the y coordinates of the two corners.
   */
  Position::value_type height() const {
    return std::abs(corner2.y - corner1.y);
  }

  /**
   * @brief Return the bottom-left corner (minimum x and y).
   * @return Bottom-left corner position.
   */
  Position bottom_left() const {
    return corner1;
  }

  /**
   * @brief Return the top-right corner (maximum x and y).
   * @return Top-right corner position.
   */
  Position top_right() const {
    return corner2;
  }

  /**
   * @brief Compute a pixel ScreenSize that preserves the aspect ratio.
   *
   * The total pixel count equals approximately @p resolution.
   *
   * @param resolution Desired total number of pixels (width × height).
   * @return ScreenSize with width and height that match the box's aspect ratio.
   */
  ScreenSize screenSize(unsigned long resolution) const {
    return ScreenSize {
      static_cast<unsigned long>(sqrt(static_cast<double>(resolution) * width () / height())),
      static_cast<unsigned long>(sqrt(static_cast<double>(resolution) * height() / width ()))
    };
  }

  /**
   * @brief Expand the bounding box to include a single position.
   * @param p Position to include.
   * @return Reference to this object.
   */
  BoundingBox & operator += (const Position & p) {

    if (is_empty) {
      is_empty = false;
      corner1  = corner2 = p;
    }
    else {
      corner1.x = std::min(corner1.x, p.x);
      corner1.y = std::min(corner1.y, p.y);
      corner2.x = std::max(corner2.x, p.x);
      corner2.y = std::max(corner2.y, p.y);
    }

    return *this;
  }

  /**
   * @brief Expand this bounding box to also enclose another bounding box.
   * @param other The other bounding box. Has no effect if @p other is empty.
   * @return Reference to this object.
   */
  BoundingBox & operator += (const BoundingBox & other) {

    if (is_empty) {
      corner1  = other.corner1;
      corner2  = other.corner2;
      is_empty = other.is_empty;
    }
    else {
      if (!other.is_empty) {
        corner1.x = std::min(corner1.x, other.corner1.x);
        corner1.y = std::min(corner1.y, other.corner1.y);
        corner2.x = std::max(corner2.x, other.corner2.x);
        corner2.y = std::max(corner2.y, other.corner2.y);
      }
    }

    return *this;
  }

  /**
   * @brief Scale the bounding box around its center.
   * @param factor Scaling factor (e.g. 1.05 adds a 5% margin).
   * @return Reference to this object.
   */
  BoundingBox & operator *= (const Position::value_type & factor) {

    // scale bounding box by a factor

    auto center = (corner1+corner2) / 2.0;
    auto half_w = width () * factor / 2.0;
    auto half_h = height() * factor / 2.0;

    corner1.x = center.x - half_w;
    corner1.y = center.y - half_h;
    corner2.x = center.x + half_w;
    corner2.y = center.y + half_h;

    return *this;
  }
};

/** @brief Return a new box that also encloses position @p p. */
inline BoundingBox operator + (const BoundingBox & b, const Position & p) {
  return BoundingBox(b) += p;
}

/** @brief Return a new box that also encloses position @p p (commutative). */
inline BoundingBox operator + (const Position & p, const BoundingBox & b) {
  return BoundingBox(b) += p;
}

/** @brief Return the union of two bounding boxes. */
inline BoundingBox operator + (const BoundingBox & b1, const BoundingBox & b2) {
  return BoundingBox(b1) += b2;
}

/** @brief Return a copy of @p b scaled by @p factor around its center. */
inline BoundingBox operator * (const BoundingBox & b, const Position::value_type & factor) {
  return BoundingBox(b) *= factor;
}

/** @brief Return a copy of @p b scaled by @p factor around its center (commutative). */
inline BoundingBox operator * (const Position::value_type & factor, const BoundingBox & b) {
  return BoundingBox(b) *= factor;
}

/**
 * @brief Stream output operator. Prints "[]" for empty boxes, or "[corner1-corner2]".
 * @param os Output stream.
 * @param b  BoundingBox to print.
 * @return Reference to output stream.
 */
inline std::ostream & operator << (std::ostream & os, const BoundingBox & b) {
  return
  b.empty()
  ? os << "[]"
  : os << "[" << b.bottom_left() << "-" << b.top_right() << "]";
}

// #endif
