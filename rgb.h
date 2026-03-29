#pragma once

//#ifndef RGB_H
//#define RGB_H

#include <map>
#include <cctype>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "json.hpp"

/**
 * @brief Represents a color in RGB color space.
 *
 * Stores red, green, and blue components as 16-bit unsigned integers.
 * Be careful though, because predefined colors use only 8 bits (0-255)!
 * The extra bits are needed to support PPM format input/output.
 * Supports construction from numeric values, hexadecimal strings,
 * and predefined named colors.
 */
class RGB {

public:

  /** @brief Underlying channel type (unsigned 16-bit integer). */
  typedef unsigned short value_type;

  /**
   * @brief Map of named colors to hexadecimal string representations.
   *
   * Key: color name  
   * Value: corresponding hex string (RRGGBB format)
   */
  typedef std::map<const std::string, const std::string> ColorMap;

  /** @brief Red channel value (0–255). */
  value_type r;

  /** @brief Green channel value (0–255). */
  value_type g;

  /** @brief Blue channel value (0–255). */
  value_type b;

  /** @brief Predefined black color (0,0,0). */
  static const RGB black;

  /** @brief Predefined white color (255,255,255). */
  static const RGB white;

  /** @brief Predefined red color (255,0,0). */
  static const RGB red;

  /** @brief Predefined green color (0,255,0). */
  static const RGB green;

  /** @brief Predefined blue color (0,0,255). */
  static const RGB blue;

  /** @brief Predefined yellow color (255,255,0). */
  static const RGB yellow;

  /** @brief Predefined cyan color (0,255,255). */
  static const RGB cyan;

  /** @brief Predefined magenta color (255,0,255). */
  static const RGB magenta;

  /** @brief Static map containing supported named colors. */
  static ColorMap colorMap;

  /**
   * @brief Default constructor.
   *
   * Initializes the color to black (0,0,0).
   */
  constexpr RGB() : r(0), g(0), b(0) {}

  /**
   * @brief Construct from explicit RGB channel values.
   *
   * @param red   Red component (0–255)
   * @param green Green component (0–255)
   * @param blue  Blue component (0–255)
   */
  constexpr RGB(
    const value_type & red,
    const value_type & green,
    const value_type & blue
  )
  : r(red), g(green), b(blue) {}

  /**
   * @brief Construct from hexadecimal string or named color.
   *
   * @param hex Hex string in format RRGGBB or #RRGGBB,
   *            or a valid named color.
   * @throw std::invalid_argument if format is invalid.
   */
  RGB(const std::string & hex) {
    set(hex);
  }

  /**
   * @brief Construct from C-string hexadecimal or named color.
   *
   * @param hex Null-terminated string in RRGGBB or #RRGGBB format,
   *            or a valid named color.
   * @throw std::invalid_argument if format is invalid.
   */
  RGB(const char* hex) {
    set(hex);
  }

  /**
   * @brief Set RGB channel values explicitly.
   *
   * @param red   Red component (0–255)
   * @param green Green component (0–255)
   * @param blue  Blue component (0–255)
   * @return Reference to this object.
   */
  constexpr RGB & set(
    const value_type & red,
    const value_type & green,
    const value_type & blue
  ) {
    r = red;
    g = green;
    b = blue;
    return *this;
  }

  /**
   * @brief Set color from hexadecimal string or named color.
   *
   * Accepts:
   * - RRGGBB
   * - #RRGGBB
   * - Named color (if present in colorMap)
   *
   * @param hex Input string. Either a color name or a hex representation.
   * @return Reference to this object.
   * @throw std::invalid_argument if format is invalid.
   */
  RGB & set(std::string hex) {

    if (colorMap.count(hex)) {
      hex = colorMap.at(hex);
    }

    if (!hex.empty() && hex[0] == '#') {
      hex = hex.substr(1);
    }

    if (hex.length() != 6) {
      throw std::invalid_argument("Hex color must be 6 digits (RRGGBB or #RRGGBB)");
    }

    for (auto & c : hex) {
      if (!std::isxdigit(c)) {
        throw std::invalid_argument("Invalid hex character in color string");
      }
      c = std::toupper(c);
    }

    unsigned int value;
    std::stringstream ss;

    ss << hex;
    ss >> std::hex >> value;

    if (!ss) {
      throw std::invalid_argument("Failed to parse hex color value");
    }

    r = static_cast<value_type>((value >> 16) & 0xFF);
    g = static_cast<value_type>((value >>  8) & 0xFF);
    b = static_cast<value_type>( value        & 0xFF);

    return *this;
  }

