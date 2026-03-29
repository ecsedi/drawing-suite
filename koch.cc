/**
 * @file koch.cc
 * @brief Koch snowflake fractal generator.
 *
 * Implements the Koch class and the genKoch() entry point.
 * The Koch snowflake is generated recursively by replacing each line segment
 * with four sub-segments: the middle third is replaced by two sides of an
 * equilateral triangle.
 *
 * Can also be compiled as a standalone executable by defining STANDALONE.
 */

#include "rgb.h"
#include "dot.h"
#include "line.h"
#include "utils.h"
#include "canvas.h"
#include "drawablelist.h"
#include "transformation.h"

#include <cmath>
#include <string>
#include <cstdlib>
#include <fstream>
#include <numbers> // C++ 20 feature, needed for pi
#include <iostream>
#include <algorithm>

#include "json.hpp"

/** @brief Degrees-to-radians conversion factor. */
constexpr double DEG = std::numbers::pi / 180.0;

/**
 * @brief Parameters for Koch snowflake generation.
 */
struct KochParams {

  RGB           bg_color;        ///< Canvas background color.
  RGB           line_color;      ///< Snowflake line color.
  double        line_width;      ///< Line width in canvas units.
  int           iteration_depth; ///< Recursion depth (higher = more detail).
  unsigned long resolution;      ///< Approximate total pixel count of the output image.
  std::string   imagefile;       ///< Output file path, or "-" for stdout.

  /**
   * @brief Default constructor with sensible defaults.
   */
  KochParams() :
  bg_color       ("white"),
  line_color     ("black"),
  line_width     (1.0),
  iteration_depth(8),
  resolution     (1000000),
  imagefile      ("-") {
  }

  /**
   * @brief Construct with explicit parameter values.
   * @param bg_color        Background color.
   * @param line_color      Line color.
   * @param line_width      Line width.
   * @param iteration_depth Recursion depth.
   * @param resolution      Approximate output pixel count.
   * @param imagefile       Output file path, or "-" for stdout.
   */
  KochParams(
    RGB                 bg_color,
    RGB                 line_color,
    double              line_width,
    int                 iteration_depth,
    unsigned long       resolution = 1000000,
    const std::string & imagefile  = "-"
  ) :
  bg_color       (bg_color       ),
  line_color     (line_color     ),
  line_width     (line_width     ),
  iteration_depth(iteration_depth),
  resolution     (resolution     ),
  imagefile      (imagefile      ) {
  }
};

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for KochParams.
   */
  template <>
  struct adl_serializer<KochParams> {

    /** @brief Serialize KochParams to JSON. */
    static void to_json(json & j, const KochParams & k) {
      j = json::object();
      j["bg_color"       ] = k.bg_color;
      j["line_color"     ] = k.line_color;
      j["line_width"     ] = k.line_width;
      j["iteration_depth"] = k.iteration_depth;
      j["resolution"     ] = k.resolution;
      j["imagefile"      ] = k.imagefile;
    }

    /** @brief Deserialize KochParams from JSON. */
    static KochParams from_json(const json & j) {
      return {
        j.at("bg_color"       ).get<RGB          >(),
        j.at("line_color"     ).get<RGB          >(),
        j.at("line_width"     ).get<double       >(),
        j.at("iteration_depth").get<int          >(),
        j.at("resolution"     ).get<unsigned long>(),
        j.value("imagefile",  std::string("-"))
      };
    }
  };
}

using json = nlohmann::json;

/**
 * @brief Drawable that renders a Koch snowflake curve.
 *
 * Recursively subdivides a base line segment into the Koch fractal pattern.
 * At each recursion level, the middle third of every segment is replaced by
 * two sides of an equilateral triangle, increasing the total segment count
 * by a factor of 4 per level.
 */
class Koch : public Drawable {

private:

  DrawableList koch; ///< Flat list of all leaf line segments.

