/**
 * @file sierp.cc
 * @brief Sierpinski triangle fractal generator.
 *
 * Implements the Sierpinski class and the genSierpinski() entry point.
 * The Sierpinski triangle is generated recursively: each triangle is
 * subdivided into three half-size copies placed at its three corners,
 * leaving the central triangle empty.
 *
 * Can also be compiled as a standalone executable by defining STANDALONE.
 */

#include "rgb.h"
#include "dot.h"
#include "line.h"
#include "utils.h"
#include "canvas.h"
#include "polygon.h"
#include "drawablelist.h"
#include "transformation.h"

#include <cmath>
#include <string>
#include <cstdlib>
#include <numbers> // C++ 20 feature, needed for pi
#include <iostream>
#include <algorithm>

#include "json.hpp"

/** @brief Degrees-to-radians conversion factor. */
constexpr double DEG = std::numbers::pi / 180.0;

/**
 * @brief Parameters for Sierpinski triangle generation.
 */
struct SierParams {

  RGB           bg_color;        ///< Canvas background color.
  RGB           line_color;      ///< Triangle edge color.
  double        line_width;      ///< Edge width in canvas units.
  int           iteration_depth; ///< Recursion depth (higher = more detail).
  unsigned long resolution;      ///< Approximate total pixel count of the output image.
  std::string   imagefile;       ///< Output file path, or "-" for stdout.

  /**
   * @brief Default constructor with sensible defaults.
   */
  SierParams() :
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
   * @param line_color      Edge color.
   * @param line_width      Edge width.
   * @param iteration_depth Recursion depth.
   * @param resolution      Approximate output pixel count.
   * @param imagefile       Output file path, or "-" for stdout.
   */
  SierParams(
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
   * @brief JSON serializer specialization for SierParams.
   */
  template <>
  struct adl_serializer<SierParams> {

    /** @brief Serialize SierParams to JSON. */
    static void to_json(json & j, const SierParams & s) {
      j = json::object();
      j["bg_color"       ] = s.bg_color;
      j["line_color"     ] = s.line_color;
      j["line_width"     ] = s.line_width;
      j["iteration_depth"] = s.iteration_depth;
      j["resolution"     ] = s.resolution;
      j["imagefile"      ] = s.imagefile;
    }

    /** @brief Deserialize SierParams from JSON. */
    static SierParams from_json(const json & j) {
      return {
        j.at("bg_color"       ).get<RGB          >(),
        j.at("line_color"     ).get<RGB          >(),
        j.at("line_width"     ).get<double       >(),
        j.at("iteration_depth").get<int          >(),
        j.at("resolution"     ).get<unsigned long>(),
        j.value("imagefile", std::string("-"))
      };
    }
  };
}

using json = nlohmann::json;

/**
 * @brief Drawable that renders a Sierpinski triangle.
 *
 * Recursively subdivides an equilateral triangle into three half-size copies
 * anchored at each original vertex, omitting the central triangle.
 * At depth 0 the original triangle polygon is stored as a leaf.
 */
class Sierpinski : public Drawable {

private:

  DrawableList sierp; ///< Flat list of all leaf triangle polygons.

  /**
   * @brief Recursively subdivide a triangle into the Sierpinski pattern.
   *
   * At depth 0 the triangle is added to the list as a leaf.
   * At higher depths three half-scale copies are placed at the three corners
   * and each is grown recursively.
   *
   * @param triangle The triangle polygon to subdivide.
   * @param depth    Remaining recursion depth.
   */
  void grow(const Polygon & triangle, int depth) {

    if (depth < 1) {
      sierp.push_back(triangle);
      return;
    }

    Transformation o1  = Transformation::translate(-triangle[0]);
    Transformation o2  = Transformation::translate( triangle[0]);

    Transformation s   = Transformation::scale(1.0/2.0);

    Transformation t2  = Transformation::translate((triangle[1]-triangle[0])/2);
    Transformation t3  = Transformation::translate((triangle[2]-triangle[0])/2);

    Transformation ts1 = o2*s*o1;
    Transformation ts2 = o2*t2*s*o1;
    Transformation ts3 = o2*t3*s*o1;

    grow(ts1*triangle, depth-1);
    grow(ts2*triangle, depth-1);
    grow(ts3*triangle, depth-1);
  }

public:

  RGB    color; ///< Edge color.
  size_t width; ///< Edge width.
  int    depth; ///< Recursion depth used to build this fractal.

  Sierpinski() = delete;

  /**
   * @brief Construct and build a Sierpinski triangle.
   *
   * The base triangle is equilateral with side length 100*width,
   * placed at the origin with one vertex at (0,0).
   *
   * @param color Edge color.
   * @param width Edge width (default 1).
   * @param depth Recursion depth (default 0 = single triangle).
   */
  Sierpinski(const RGB & color, size_t width = 1, int depth = 0)
  : color(color), width(width), depth(depth) {
    Polygon triangle(
      {
        {0,                       0                      },
        {100.0*width,             0                      },
        {100.0*width*cos(60*DEG), 100.0*width*sin(60*DEG)},
        {0,                       0                      }
      },
      color,
      width
    );
    grow(triangle, depth);
  }

  Sierpinski(const Sierpinski &) = default;
  Sierpinski(Sierpinski &&) = default;

  virtual ~Sierpinski() override {
  }

  Sierpinski & operator = (const Sierpinski &) = default;
  Sierpinski & operator = (Sierpinski &&) = default;

  /** @copydoc Drawable::draw */
  virtual Canvas & draw(Canvas & canvas) const override {
    return sierp.draw(canvas);
  }

  /** @copydoc Drawable::transform */
  virtual Sierpinski & transform(const Transformation & t) override {
    sierp.transform(t);
    return *this;
  }

  /** @copydoc Drawable::boundingBox */
  virtual BoundingBox boundingBox() const override {
    return sierp.boundingBox();
  }

  /** @copydoc Drawable::copy */
  virtual Sierpinski * copy() const override {
    return new Sierpinski(*this);
  }
};

/**
 * @brief Generate a Sierpinski triangle image from JSON parameters and save it.
 *
 * Parses the parameters, builds the fractal, fits it to the requested
 * resolution, renders onto a canvas, and saves the result.
 *
 * @param spjson JSON object containing SierParams fields.
 */
void genSierpinski(const json & spjson) {

  SierParams sp = spjson.get<SierParams>();
  Sierpinski sierp(sp.line_color, sp.line_width, sp.iteration_depth);
  
  auto bbox        = sierp.boundingBox() * 1.05;
  auto screen_size = bbox.screenSize(sp.resolution);

  Canvas pic(screen_size.width, screen_size.height, sp.bg_color);

  auto ts = Transformation::toScreen(bbox, pic.width(), pic.height());

  pic << ts * sierp;
  pic.save(sp.imagefile);

  if (sp.imagefile != "-") {
    std::cerr << "Saved image to " << sp.imagefile << std::endl;
  }
}

#ifdef STANDALONE

/**
 * @brief Standalone entry point (only compiled when STANDALONE is defined).
 *
 * Usage: sierp <JSON_FILE>
 *
 * @param argc Argument count (must be 2).
 * @param argv argv[1] is the JSON parameter file path.
 * @return 0 on success, 1 on argument error.
 */
int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " JSON_FILE" << std::endl;
    exit(1);
  }

  genSierpinski(readJsonFile(argv[1]));
  return 0;
}

#endif