  // Factory method.
  // Converts HSV (0-360, 0-1, 0-1) to RGB (0-maxcomp each)
  /**
   * @brief Factory method to convert HSV values to an RGB color.
   *
   * @param h       Hue (0–360°).
   * @param s       Saturation (0-1).
   * @param v       Value (0-1).
   * @param maxcomp Optional maximum color component value (default 255).
   * @return The created RGB color.
   */
  static RGB from_hsv(double h, double s, double v, value_type maxcomp = 255) {

    double r, g, b;
    int    i = static_cast<int>(h / 60.0) % 6;
    double f = (h / 60.0) - std::floor(h / 60.0);
    double p = v * (1.0 -             s);
    double q = v * (1.0 -        f  * s);
    double t = v * (1.0 - (1.0 - f) * s);

    switch (i) {
      case 0:  r = v; g = t; b = p; break;
      case 1:  r = q; g = v; b = p; break;
      case 2:  r = p; g = v; b = t; break;
      case 3:  r = p; g = q; b = v; break;
      case 4:  r = t; g = p; b = v; break;
      case 5:  r = v; g = p; b = q; break;
      default: r = 0; g = 0; b = 0; break;
    }

    return {
      static_cast<value_type>(r * maxcomp),
      static_cast<value_type>(g * maxcomp),
      static_cast<value_type>(b * maxcomp)
    };
  }

  /**
   * @brief Convert to hexadecimal string representation.
   *
   * @param maxcomp Optional maximum possible color component value (default: 255).
   * @param prefix  Optional prefix (default "#").
   * @return Uppercase hexadecimal string.
   */
  std::string toHex(value_type maxcomp = 255, const std::string & prefix = "#") const {

    std::ostringstream oss;

    maxcomp = std::max({r, g, b, maxcomp}); // In case one of them is larger.
    oss << std::hex << std::uppercase << maxcomp;
    int w = oss.str().size();
    if (w < 2) { w = 2; }
    oss.str(""); // Reset buffer.

    oss
    << prefix
    << std::setfill('0') << std::setw(w) << r
    << std::setfill('0') << std::setw(w) << g
    << std::setfill('0') << std::setw(w) << b;

    return oss.str();
  }

  /**
   * @brief Convert to hexadecimal string without prefix.
   *
   * @param maxcomp Optional maximum possible color component value (default: 255).
   * @return Uppercase hexadecimal string in RRGGBB format.
   */
  std::string toHexNoPrefix(value_type maxcomp = 255) const {
    return toHex(maxcomp, "");
  }

  /**
   * @brief Print color to output stream.
   *
   * If the stream is in hexadecimal mode, prints uppercase hex values.
   * Otherwise prints decimal values aligned for readability.
   *
   * @param os Output stream.
   * @return Reference to output stream.
   */
  std::ostream & print(std::ostream & os) const {
    return
    os.flags() & std::ios_base::hex
    ?
    os
    << std::uppercase
    << std::setw(2) << std::setfill('0') << r << " "
    << std::setw(2) << std::setfill('0') << g << " "
    << std::setw(2) << std::setfill('0') << b
    :
    os
    << std::setw(3) << std::setfill(' ') << r << " "
    << std::setw(3) << std::setfill(' ') << g << " "
    << std::setw(3) << std::setfill(' ') << b;
  }

  /**
   * @brief Multiply color intensity by a factor.
   *
   * @param factor Scaling factor.
   * @return Reference to this object.
   */
  RGB & operator *= (double factor) {
    r = static_cast<value_type>(r*factor);
    g = static_cast<value_type>(g*factor);
    b = static_cast<value_type>(b*factor);
    return *this;
  }

  /**
   * @brief Divide color intensity by a factor.
   *
   * @param factor Divisor.
   * @return Reference to this object.
   */
  RGB & operator /= (double factor) {
    r = static_cast<value_type>(r/factor);
    g = static_cast<value_type>(g/factor);
    b = static_cast<value_type>(b/factor);
    return *this;
  }

  /**
   * @brief Fade current color toward another color.
   *
   * @param target_color Target color.
   * @param speed        Fade speed (0-1).
   * @return Reference to this object.
   */
  RGB & fade(const RGB & target_color, double speed) {

    if (speed < 0 || speed > 1) {
      throw std::invalid_argument("Invalid fade speed value");
    }

    r = static_cast<value_type>(r + speed * (target_color.r - r));
    g = static_cast<value_type>(g + speed * (target_color.g - g));
    b = static_cast<value_type>(b + speed * (target_color.b - b));

    return *this;
  }

};

