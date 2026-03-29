#pragma once

//#ifndef DRAWABLE_H
//#define DRAWABLE_H

#include "canvas.h"
#include "boundingbox.h"
#include "transformation.h"

#include <concepts> // C++ 20 feature

/**
 * @brief Abstract base class for all drawable objects.
 *
 * Defines the interface that every drawable shape must implement:
 * rendering onto a Canvas, applying a Transformation, computing
 * its BoundingBox, and producing a heap-allocated copy.
 *
 * DrawableList and the operator<< helper allow drawable objects to be
 * composed and rendered without exposing ownership details.
 */
class Drawable {

public:

  virtual ~Drawable() {
  }

  /**
   * @brief Render this object onto a canvas.
   * @param canvas Target canvas to draw on.
   * @return Reference to @p canvas (allows chaining).
   */
  virtual Canvas & draw(Canvas &) const = 0;

  /**
   * @brief Apply an affine transformation to this object in-place.
   * @param t Transformation to apply.
   * @return Reference to this object (allows chaining).
   */
  virtual Drawable & transform(const Transformation &) = 0;

  /**
   * @brief Compute the axis-aligned bounding box of this object.
   * @return The tightest BoundingBox that encloses this object.
   */
  virtual BoundingBox boundingBox() const = 0;

  /**
   * @brief Create a heap-allocated copy of this object.
   *
   * Callers take ownership of the returned pointer.
   * This is used by DrawableList to store polymorphic objects by pointer.
   *
   * @return Pointer to a new copy of this object.
   */
  virtual Drawable * copy() const = 0;
};

/**
 * @brief Convenience stream operator: draws @p drawable onto @p canvas.
 * @param canvas   Target canvas.
 * @param drawable Object to draw.
 * @return Reference to @p canvas.
 */
inline Canvas & operator << (Canvas & canvas, const Drawable & drawable) {
  return drawable.draw(canvas);
}

/**
 * @brief C++20 concept: satisfied by any type derived from Drawable.
 * @tparam T Type to check.
 */
template <typename T>
concept DrawableObject = std::derived_from<T, Drawable>;

/**
 * @brief Return a transformed copy of a drawable object.
 *
 * Creates a copy of @p original and applies @p t to it.
 *
 * @tparam D Concrete drawable type (must satisfy DrawableObject).
 * @param t        Transformation to apply.
 * @param original Object to transform.
 * @return A new object of type D with the transformation applied.
 */
template <DrawableObject D>
inline D operator * (const Transformation & t, const D & original) {
  return D(original).transform(t);
}

/**
 * @brief Create a heap-allocated copy of a drawable object.
 *
 * Thin wrapper around Drawable::copy() that returns a typed pointer.
 *
 * @tparam D Concrete drawable type (must satisfy DrawableObject).
 * @param original Object to copy.
 * @return Pointer to a new heap-allocated copy.
 */
template <DrawableObject D>
inline D * copy(const D & original) {
  return original.copy();
}

//#endif
