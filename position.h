#pragma once

//#ifndef POSITION_H
//#define POSITION_H

#include <complex>
#include <iostream>

#include "json.hpp"

/**
 * @brief Represents a 2D position (or vector) in Cartesian coordinates.
 *
 * Provides arithmetic operators for vector addition, subtraction, and scaling,
 * as well as JSON serialization support via the nlohmann::json library.
 */
class Position {

public:

  /** @brief Underlying coordinate value type. */
  typedef double                   value_type;

  /** @brief Complex number type used for interoperability. */
  typedef std::complex<value_type> complex;

  /** @brief X coordinate. */
  value_type x;

  /** @brief Y coordinate. */
  value_type y;

  /**
   * @brief Default constructor. Initializes position to (0, 0).
   */
  constexpr Position() : x(0), y(0) {
  }

  /**
   * @brief Construct from explicit x and y coordinates.
   * @param pos_x X coordinate.
   * @param pos_y Y coordinate.
   */
  constexpr Position(value_type pos_x, value_type pos_y) : x(pos_x), y(pos_y) {
  }

  /**
   * @brief Construct from a complex number (real → x, imag → y).
   * @param pos Complex number whose real and imaginary parts become x and y.
   */
  constexpr Position(const complex & pos) : x(pos.real()), y(pos.imag()) {
  }

  /**
   * @brief Add another position (vector addition).
   * @param other The position to add.
   * @return Reference to this object.
   */
  Position & operator += (const Position & other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  /**
   * @brief Subtract another position (vector subtraction).
   * @param other The position to subtract.
   * @return Reference to this object.
   */
  Position & operator -= (const Position & other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  /**
   * @brief Scale this position by a scalar factor.
   * @param v Scalar multiplier.
   * @return Reference to this object.
   */
  Position & operator *= (value_type v) {
    x *= v;
    y *= v;
    return *this;
  }

  /**
   * @brief Divide this position by a scalar factor.
   * @param v Scalar divisor.
   * @return Reference to this object.
   */
  Position & operator /= (value_type v) {
    x /= v;
    y /= v;
    return *this;
  }

};

/** @brief Unary negation. Returns a position with both coordinates negated. */
inline Position operator - (const Position & p) {
  return Position(-p.x, -p.y);
}

/** @brief Vector addition of two positions. */
inline Position operator + (const Position & p1, Position p2) {
  return Position(p1) += p2;
}

/** @brief Vector subtraction of two positions. */
inline Position operator - (const Position & p1, Position p2) {
  return Position(p1) -= p2;
}

/** @brief Scale position by a scalar (position * scalar). */
inline Position operator * (const Position & p, Position::value_type v) {
  return Position(p) *= v;
}

/** @brief Scale position by a scalar (scalar * position). */
inline Position operator * (Position::value_type v, const Position & p) {
  return Position(p) *= v;
}

/** @brief Divide position by a scalar. */
inline Position operator / (const Position & p, Position::value_type v) {
  return Position(p) /= v;
}

/**
 * @brief Stream output operator. Prints position as "(x,y)".
 * @param os Output stream.
 * @param p  Position to print.
 * @return Reference to output stream.
 */
inline std::ostream & operator << (std::ostream & os, const Position & p) {
  return os << "(" << p.x << "," << p.y << ")";
}

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Position.
   *
   * Serializes as a two-element JSON array [x, y].
   */
  template <>
  struct adl_serializer<Position> {

    /** @brief Serialize Position to JSON array [x, y]. */
    static void to_json(json & j, const Position & position) {
      j = json::array();
      j.push_back(position.x);
      j.push_back(position.y);
    }

    /** @brief Deserialize Position from JSON array [x, y]. */
    static void from_json(const json & j, Position & position) {
      position.x = j.at(0);
      position.y = j.at(1);
    }
  };
}

//#endif
