/**
 * @file draw.cc
 * @brief Main entry point for the drawing suite.
 *
 * Reads a JSON parameter file whose top-level "type" field selects which
 * fractal generator to invoke (koch, mandelbrot, sierpinski, or tree),
 * then dispatches to the appropriate generator function.
 *
 * Usage:
 * @code
 *   draw <JSON_FILE>
 * @endcode
 */

#include <map>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "draw.h"
#include "utils.h"

#include "json.hpp"

/**
 * @brief Enumeration of supported drawing types.
 */
enum class DrawingType {
  koch,        ///< Koch snowflake fractal.
  mandelbrot,  ///< Mandelbrot set.
  sierpinski,  ///< Sierpinski triangle fractal.
  tree,        ///< Fractal tree.
  UNKNOWN      ///< Unrecognized type string.
};

/**
 * @brief Convert a type string to the corresponding DrawingType enum value.
 *
 * @param t Type string (e.g. "koch", "mandelbrot").
 * @return The matching DrawingType, or DrawingType::UNKNOWN if not recognized.
 */
DrawingType getType(const std::string & t) {

  static const std::unordered_map<std::string, DrawingType> typeMap = {
    { "koch",       DrawingType::koch       },
    { "mandelbrot", DrawingType::mandelbrot },
    { "sierpinski", DrawingType::sierpinski },
    { "tree",       DrawingType::tree       }
  };

  auto it = typeMap.find(t);
  return (it == typeMap.end()) ? DrawingType::UNKNOWN : it->second;
}

/**
 * @brief Program entry point.
 *
 * Expects exactly one command-line argument: the path to a JSON parameter
 * file (or "-" to read from stdin).  Reads the file, determines the drawing
 * type from the "type" field, and dispatches to the appropriate generator.
 *
 * @param argc Argument count (must be 2).
 * @param argv Argument vector; argv[1] is the JSON file path.
 * @return 0 on success, 1 on bad arguments, 2 on unknown drawing type.
 */
int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " JSON_FILE" << std::endl;
    exit(1);
  }

  auto params = readJsonFile(argv[1]);
  auto type   = params["type"].get<std::string>();

  switch (getType(type)) {
    case DrawingType::koch:       genKoch      (params); break;
    case DrawingType::mandelbrot: genMandel    (params); break;
    case DrawingType::sierpinski: genSierpinski(params); break;
    case DrawingType::tree:       genTree      (params); break;
    default: std::cerr << "Unknown type: " << type << std::endl; exit(2);
  }

  return 0;
}
