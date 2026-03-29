#pragma once

//#ifndef POINT_H
//#define POINT_H

#include "canvas.h"
#include "drawable.h"
#include "position.h"
#include "boundingbox.h"
#include "transformation.h"

#include "json.hpp"

/**
 * @brief A single-pixel point drawable.
 *
 * Sets exactly one pixel on the canvas at the given position.
 * Unlike Dot, no diameter is involved — the point always occupies exactly
 * one pixel regardless of scale.
 */
class Point : public Drawable {

public:

  /** @brief Color type (alias for the canvas pixel type, RGB). */
  typedef Canvas::value_type color_type;

  Position   pos;    ///< Pixel position of the point.
  color_type color;  ///< Point color.

  /** @brief Default constructor. */
  Point() = default;

  /**
   * @brief Construct a point at the given position.
   * @param position    Pixel position.
   * @param point_color Color (default black).
   */
  constexpr Point(
    const Position   & position,
    const color_type & point_color = color_type(0,0,0)
  )
  : pos(position), color(point_color) {
  }

  Point(const Point &) = default;
  Point(Point &&) = default;

  virtual ~Point() override {
  }

  Point & operator = (const Point &) = default;

  /**
   * @brief Set the single pixel at @c pos to @c color.
   * @param canvas Target canvas.
   * @return Reference to @p canvas.
   */
  virtual Canvas & draw(Canvas & canvas) const override {
    canvas[pos] = color;
    return canvas;
  }

  /**
   * @brief Apply a transformation to the point's position.
   * @param t Transformation to apply.
   * @return Reference to this point.
   */
  virtual Point & transform(const Transformation & t) override {
    pos = t*pos;
    return *this;
  }

  /**
   * @brief Compute the bounding box of this point.
   *
   * Returns a 1×1 box (half a unit in each direction) centered on @c pos.
   *
   * @return Bounding box.
   */
  virtual BoundingBox boundingBox() const override {
    return BoundingBox(
      { pos.x - 0.5, pos.y - 0.5 },
      { pos.x + 0.5, pos.y + 0.5 }
    );
  }

  /**
   * @brief Create a heap-allocated copy of this point.
   * @return Pointer to a new Point (caller takes ownership).
   */
  virtual Point * copy() const override {
    return new Point(*this);
  }
};

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Point.
   *
   * Serializes as a JSON object with keys "pos" and "color".
   */
  template <>
  struct adl_serializer<Point> {

    /** @brief Serialize Point to JSON. */
    static void to_json(json & j, const Point & p) {
      j = json::object();
      j["pos"  ] = p.pos;
      j["color"] = p.color;
    }

    /** @brief Deserialize Point from JSON. */
    static void from_json(const json & j, Point & p) {
      p.pos   = j.at("pos"  );
      p.color = j.at("color");
    }
  };
}

//#endif