/**
 * @brief Stream insertion operator.
 */
inline std::ostream & operator << (std::ostream & os, const RGB & color) {
  return color.print(os);
}

/**
 * @brief Stream extraction operator.
 */
inline std::istream & operator >> (std::istream & is, RGB & color) {
  int r, g, b;
  is >> r >> g >> b;
  if (is) {
    color = RGB(r, g, b);
  }
  return is;
}

/** @brief Equality comparison operator. */
inline bool operator == (const RGB & left, const RGB & right) {
  return left.r == right.r && left.g == right.g && left.b == right.b;
}

/** @brief Inequality comparison operator. */
inline bool operator != (const RGB & left, const RGB & right) {
  return !(left == right);
}

/** @brief Multiply color by scalar. */
inline RGB operator * (const RGB & color, double factor) {
  return RGB(color)*=factor;
}

/** @brief Multiply scalar by color. */
inline RGB operator * (double factor, const RGB & color) {
  return RGB(color)*=factor;
}

/** @brief Divide color by scalar. */
inline RGB operator / (const RGB & color, double factor) {
  return RGB(color)/=factor;
}

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for RGB.
   *
   * Serializes as the hexadecimal representation of an RGB color.
   */
  template <>
  struct adl_serializer<RGB> {

    /** @brief Serialize RGB to JSON string "#RRGGBB". */
    static void to_json(json & j, const RGB & color) {
      j = json(color.toHex());
    }

    /** @brief Deserialize RGB from JSON string. */
    static void from_json(const json & j, RGB & color) {
      color.set(j);
    }
  };
}

constexpr RGB RGB::black  (  0,   0,   0);
constexpr RGB RGB::white  (255, 255, 255);
constexpr RGB RGB::red    (255,   0,   0);
constexpr RGB RGB::green  (  0, 255,   0);
constexpr RGB RGB::blue   (  0,   0, 255);
constexpr RGB RGB::yellow (255, 255,   0);
constexpr RGB RGB::cyan   (  0, 255, 255);
constexpr RGB RGB::magenta(255,   0, 255);

