#pragma once

//#ifndef POLYGON_H
//#define POLYGON_H

#include "dot.h"
#include "line.h"
#include "canvas.h"
#include "drawable.h"
#include "position.h"
#include "boundingbox.h"
#include "transformation.h"

#include "json.hpp"

#include <vector>
#include <initializer_list>

/**
 * @brief An open or closed polygon drawable.
 *
 * Inherits from both Drawable and std::vector<Position>, so it stores its
 * vertices directly and supports standard vector operations.  Drawing
 * connects consecutive vertices with thick Line segments.  If the first and
 * last vertices are the same the polygon is visually closed.
 */
class Polygon : public Drawable, public std::vector<Position> {

public:

  /** @brief Color type (alias for the canvas pixel type, RGB). */
  typedef Canvas::value_type color_type;

  color_type color;  ///< Edge color.
  double     width;  ///< Edge width in canvas units.

  /** @brief Default constructor. Creates an empty polygon. */
  Polygon() = default;

  /**
   * @brief Construct a polygon from a list of vertices.
   * @param vertices   Vertex positions.
   * @param line_color Edge color (default black).
   * @param line_width Edge width in canvas units (default 1.0).
   */
  Polygon(
    std::initializer_list<Position> vertices,
    const color_type & line_color = color_type(0,0,0),
    const double     & line_width = 1.0
  )
  : std::vector<Position>(vertices), color(line_color), width(line_width) {
  }

  Polygon(const Polygon &) = default;
  Polygon(Polygon &&) = default;

  virtual ~Polygon() override {
  }

  Polygon & operator = (const Polygon &) = default;
  Polygon & operator = (Polygon &&) = default;

  /**
   * @brief Factory: create an axis-aligned rectangle.
   *
   * The rectangle visits corner1, then the two remaining corners, then
   * corner2, and finally returns to corner1 (5 vertices including closure).
   *
   * @param corner1    One corner of the rectangle.
   * @param corner2    The diagonally opposite corner.
   * @param line_color Edge color (default black).
   * @param line_width Edge width in canvas units (default 1.0).
   * @return A Polygon representing the rectangle.
   */
  static Polygon rectangle(
    const Position   & corner1,
    const Position   & corner2,
    const color_type & line_color = color_type(0,0,0),
    const double     & line_width = 1.0
  ) {
    return Polygon(
      { corner1, { corner1.x, corner2.y }, corner2, { corner2.x, corner1.y }, corner1 },
      line_color,
      line_width
    );
  }

  /**
   * @brief Draw the polygon as a sequence of thick line segments.
   *
   * The first vertex is drawn as a Dot to cap the start of the path.
   * Each subsequent vertex is connected to the previous one with a Line.
   *
   * @param canvas Target canvas.
   * @return Reference to @p canvas.
   */
  virtual Canvas & draw(Canvas & canvas) const override {

    if (size() > 0) {
      auto v1 = (*this)[0];
      canvas << Dot(v1, color, width);
      for (size_type i = 1; i < size(); ++i) {
        auto v2 = (*this)[i];
        canvas << Line(v1, v2, color, width);
        v1 = v2;
      }
    }

    return canvas;
  }

  /**
   * @brief Apply a transformation to all vertices.
   * @param t Transformation to apply.
   * @return Reference to this polygon.
   */
  virtual Polygon & transform(const Transformation & t) override {
    for (auto & vertex : *this) {
      vertex = t*vertex;
    }
    return *this;
  }

  /**
   * @brief Compute the axis-aligned bounding box of all vertices.
   * @return Bounding box enclosing all vertices.
   */
  virtual BoundingBox boundingBox() const override {
    return BoundingBox(*this);
  }

  /**
   * @brief Create a heap-allocated copy of this polygon.
   * @return Pointer to a new Polygon (caller takes ownership).
   */
  virtual Polygon * copy() const override {
    return new Polygon(*this);
  }
};

/**
 * @brief Stream output operator. Prints vertices as "[v0-v1-...-vN]".
 * @param os      Output stream.
 * @param polygon Polygon to print.
 * @return Reference to @p os.
 */
inline std::ostream & operator << (std::ostream & os, const Polygon & polygon) {

  os << "[";
  std::string sep = "";

  for (auto & vertex : polygon) {
    os << sep << vertex;
    sep = "-";
  }

  os << "]";
  return os;
}

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Polygon.
   *
   * Serializes as a JSON object with keys "color", "width", and "vertices".
   */
  template <>
  struct adl_serializer<Polygon> {

    /** @brief Serialize Polygon to JSON. */
    static void to_json(json & j, const Polygon & p) {
      j = json::object();
      j["color"   ] = p.color;
      j["width"   ] = p.width;
      j["vertices"] = dynamic_cast<const std::vector<Position> &>(p);
    }

    /** @brief Deserialize Polygon from JSON. */
    static void from_json(const json & j, Polygon & p) {
      p.clear();
      p.color = j.at("color"   );
      p.width = j.at("width"   );
      auto v  = j.at("vertices").get<std::vector<Position>>();
      p.reserve(v.size());
      for (auto & vertex : v) {
        p.push_back(vertex);
      }
    }
  };
}

//#endif
