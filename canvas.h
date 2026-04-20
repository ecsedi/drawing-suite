#pragma once

//#ifndef CANVAS_H
//#define CANVAS_H

#include "rgb.h"
#include "utils.h"
#include "matrix.h"
#include "position.h"

#include <cctype>    // for std::isspace
#include <limits>
#include <string>
#include <vector>
#include <cstddef>   // for size_t
#include <fstream>
#include <iostream>
#include <iterator>  // for iterator traits
#include <stdexcept> // for out_of_range

/**
 * @brief A 2D pixel canvas backed by a Matrix of RGB colors.
 *
 * Wraps Matrix<RGB> and adds coordinate-space pixel access (x, y) as well
 * as PPM file I/O.  Pixel (0,0) is the top-left of the canvas; x grows
 * right, y grows down, matching the mathematical convention used elsewhere in
 * this project.
 *
 * Out-of-bounds writes through operator() and operator[] are silently
 * discarded (written to an offscreen pixel sink).  Use at() when you want
 * a bounds-checking throw instead.
 */
class Canvas : public Matrix<RGB> {

public:

  /** @brief Coordinate type for x and y pixel positions. */
  typedef Position::value_type   coord_type;

  /** @brief Type of a single RGB color channel value. */
  typedef value_type::value_type rgb_component;

private:

  rgb_component      rgb_max;         ///< Maximum possible value of an RGB color component.
  mutable value_type offscreen_pixel; ///< Sink for out-of-bounds writes.

  /**
   * @brief Check whether (x, y) lies within the canvas.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return true if the pixel is on-screen.
   */
  bool onscreen(coord_type x, coord_type y) const {
    return x >= 0 && x < width() && y >= 0 && y < height();
  }

  /**
   * @brief Check whether a Position lies within the canvas.
   * @param pos Position to check.
   * @return true if the pixel is on-screen.
   */
  bool onscreen(const Position & pos) const {
    return onscreen(pos.x, pos.y);
  }

public:

  Canvas() = delete;

  /**
   * @brief Construct a blank canvas of the given size.
   * @param canvas_width      Width in pixels.
   * @param canvas_height     Height in pixels.
   * @param max_rgb_component Maximum RGB channel value (default 255).
   */
  Canvas(
    size_type     canvas_width,
    size_type     canvas_height,
    rgb_component max_rgb_component = 255
  )
  : Matrix<RGB>(canvas_height, canvas_width), rgb_max(max_rgb_component) {
  }

  /**
   * @brief Construct a canvas filled with a solid background color.
   * @param canvas_width      Width in pixels.
   * @param canvas_height     Height in pixels.
   * @param color             Fill color for all pixels.
   * @param max_rgb_component Maximum RGB channel value (default 255).
   */
  Canvas(
    size_type canvas_width,
    size_type canvas_height,
    const value_type & color,
    rgb_component max_rgb_component = 255
  )
  : Matrix<RGB>(canvas_height, canvas_width, color), rgb_max(max_rgb_component) {
  }

  Canvas(const Canvas &) = delete;
  Canvas(Canvas &&) = default;
  ~Canvas() = default;
  Canvas & operator = (const Canvas &) = delete;
  Canvas & operator = (Canvas &&) = default;

  /**
   * @brief Return the canvas width in pixels.
   * @return Number of pixel columns.
   */
  coord_type width() const {
    return static_cast<coord_type>(cols());
  }

  /**
   * @brief Return the canvas height in pixels.
   * @return Number of pixel rows.
   */
  coord_type height() const {
    return static_cast<coord_type>(rows());
  }

  /**
   * @brief Return the current maximum RGB channel value.
   * @return Maximum component value (e.g. 255).
   */
  rgb_component maximum_rgb_component() const {
    return rgb_max;
  }

  /**
   * @brief Rescale all pixel values to a new maximum RGB channel value.
   *
   * All existing pixel values are multiplied by (new_max+1)/(old_max+1).
   *
   * @param max_rgb_component New maximum channel value.
   */
  void maximum_rgb_component(rgb_component max_rgb_component) {
    if (max_rgb_component != rgb_max) {
      double scale_factor = static_cast<double>(max_rgb_component+1) / static_cast<double>(rgb_max+1);
      for (auto & color : *this) {
        color *= scale_factor;
      }
      rgb_max = max_rgb_component;
    }
  }

  /**
   * @brief Non-const pixel access by (x, y) coordinates.
   *
   * Out-of-bounds coordinates write to a silent sink pixel.
   *
   * @param x X coordinate (column).
   * @param y Y coordinate (row, 0 = bottom).
   * @return Reference to the pixel.
   */
  value_type & operator() (coord_type x, coord_type y) {
    return onscreen(x, y)
    ? Matrix<RGB>::operator()(static_cast<size_type>(y), static_cast<size_type>(x))
    : offscreen_pixel;
  }

  /**
   * @brief Const pixel access by (x, y) coordinates.
   *
   * Out-of-bounds coordinates return the offscreen sink pixel.
   *
   * @param x X coordinate (column).
   * @param y Y coordinate (row, 0 = bottom).
   * @return Const reference to the pixel.
   */
  const value_type & operator() (coord_type x, coord_type y) const {
    return onscreen(x, y)
    ? Matrix<RGB>::operator()(static_cast<size_type>(y), static_cast<size_type>(x))
    : offscreen_pixel;
  }

  /**
   * @brief Non-const pixel access by Position.
   * @param pos Pixel position.
   * @return Reference to the pixel.
   */
  value_type & operator [] (const Position & pos) {
    return operator()(pos.x, pos.y);
  }

