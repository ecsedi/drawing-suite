/**
 * @file mandelbrot.cc
 * @brief Mandelbrot set image generator.
 *
 * Renders the Mandelbrot set by iterating z = z² + c for each point c on a
 * ComplexGrid and mapping the escape iteration count to a color palette.
 *
 * Can also be compiled as a standalone executable by defining STANDALONE.
 */

#include "rgb.h"
#include "utils.h"
#include "canvas.h"
#include "complexgrid.h"

#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "json.hpp"

/** @brief Convenience alias for the complex number type used in iteration. */
typedef ComplexGrid::complex complex;

/** @brief An ordered sequence of RGB colors used to colorize iteration counts. */
typedef std::vector<RGB>     Palette;

/**
 * @brief Parameters for Mandelbrot set rendering.
 */
struct MandelParams {

  complex       corner1;    ///< One corner of the complex plane region to render.
  complex       corner2;    ///< The opposite corner of the region.
  unsigned long resolution; ///< Approximate total pixel count of the output image.
  unsigned int  maxiter;    ///< Maximum iteration count (points that reach this are in the set).
  unsigned int  palgenseed; ///< RNG seed for palette generation (0 = random each run).
  std::string   imagefile;  ///< Output file path, or "-" for stdout.

  /**
   * @brief Default constructor covering the region [-2,-2] to [2,2].
   */
  MandelParams() :
    corner1   (-2,-2),
    corner2   ( 2, 2),
    resolution(1000000),
    maxiter   (255),
    palgenseed(0),
    imagefile ("-") {
  }

  /**
   * @brief Construct with explicit parameter values.
   * @param corner1    First corner of the complex region.
   * @param corner2    Opposite corner of the complex region.
   * @param resolution Approximate output pixel count.
   * @param maxiter    Maximum iteration depth.
   * @param palgenseed RNG seed for palette generation (0 = random).
   * @param imagefile  Output file path, or "-" for stdout.
   */
  MandelParams(
    const complex &     corner1,
    const complex &     corner2,
    unsigned long       resolution,
    unsigned int        maxiter,
    unsigned int        palgenseed,
    const std::string & imagefile
  ) :
  corner1   (corner1   ),
  corner2   (corner2   ),
  resolution(resolution),
  maxiter   (maxiter   ),
  palgenseed(palgenseed),
  imagefile (imagefile ) {
  }
};

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for std::complex<double>.
   *
   * Serializes as a two-element JSON array [real, imag].
   */
  template <>
  struct adl_serializer<std::complex<double>> {

    /** @brief Serialize complex number to JSON array [real, imag]. */
    static void to_json(json & j, const std::complex<double> & c) {
      j = json::array();
      j.push_back(c.real());
      j.push_back(c.imag());
    }

    /** @brief Deserialize complex number from JSON array [real, imag]. */
    static std::complex<double> from_json(const json & j) {
      return { j.at(0).get<double>(), j.at(1).get<double>() };
    }
  };

  /**
   * @brief JSON serializer specialization for MandelParams.
   */
  template <>
  struct adl_serializer<MandelParams> {

    /** @brief Serialize MandelParams to JSON. */
    static void to_json(json & j, const MandelParams & p) {
      j = json::object();
      j["corner1"   ] = p.corner1;
      j["corner2"   ] = p.corner2;
      j["resolution"] = p.resolution;
      j["maxiter"   ] = p.maxiter;
      j["palgenseed"] = p.palgenseed;
      j["imagefile" ] = p.imagefile;
    }

    /** @brief Deserialize MandelParams from JSON. */
    static MandelParams from_json(const json & j) {
      return {
        j.at("corner1"   ).get<std::complex<double>>(),
        j.at("corner2"   ).get<std::complex<double>>(),
        j.at("resolution").get<unsigned long       >(),
        j.at("maxiter"   ).get<unsigned int        >(),
        j.value("palgenseed", 0U              ),
        j.value("imagefile",  std::string("-"))
      };
    }
  };
}

using json = nlohmann::json;

/**
 * @brief Generate a randomized HSV-based color palette.
 *
 * Produces @p numColors colors by sweeping hue, modulating saturation and
 * value with sinusoidal functions whose frequency and phase are drawn from
 * @p seed.  Pass seed=0 to use a fresh random seed each call (the actual
 * seed used is printed to stderr).
 *
 * Some aesthetically pleasing seeds: 1753413148, 3024201808, 3396189049,
 * 4152498751, 3637081748, 324342204.
 *
 * @param numColors Number of colors to generate.
 * @param seed      RNG seed (0 = random).
 * @return A Palette of @p numColors RGB values.
 */
