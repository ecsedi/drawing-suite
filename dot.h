#pragma once

//#ifndef DOT_H
//#define DOT_H

#include "line.h"
#include "position.h"
#include "drawable.h"
#include "boundingbox.h"
#include "transformation.h"

#include "json.hpp"

/**
 * @brief A filled circular dot drawable.
 *
 * Renders as a zero-length Line segment with the given diameter, which
 * produces a disk of the specified size centered at @c pos.
 */
class Dot : public Drawable {

public:

  /** @brief Color type (alias for the canvas pixel type, RGB). */
  typedef Canvas::value_type color_type;

  Position   pos;    ///< Center position of the dot.
  color_type color;  ///< Dot color.
  double     size;   ///< Dot diameter in canvas units.

  /** @brief Default constructor. */
  Dot() = default;

  /**
   * @brief Construct a dot at a given position.
   * @param position  Center position.
   * @param dot_color Dot color (default black).
   * @param diameter  Dot diameter in canvas units (default 1).
   */
  Dot(
    const Position   & position,
    const color_type & dot_color = color_type(0,0,0),
    double             diameter  = 1
  )
  : pos(position), color(dot_color), size(diameter) {
  }

  virtual ~Dot() override = default;

  /**
   * @brief Draw the dot as a degenerate (zero-length) line segment.
   * @param canvas Target canvas.
   * @return Reference to @p canvas.
   */
  virtual Canvas & draw(Canvas& canvas) const override {
    return Line(pos, pos, color, size).draw(canvas);
  }

  /**
   * @brief Apply a transformation to the dot's center position.
   * @param t Transformation to apply.
   * @return Reference to this dot.
   */
  virtual Dot & transform(const Transformation & t) override {
    pos = t*pos;
    return *this;
  }

  /**
   * @brief Compute the bounding box of this dot.
   *
   * Returns a box of side @c size centered on @c pos.
   *
   * @return Bounding box.
   */
  virtual BoundingBox boundingBox() const override {
    return BoundingBox(
      { pos.x - size/2.0, pos.y - size/2.0 },
      { pos.x + size/2.0, pos.y + size/2.0 }
    );
  }

  /**
   * @brief Create a heap-allocated copy of this dot.
   * @return Pointer to a new Dot (caller takes ownership).
   */
  virtual Dot * copy() const override {
    return new Dot(*this);
  }
};

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Dot.
   *
   * Serializes as a JSON object with keys "pos", "color", and "size".
   */
  template <>
  struct adl_serializer<Dot> {

    /** @brief Serialize Dot to JSON. */
    static void to_json(json & j, const Dot & d) {
      j = json::object();
      j["pos"  ] = d.pos;
      j["color"] = d.color;
      j["size" ] = d.size;
    }

    /** @brief Deserialize Dot from JSON. */
    static void from_json(const json & j, Dot & d) {
      d.pos   = j["pos"  ];
      d.color = j["color"];
      d.size  = j["size" ];
    }
  };
}

//#endif