  /**
   * @brief Const pixel access by Position.
   * @param pos Pixel position.
   * @return Const reference to the pixel.
   */
  const value_type & operator [] (const Position & pos) const {
    return operator()(pos.x, pos.y);
  }

  /**
   * @brief Bounds-checked non-const pixel access.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return Reference to the pixel.
   * @throw std::out_of_range if the coordinates are outside the canvas.
   */
  value_type & at(coord_type x, coord_type y) {
    if (!onscreen(x, y)) {
      throw std::out_of_range("Coordinates out of bounds");
    }
    return operator()(x,y);
  }

  /**
   * @brief Bounds-checked const pixel access.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @return Const reference to the pixel.
   * @throw std::out_of_range if the coordinates are outside the canvas.
   */
  const value_type & at(coord_type x, coord_type y) const {
    if (!onscreen(x, y)) {
      throw std::out_of_range("Coordinates out of bounds");
    }
    return operator()(x,y);
  }

  /**
   * @brief Bounds-checked non-const pixel access by Position.
   * @param pos Pixel position.
   * @return Reference to the pixel.
   * @throw std::out_of_range if the position is outside the canvas.
   */
  value_type & at(const Position & pos) {
    if (!onscreen(pos)) {
      throw std::out_of_range("Position out of bounds");
    }
    return operator[](pos);
  }

  /**
   * @brief Bounds-checked const pixel access by Position.
   * @param pos Pixel position.
   * @return Const reference to the pixel.
   * @throw std::out_of_range if the position is outside the canvas.
   */
  const value_type & at(const Position & pos) const {
    if (!onscreen(pos)) {
      throw std::out_of_range("Position out of bounds");
    }
    return operator[](pos);
  }

  /**
   * @brief Load a PPM (P3) image from a stream.
   *
   * The stream must begin with the literal characters "P3" followed by at
   * least one whitespace character.  Comments (lines starting with '#') are
   * skipped when reading the header fields.  On success the canvas is
   * replaced with the loaded image.  On failure the stream failbit is set
   * and the canvas is left unchanged.
   *
   * @param is Input stream.
   * @return Reference to @p is.
   */
  std::istream & load(std::istream & is) {

    // Strictly check first two characters: 'P' and '3', no leading whitespace or comments

    char magic1 = is.get();
    char magic2 = is.get();

    if (!is) { return is; }

    if (magic1 != 'P' || magic2 != '3') {
      is.clear(std::ios_base::failbit);
      return is;
    }

    // Enforce at least one whitespace character after magic

    if (!std::isspace(is.peek())) {
      is.clear(std::ios_base::failbit);
      return is;
    }

    size_type w, h, m; // width, height, max RGB component
    is >> skipComments >> w >> skipComments >> h >> skipComments >> m;

    if (!is) { return is; }

    if (w <= 0 || h <= 0 || m <= 0) {
      is.clear(std::ios_base::failbit);
      return is;
    }

    Canvas newcanvas(w, h, m);

    is >> dynamic_cast<Matrix<RGB> &>(newcanvas);

    if (is) {
      *this = std::move(newcanvas);
    }

    return is;
  }

  /**
   * @brief Load a PPM (P3) image from a file.
   *
   * Pass "-" to read from standard input.
   *
   * @param filename Path to the PPM file, or "-" for stdin.
   * @throw std::runtime_error if the file cannot be opened or the data is invalid.
   */
  void load(const std::string & filename) {
    if (filename == "-") {
      if (!load(std::cin)) {
        throw std::runtime_error("Failed to read image from STDIN");
      }
    }
    else {
      std::ifstream f(filename);
      if (!f) {
        throw std::runtime_error("Failed to open " + filename + " for reading");
      }
      if (!load(f)) {
        throw std::runtime_error("Failed to read image from " + filename);
      }
    }
  }

  /**
   * @brief Write the canvas to a stream in PPM P3 format.
   * @param os Output stream.
   * @return Reference to @p os.
   */
  std::ostream & save(std::ostream & os) const {
    return os
    << "P3"                                      << "\n"
    << width() << " " << height()                << "\n"
    << static_cast<int>(rgb_max) << "\n"
    << dynamic_cast<const Matrix<RGB> &>(*this)  << "\n";
  }

  /**
   * @brief Save the canvas to a file in PPM P3 format.
   *
   * Pass "-" to write to standard output.
   *
   * @param filename Path to the output file, or "-" for stdout.
   * @throw std::runtime_error if the file cannot be opened or the write fails.
   */
  void save(const std::string & filename) const {
    if (filename == "-") {
      if (!save(std::cout)) {
        throw std::runtime_error("Failed to write image to STDOUT");
      }
    }
    else {
      std::ofstream f(filename);
      if (!f) {
        throw std::runtime_error("Failed to open " + filename + " for writing");
      }
      if (!save(f)) {
        throw std::runtime_error("Failed to write image to " + filename);
      }
    }
  }
};

/**
 * @brief Stream output operator: saves canvas as a PPM P3 image.
 * @param os     Output stream.
 * @param canvas Canvas to save.
 * @return Reference to @p os.
 */
inline std::ostream & operator << (std::ostream & os, const Canvas & canvas) {
  return canvas.save(os);
}

/**
 * @brief Stream input operator: loads a PPM P3 image into a canvas.
 * @param is     Input stream.
 * @param canvas Canvas to load into.
 * @return Reference to @p is.
 */
inline std::istream & operator >> (std::istream & is, Canvas & canvas) {
  return canvas.load(is);
}

//#endif