Palette generateRandomPalette(int numColors, unsigned int seed = 0) {

  if (!seed) {
    seed = std::random_device{}();
  }

  std::mt19937 rng(seed);

  std::cerr << "seed: " << seed << std::endl;

  std::uniform_real_distribution<double> hueDist    (0.0, 360.0     );
  std::uniform_real_distribution<double> satBiasDist(0.5, 1.0       );
  std::uniform_real_distribution<double> valBiasDist(0.4, 0.8       );
  std::uniform_real_distribution<double> freqDist   (0.5, 3.0       );
  std::uniform_real_distribution<double> phaseDist  (0.0, M_PI * 2.0);

  // Randomise the key palette parameters once per call
  const double hueStart = hueDist    (rng);
  const double hueRange = hueDist    (rng) * 0.5 + 180.0; // 180°–360° sweep
  const double satBias  = satBiasDist(rng);
  const double valBias  = valBiasDist(rng);
  const double satFreq  = freqDist   (rng);
  const double valFreq  = freqDist   (rng);
  const double satPhase = phaseDist  (rng);
  const double valPhase = phaseDist  (rng);

  Palette palette;
  palette.reserve(numColors);

  for (int i = 0; i < numColors; ++i) {

    double t          = static_cast<double>(i) / numColors;
    double hue        = std::fmod(hueStart + t * hueRange, 360.0);
    double saturation = satBias + (1.0 - satBias) * std::sin(t * M_PI * 2.0 * satFreq + satPhase);
    double value      = valBias + (1.0 - valBias) * std::pow(std::sin(t * M_PI * numColors / 8.0 * valFreq + valPhase), 2);

    // Clamp to valid range to guard against floating point drift
    saturation = std::clamp(saturation, 0.0, 1.0);
    value      = std::clamp(value,      0.0, 1.0);

    palette.push_back(RGB::from_hsv(hue, saturation, value));
  }

  return palette;
}

/**
 * @brief Generate a deterministic HSV-based color palette.
 *
 * Sweeps the full hue spectrum starting at blue (240°), with subtle
 * sinusoidal modulation of saturation and value to avoid flat-looking bands.
 *
 * @param numColors Number of colors to generate.
 * @return A Palette of @p numColors RGB values.
 */
Palette generateBasicPalette(int numColors) {

  Palette palette;
  palette.reserve(numColors);

  for (int i = 0; i < numColors; ++i) {

    double t = static_cast<double>(i) / numColors;

    // Cycle hue across the full spectrum (offset by 240° to start at blue)
    double hue        = std::fmod(240.0 + t * 360.0, 360.0);
    // Vary saturation subtly so colors don't all look flat
    double saturation = 0.7 + 0.3 * std::sin(t * M_PI * 2.0);
    // Vary brightness to create bands of light and dark
    double value      = 0.5 + 0.5 * std::pow(std::sin(t * M_PI * numColors / 8.0), 2);

    palette.push_back(RGB::from_hsv(hue, saturation, value));
  }

  return palette;
}

/**
 * @brief Compute the Mandelbrot escape iteration count for a single point.
 *
 * Iterates z = z² + c starting from z = 0.  Returns the number of
 * iterations before |z| ≥ 2 (escape), or 0 if @p maxiter is reached
 * without escaping (i.e. the point is in the set).
 *
 * @param c       Complex parameter (the point being tested).
 * @param maxiter Maximum number of iterations.
 * @return Escape iteration count, or 0 if the point is in the set.
 */
unsigned int mandel(const complex & c, unsigned int maxiter) {

  complex      z = 0;
  unsigned int i = 0;

  while (i < maxiter+1 && std::abs(z) < 2) {
    ++i;
    z = z*z + c;
  }

  return i > maxiter ? 0 : i;
}

/**
 * @brief Generate a Mandelbrot set image from JSON parameters and save it.
 *
 * Builds a ComplexGrid over the specified region, computes the escape count
 * for each grid point, maps it to a palette color, and saves the canvas.
 *
 * @param mpjson JSON object containing MandelParams fields.
 */
void genMandel(const json & mpjson) {

  MandelParams mp = mpjson.get<MandelParams>();
  ComplexGrid  grid(mp.corner1, mp.corner2, mp.resolution);
  Canvas       canvas(grid.width(), grid.height()); // max color component defaults to 255
  Palette      palette = generateRandomPalette(mp.maxiter+1, mp.palgenseed);

  ComplexGrid::size_type n = 0;

  for (auto & pixel : canvas) {
    auto iter = mandel(grid[n], mp.maxiter);
    pixel     = palette[iter];
    ++n;
  }

  canvas.save(mp.imagefile);

  if (mp.imagefile != "-") {
    std::cerr << "Saved image to " << mp.imagefile << std::endl;
  }
}

#ifdef STANDALONE

/**
 * @brief Standalone entry point (only compiled when STANDALONE is defined).
 *
 * Usage: mandelbrot <JSON_FILE>
 *
 * @param argc Argument count (must be 2).
 * @param argv argv[1] is the JSON parameter file path.
 * @return 0 on success, 1 on argument error.
 */
int main(int argc, char ** argv) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " JSON_FILE" << std::endl;
    exit(1);
  }

  genMandel(readJsonFile(argv[1]));
  return 0;
}

#endif
