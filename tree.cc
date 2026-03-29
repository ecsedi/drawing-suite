/**
 * @file tree.cc
 * @brief Fractal tree generator.
 *
 * Implements the Tree class and the genTree() entry point.
 * A fractal tree is built by recursively spawning branches from each trunk
 * segment according to a configurable list of Branch descriptors.  Each
 * branch descriptor specifies how its child trunk segment is scaled, rotated,
 * translated, and color-faded relative to its parent.
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

#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <numbers> // C++ 20 feature, needed for pi
#include <iostream>
#include <algorithm>
#include <initializer_list>

#include "json.hpp"

/** @brief Degrees-to-radians conversion factor. */
constexpr double DEG = std::numbers::pi / 180.0;

/**
 * @brief Drawable that renders a fractal tree.
 *
 * Starting from a base trunk Line, growTree() is called recursively.
 * At each level the trunk is drawn and one child branch is spawned per
 * Branch descriptor in the @c branches list.  Each child segment inherits
 * the parent's orientation and color, modified by the Branch parameters.
 */
class Tree : public Drawable {

public:

  /**
   * @brief Describes how a single child branch grows from its parent trunk.
   */
  struct Branch {
    double width_scale;  ///< Child width as a fraction of the parent width ([0,1]).
    double length_scale; ///< Child length as a fraction of the parent length ([0,1]).
    double rotate;       ///< Rotation angle from the parent direction (degrees).
    double translate;    ///< Attachment point along the parent, as a fraction of its length ([0,1]).
    double fade_speed;   ///< Color interpolation speed toward @c fade_to ([0,1]).
    RGB    fade_to;      ///< Target color for the fade.
  };

  Line                base_trunk;      ///< The root trunk segment.
  std::vector<Branch> branches;        ///< Branch descriptors applied at every level.
  int                 iteration_depth; ///< Total recursion depth.

private:

  DrawableList        tree; ///< Flat list of all rendered trunk segments.

public:

  Tree() = delete;

  /**
   * @brief Construct and build a fractal tree from an initializer list of branches.
   * @param trunk    Root trunk line segment.
   * @param branches Branch descriptors.
   * @param depth    Recursion depth.
   */
  Tree(const Line & trunk, std::initializer_list<Branch> branches, int depth)
  : base_trunk(trunk), branches(branches), iteration_depth(depth) {
    growTree(base_trunk, iteration_depth);
  }

  /**
   * @brief Construct and build a fractal tree from a vector of branches.
   * @param trunk    Root trunk line segment.
   * @param branches Branch descriptors.
   * @param depth    Recursion depth.
   */
  Tree(const Line & trunk, const std::vector<Branch> & branches, int depth)
  : base_trunk(trunk), branches(branches), iteration_depth(depth) {
    growTree(base_trunk, iteration_depth);
  }

  Tree(const Tree &) = default;
  Tree(Tree &&) = default;

  virtual ~Tree() override {
  }

  Tree & operator = (const Tree &) = default;
  Tree & operator = (Tree &&) = default;

  /** @copydoc Drawable::draw */
  virtual Canvas & draw(Canvas & canvas) const override {
    return tree.draw(canvas);
  }

  /** @copydoc Drawable::transform */
  virtual Tree & transform(const Transformation & t) override {
    tree.transform(t);
    return *this;
  }

  /** @copydoc Drawable::boundingBox */
  virtual BoundingBox boundingBox() const override {
    return tree.boundingBox();
  }

  /** @copydoc Drawable::copy */
  virtual Tree * copy() const override {
    return new Tree(*this);
  }

private:

  /**
   * @brief Recursively grow the tree from a given trunk segment.
   *
   * At depth 0 the function returns without adding anything.
   * Otherwise the trunk is added to the list, then for each Branch
   * descriptor a child trunk is computed and grown recursively.
   *
   * The child trunk is derived from the parent by:
   *  1. Rotating by @c branch.rotate degrees around the parent's start point.
   *  2. Scaling its length by @c branch.length_scale.
   *  3. Translating the attachment point by @c branch.translate along the parent.
   *  4. Scaling the width by @c branch.width_scale.
   *  5. Fading the color toward @c branch.fade_to at speed @c branch.fade_speed.
   *
   * @param trunk The current trunk segment.
   * @param depth Remaining recursion depth.
   */
  void growTree(const Line & trunk, int depth) {

    if (depth < 1) { return; }
    tree.push_back(trunk);

    auto o1 = Transformation::translate(-trunk.from);
    auto o2 = Transformation::translate( trunk.from);

    for (auto & branch : branches) {
      auto r = Transformation::rotate(branch.rotate * DEG);
      auto s = Transformation::scale(branch.length_scale);
      auto t = Transformation::translate(branch.translate * (trunk.to - trunk.from));
      auto new_trunk   = o2*t*s*r*o1*trunk;
      new_trunk.width *= branch.width_scale;
      new_trunk.color.fade(branch.fade_to, branch.fade_speed);
      growTree(new_trunk, depth-1);
    }
  }
};

