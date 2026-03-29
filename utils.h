#pragma once

//#ifndef UTILS_H
//#define UTILS_H

#include <limits>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "json.hpp"

/**
 * @brief Stream manipulator that skips whitespace and PPM-style comments.
 *
 * After consuming leading whitespace, any line beginning with '#' is skipped
 * entirely; the process repeats until a non-comment, non-whitespace character
 * is pending in the stream.  Intended for use when parsing PPM headers.
 *
 * Usage:
 * @code
 *   int width;
 *   is >> skipComments >> width;
 * @endcode
 *
 * @param is Input stream to manipulate.
 * @return Reference to @p is.
 */
inline std::istream & skipComments(std::istream & is) {
  // Skip whitespace, then if '#' is peeked, ignore the rest of the line
  is >> std::ws;
  while (is.peek() == '#') {
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    is >> std::ws;
  }
  return is;
}

/**
 * @brief Read and parse a JSON file, or read from standard input.
 *
 * Pass "-" as @p filename to read from stdin.
 *
 * @param filename Path to the JSON file, or "-" for stdin.
 * @return Parsed nlohmann::json value.
 * @throw std::runtime_error if the file cannot be opened or parsing fails.
 */
inline nlohmann::json readJsonFile(const std::string & filename) {
  nlohmann::json result;
  if (filename == "-") {
    // Read from standard input.
    std::cin >> result;
    if (!std::cin) {
      throw std::runtime_error("Error while reading JSON data from STDIN");
    }
  }
  else {
    std::ifstream f(filename);
    if (!f) {
      throw std::runtime_error("Error while opening  " + filename);
    }
    f >> result;
    if (!f) {
      throw std::runtime_error("Error while reading JSON data from  " + filename);
    }
  }
  return result;
}

//#endif