  /**
   * @brief Recursively grow the Koch curve from a single line segment.
   *
   * At depth 0 the segment is added to the list as a leaf.
   * At higher depths it is split into four sub-segments and each is grown
   * recursively.
   *
   * @param line  The segment to subdivide.
   * @param depth Remaining recursion depth.
   */
  void grow(const Line & line, int depth) {

    if (depth < 1) {
      koch.push_back(line);
      return;
    }

    auto o1 = Transformation::translate(-line.from);
    auto o2 = Transformation::translate( line.from);

    auto s  = Transformation::scale(1.0/3.0);

    auto l1 = s*o1*line;

    auto r2 = Transformation::rotate( 60*DEG);
    auto t2 = Transformation::translate(l1.to);
    auto l2 = t2*r2*l1;

    auto r3 = Transformation::rotate(-60*DEG);
    auto t3 = Transformation::translate(l2.to);
    auto l3 = t3*r3*l1;

    auto t4 = Transformation::translate(l3.to);
    auto l4 = t4*l1;

    l1 = o2*l1;
    l2 = o2*l2;
    l3 = o2*l3;
    l4 = o2*l4;

    grow(l1, depth-1);
    grow(l2, depth-1);
    grow(l3, depth-1);
    grow(l4, depth-1);
  }

public:

  RGB    color; ///< Line color.
  double width; ///< Line width.
  int    depth; ///< Recursion depth used to build this curve.

  Koch() = delete;

  /**
   * @brief Construct and build a Koch curve.
   *
   * The base segment runs from (0,0) to (100*width, 0).
   *
   * @param color Line color.
   * @param width Line width (default 1.0).
   * @param depth Recursion depth (default 0 = single segment).
   */
  explicit Koch(const RGB & color, double width = 1.0, int depth = 0)
  : color(color), width(width), depth(depth) {
    grow(Line({0,0},{100.0*width,0}, color, width), depth);
  }

  Koch(const Koch &) = default;
  Koch(Koch &&) = default;

  virtual ~Koch() override {
  }

  Koch & operator = (const Koch &) = default;
  Koch & operator = (Koch &&) = default;

  /** @copydoc Drawable::draw */
  virtual Canvas & draw(Canvas & canvas) const override {
    return koch.draw(canvas);
  }

  /** @copydoc Drawable::transform */
  virtual Koch & transform(const Transformation & t) override {
    koch.transform(t);
    return *this;
  }

  /** @copydoc Drawable::boundingBox */
  virtual BoundingBox boundingBox() const override {
    return koch.boundingBox();
  }

  /** @copydoc Drawable::copy */
  virtual Koch * copy() const override {
    return new Koch(*this);
  }
};

/**
 * @brief Generate a Koch snowflake image from JSON parameters and save it.
 *
 * Parses the parameters, builds the Koch fractal, fits it to the requested
 * resolution, renders onto a canvas, and saves the result.
 *
 * @param kpjson JSON object containing KochParams fields.
 */
void genKoch(const json & kpjson) {

  KochParams kp = kpjson.get<KochParams>();
  Koch koch(kp.line_color, kp.line_width, kp.iteration_depth);
  
  auto bbox        = koch.boundingBox() * 1.05;
  auto screen_size = bbox.screenSize(kp.resolution);

  Canvas pic(screen_size.width, screen_size.height, kp.bg_color);

  auto ts = Transformation::toScreen(bbox, pic.width(), pic.height());

  pic << ts * koch;
  pic.save(kp.imagefile);

  if (kp.imagefile != "-") {
    std::cout << "Saved image to " << kp.imagefile << std::endl;
  }
}

#ifdef STANDALONE

/**
 * @brief Standalone entry point (only compiled when STANDALONE is defined).
 *
 * Usage: koch <JSON_FILE>
 *
 * @param argc Argument count (must be 2).
 * @param argv argv[1] is the JSON parameter file path.
 * @return 0 on success, 1 on argument error.
 */
int main(int argc, char** argv) {

  if (argc != 2 && argc != 4) {
    std::cerr << "Usage: " << argv[0] << " JSON_FILE" << std::endl;
    exit(1);
  }

  genKoch(readJsonFile(argv[1]));
  return 0;
}

#endif