/**
 * @brief Parameters for fractal tree generation.
 */
struct TreeParams {
  RGB                       bg_color;        ///< Canvas background color.
  Line                      base_trunk;      ///< Root trunk line segment.
  std::vector<Tree::Branch> branches;        ///< Branch descriptors.
  int                       iteration_depth; ///< Recursion depth.
  unsigned long             resolution;      ///< Approximate total pixel count.
  std::string               imagefile;       ///< Output file path, or "-" for stdout.
};

namespace nlohmann {

  /**
   * @brief JSON serializer specialization for Tree::Branch.
   */
  template <>
  struct adl_serializer<Tree::Branch> {

    /** @brief Serialize Tree::Branch to JSON. */
    static void to_json(json & j, const Tree::Branch & branch) {
      j = json::object();
      j["width_scale" ] = branch.width_scale;
      j["length_scale"] = branch.length_scale;
      j["rotate"      ] = branch.rotate;
      j["translate"   ] = branch.translate;
      j["fade_speed"  ] = branch.fade_speed;
      j["fade_to"     ] = branch.fade_to;
    }

    /** @brief Deserialize Tree::Branch from JSON. */
    static Tree::Branch from_json(const json & j) {
      return {
        j.at("width_scale" ),
        j.at("length_scale"),
        j.at("rotate"      ),
        j.at("translate"   ),
        j.at("fade_speed"  ),
        j.at("fade_to"     )
      };
    }
  };

  /**
   * @brief JSON serializer specialization for TreeParams.
   */
  template <>
  struct adl_serializer<TreeParams> {

    /** @brief Serialize TreeParams to JSON. */
    static void to_json(json & j, const TreeParams & tp) {
      j = json::object();
      j["bg_color"       ] = tp.bg_color;
      j["base_trunk"     ] = tp.base_trunk;
      j["branches"       ] = tp.branches;
      j["iteration_depth"] = tp.iteration_depth;
      j["resolution"     ] = tp.resolution;
      j["imagefile"      ] = tp.imagefile;
    }

    /** @brief Deserialize TreeParams from JSON. */
    static TreeParams from_json(const json & j) {
      return {
        j.at("bg_color"       ).get<RGB                      >(),
        j.at("base_trunk"     ).get<Line                     >(),
        j.at("branches"       ).get<std::vector<Tree::Branch>>(),
        j.at("iteration_depth").get<int                      >(),
        j.at("resolution"     ).get<unsigned long            >(),
        j.value("imagefile", std::string("-"))
      };
    }
  };
}

using json = nlohmann::json;

/**
 * @brief Generate a fractal tree image from JSON parameters and save it.
 *
 * Parses the parameters, builds the tree, fits it to the requested
 * resolution, renders onto a canvas, and saves the result.
 *
 * @param tpjson JSON object containing TreeParams fields.
 */
void genTree(const json & tpjson) {

  TreeParams tp = tpjson.get<TreeParams>();
  Tree       tree(tp.base_trunk, tp.branches, tp.iteration_depth);
  auto       bbox        = tree.boundingBox() * 1.05;
  auto       screen_size = bbox.screenSize(tp.resolution);

  Canvas pic(screen_size.width, screen_size.height, tp.bg_color);

  auto ts = Transformation::toScreen(bbox, pic.width(), pic.height());

  pic << ts * tree;
  pic.save(tp.imagefile);

  if (tp.imagefile != "-") {
    std::cerr << "Saved image to " << tp.imagefile << std::endl;
  }
}

#ifdef STANDALONE

/**
 * @brief Standalone entry point (only compiled when STANDALONE is defined).
 *
 * Usage: tree <JSON_FILE>
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

  genTree(readJsonFile(argv[1]));
  return 0;
}

#endif
