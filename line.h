#pragma once

//#ifndef LINE_H
//#define LINE_H

#include "canvas.h"
#include "drawable.h"
#include "position.h"
#include "boundingbox.h"
#include "transformation.h"

#include <cmath>
#include <algorithm>

#include "json.hpp"

/**
 * @brief A thick line segment drawable.
 *
 * Draws a line segment with a given width using a per-pixel distance test:
 * each pixel within half the line width from the segment is colored.
 * This produces rounded endpoints and smooth edges regardless of orientation.
 */
class Line : public Drawable {

public:

  /** @brief Color type (alias for the canvas pixel type, RGB). */
  typedef Canvas::value_type color_type;

  Position   from;   ///< Start point of the line segment.
  Position   to;     ///< End point of the line segment.
  color_type color;  ///< Line color.
  double     width;  ///< Line width (diameter) in canvas units.

private:

  /**
   * @brief Compute the perpendicular distance from point (px, py) to the
   *        line segment from (x0, y0) to (x1, y1).
   *
   * If the segment is degenerate (zero length), returns the distance from
   * the point to the single endpoint.
   *
   * @param px X coordinate of the test point.
   * @param py Y coordinate of the test point.
   * @param x0 X coordinate of the segment start.
   * @param y0 Y coordinate of the segment start.
   * @param x1 X coordinate of the segment end.
   * @param y1 Y coordinate of the segment end.
   * @return Euclidean distance from (px, py) to the nearest point on the segment.
   */
  double pointToSegmentDistance(double px, double py, double x0, double y0, double x1, double y1) const {

    double dx        = x1 - x0;
    double dy        = y1 - y0;
    double length_sq = dx * dx + dy * dy;

    if (length_sq == 0.0) {
      dx = px - x0;
      dy = py - y0;
      return std::sqrt(dx * dx + dy * dy);
    }

    double t = ((px - x0) * dx + (py - y0) * dy) / length_sq;
    t = std::max(0.0, std::min(1.0, t));

    double proj_x = x0 + t * dx;
    double proj_y = y0 + t * dy;

    dx = px - proj_x;
    dy = py - proj_y;

    return std::sqrt(dx * dx + dy * dy);
  }

public:

  /**
   * @brief Default constructor. Creates a zero-length black line at the origin.
   */
  Line() : from(0,0), to(0,0), color(0,0,0), width(0) {
  }

  /**
   * @brief Construct a line segment with explicit endpoints, color, and width.
   * @param pos_from   Start position.
   * @param pos_to     End position.
   * @param line_color Line color (default black).
   * @param line_width Line width in canvas units (default 1.0).
   */
  constexpr Line(
    const Position   & pos_from,
    const Position   & pos_to,
    const color_type & line_color = color_type(0,0,0),
    const double     & line_width = 1.0
  )
  : from(pos_from), to(pos_to), color(line_color), width(line_width) {
  }

  virtual ~Line() override = default;

  /**
   * @brief Compute the Euclidean length of the line segment.
   * @return Length in canvas units.
   */
  double length() const {
    return sqrt( (to.x-from.x) * (to.x-from.x) + (to.y-from.y) * (to.y-from.y) );
  }

  /**
   * @brief Rasterize the line segment onto a canvas.
   *
   * Iterates over all pixels within the bounding rectangle of the line
   * (expanded by half the line width) and colors any pixel whose center
   * lies within half the width of the segment.
   *
   * @param canvas Target canvas.
   * @return Reference to @p canvas.
   */
  virtual Canvas & draw(Canvas & canvas) const override {

    if (width <= 0.0) {
      return canvas;
    }

    double half_width = width / 2.0;

    double min_x = std::min(from.x, to.x) - half_width - 1.0;
    double max_x = std::max(from.x, to.x) + half_width + 1.0;
    double min_y = std::min(from.y, to.y) - half_width - 1.0;
    double max_y = std::max(from.y, to.y) + half_width + 1.0;

    int left   = static_cast<int>(std::floor(min_x));
    int right  = static_cast<int>(std::ceil (max_x));
    int bottom = static_cast<int>(std::floor(min_y));
    int top    = static_cast<int>(std::ceil (max_y));

    for (int y = bottom; y <= top; ++y) {
      for (int x = left; x <= right; ++x) {
        double px   = static_cast<double>(x) + 0.5;
        double py   = static_cast<double>(y) + 0.5;
        double dist = pointToSegmentDistance(px, py, from.x, from.y, to.x, to.y);
        if (dist <= half_width) {
          canvas(x, y) = color;
        }
      }
    }

    return canvas;
  }

  /**
   * @brief Apply a transformation to both endpoints.
   * @param t Transformation to apply.
   * @return Reference to this line.
   */
  virtual Line & transform (const Transformation & t) override {
    from = t*from;
    to   = t*to;
    return *this;
  }

  /**
   * @brief Compute the axis-aligned bounding box of this line.
   *
   * The box is expanded by half the line width on all sides.
   *
   * @return Bounding box.
   */
  virtual BoundingBox boundingBox() const override {
    return BoundingBox(
      { std::min(from.x, to.x) - width/2.0, std::min(from.y, to.y) - width/2.0 },
      { std::max(from.x, to.x) + width/2.0, std::max(from.y, to.y) + width/2.0 }
    );
  }

  /**
   * @brief Create a heap-allocated copy of this line.
   * @return Pointer to a new Line (caller takes ownership).
   */
  virtual Line * copy() const override {
    return new Line(*this);
  }
};

/**
 * @brief Stream output operator. Prints the line as "[from-to]".
 * @param os   Output stream.
 * @param line Line to print.
 * @return Reference to @p os.
 */
inline std::ostream & operator << (std::ostream & os, const Line & line) {
  return os << "[" << line.from << "-" << line.to << "]";
}

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Line.
   *
   * Serializes as a JSON object with keys "from", "to", "color", and "width".
   */
  template <>
  struct adl_serializer<Line> {

    /** @brief Serialize Line to JSON. */
    static void to_json(json & j, const Line & l) {
      j = json::object();
      j["from" ] = l.from;
      j["to"   ] = l.to;
      j["color"] = l.color;
      j["width"] = l.width;
    }

    /** @brief Deserialize Line from JSON. */
    static void from_json(const json & j, Line & l) {
      l.from  = j.at("from" );
      l.to    = j.at("to"   );
      l.color = j.at("color");
      l.width = j.at("width");
    }
  };
}

//#endif