inline RGB::ColorMap RGB::colorMap = {

  // Red

  {"indianred",   "#CD5C5C"},
  {"lightcoral",  "#F08080"},
  {"salmon",      "#FA8072"},
  {"darksalmon",  "#E9967A"},
  {"lightsalmon", "#FFA07A"},
  {"crimson",     "#DC143C"},
  {"red",         "#FF0000"},
  {"firebrick",   "#B22222"},
  {"darkred",     "#8B0000"},

  // Pink
  {"pink",            "#FFC0CB"},
  {"lightpink",       "#FFB6C1"},
  {"hotpink",         "#FF69B4"},
  {"deeppink",        "#FF1493"},
  {"mediumvioletred", "#C71585"},
  {"palevioletred",   "#DB7093"},

  // Orange
  {"lightsalmon", "#FFA07A"},
  {"coral",       "#FF7F50"},
  {"tomato",      "#FF6347"},
  {"orangered",   "#FF4500"},
  {"darkorange",  "#FF8C00"},
  {"orange",      "#FFA500"},

  // Yellow
  {"gold",                 "#FFD700"},
  {"yellow",               "#FFFF00"},
  {"lightyellow",          "#FFFFE0"},
  {"lemonchiffon",         "#FFFACD"},
  {"lightgoldenrodyellow", "#FAFAD2"},
  {"papayawhip",           "#FFEFD5"},
  {"moccasin",             "#FFE4B5"},
  {"peachpuff",            "#FFDAB9"},
  {"palegoldenrod",        "#EEE8AA"},
  {"khaki",                "#F0E68C"},
  {"darkkhaki",            "#BDB76B"},

  // Purple
  {"lavender",        "#E6E6FA"},
  {"thistle",         "#D8BFD8"},
  {"plum",            "#DDA0DD"},
  {"violet",          "#EE82EE"},
  {"orchid",          "#DA70D6"},
  {"fuchsia",         "#FF00FF"},
  {"magenta",         "#FF00FF"},
  {"mediumorchid",    "#BA55D3"},
  {"mediumpurple",    "#9370DB"},
  {"rebeccapurple",   "#663399"},
  {"blueviolet",      "#8A2BE2"},
  {"darkviolet",      "#9400D3"},
  {"darkorchid",      "#9932CC"},
  {"darkmagenta",     "#8B008B"},
  {"purple",          "#800080"},
  {"indigo",          "#4B0082"},
  {"slateblue",       "#6A5ACD"},
  {"darkslateblue",   "#483D8B"},
  {"mediumslateblue", "#7B68EE"},

  // Green
  {"greenyellow",       "#ADFF2F"},
  {"chartreuse",        "#7FFF00"},
  {"lawngreen",         "#7CFC00"},
  {"lime",              "#00FF00"},
  {"limegreen",         "#32CD32"},
  {"palegreen",         "#98FB98"},
  {"lightgreen",        "#90EE90"},
  {"mediumspringgreen", "#00FA9A"},
  {"springgreen",       "#00FF7F"},
  {"mediumseagreen",    "#3CB371"},
  {"seagreen",          "#2E8B57"},
  {"forestgreen",       "#228B22"},
  {"green",             "#008000"},
  {"darkgreen",         "#006400"},
  {"yellowgreen",       "#9ACD32"},
  {"olivedrab",         "#6B8E23"},
  {"olive",             "#808000"},
  {"darkolivegreen",    "#556B2F"},
  {"mediumaquamarine",  "#66CDAA"},
  {"darkseagreen",      "#8FBC8B"},
  {"lightseagreen",     "#20B2AA"},
  {"darkcyan",          "#008B8B"},
  {"teal",              "#008080"},

  // Blue
  {"aqua",            "#00FFFF"},
  {"cyan",            "#00FFFF"},
  {"lightcyan",       "#E0FFFF"},
  {"paleturquoise",   "#AFEEEE"},
  {"aquamarine",      "#7FFFD4"},
  {"turquoise",       "#40E0D0"},
  {"mediumturquoise", "#48D1CC"},
  {"darkturquoise",   "#00CED1"},
  {"cadetblue",       "#5F9EA0"},
  {"steelblue",       "#4682B4"},
  {"lightsteelblue",  "#B0C4DE"},
  {"powderblue",      "#B0E0E6"},
  {"lightblue",       "#ADD8E6"},
  {"skyblue",         "#87CEEB"},
  {"lightskyblue",    "#87CEFA"},
  {"deepskyblue",     "#00BFFF"},
  {"dodgerblue",      "#1E90FF"},
  {"cornflowerblue",  "#6495ED"},
  {"mediumslateblue", "#7B68EE"},
  {"royalblue",       "#4169E1"},
  {"blue",            "#0000FF"},
  {"mediumblue",      "#0000CD"},
  {"darkblue",        "#00008B"},
  {"navy",            "#000080"},
  {"midnightblue",    "#191970"},

  // Brown
  {"cornsilk",       "#FFF8DC"},
  {"blanchedalmond", "#FFEBCD"},
  {"bisque",         "#FFE4C4"},
  {"navajowhite",    "#FFDEAD"},
  {"wheat",          "#F5DEB3"},
  {"burlywood",      "#DEB887"},
  {"tan",            "#D2B48C"},
  {"rosybrown",      "#BC8F8F"},
  {"sandybrown",     "#F4A460"},
  {"goldenrod",      "#DAA520"},
  {"darkgoldenrod",  "#B8860B"},
  {"peru",           "#CD853F"},
  {"chocolate",      "#D2691E"},
  {"saddlebrown",    "#8B4513"},
  {"sienna",         "#A0522D"},
  {"brown",          "#A52A2A"},
  {"maroon",         "#800000"},

  // White
  {"white",         "#FFFFFF"},
  {"snow",          "#FFFAFA"},
  {"honeydew",      "#F0FFF0"},
  {"mintcream",     "#F5FFFA"},
  {"azure",         "#F0FFFF"},
  {"aliceblue",     "#F0F8FF"},
  {"ghostwhite",    "#F8F8FF"},
  {"whitesmoke",    "#F5F5F5"},
  {"seashell",      "#FFF5EE"},
  {"beige",         "#F5F5DC"},
  {"oldlace",       "#FDF5E6"},
  {"floralwhite",   "#FFFAF0"},
  {"ivory",         "#FFFFF0"},
  {"antiquewhite",  "#FAEBD7"},
  {"linen",         "#FAF0E6"},
  {"lavenderblush", "#FFF0F5"},
  {"mistyrose",     "#FFE4E1"},

  // Gray
  {"gainsboro",      "#DCDCDC"},
  {"lightgray",      "#D3D3D3"},
  {"silver",         "#C0C0C0"},
  {"darkgray",       "#A9A9A9"},
  {"gray",           "#808080"},
  {"dimgray",        "#696969"},
  {"lightslategray", "#778899"},
  {"slategray",      "#708090"},
  {"darkslategray",  "#2F4F4F"},
  {"black",          "#000000"}
};

//#endif
