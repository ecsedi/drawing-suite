#pragma once

//#ifndef DRAWABLELIST_H
//#define DRAWABLELIST_H

#include "canvas.h"
#include "drawable.h"
#include "transformation.h"

#include <vector>
#include <algorithm>
#include <initializer_list>

/**
 * @brief An ordered, owning collection of Drawable objects.
 *
 * DrawableList is essentially a vector of Drawable objects.  Because Drawable
 * is an abstract class, std::vector<Drawable> is impossible.  Instead, the
 * list stores heap-allocated pointers (std::vector<Drawable*>) and hides
 * pointer management behind custom iterator classes.
 *
 * The list owns its elements: it deep-copies on construction and copy-
 * assignment (via Drawable::copy()), and deletes each element on destruction
 * or clear().  Move construction and move assignment transfer ownership
 * without copying.
 *
 * DrawableList itself satisfies the Drawable interface, so lists can be
 * nested inside other lists.
 */
class DrawableList : public Drawable {

public:

  /** @brief Size / index type. */
  typedef size_t size_type;

private:

  std::vector<Drawable*> items;

public:

  /**
   * @brief Mutable forward iterator over the Drawable elements.
   *
   * Dereferences to Drawable& rather than Drawable* to hide pointer details.
   */
  class iterator {

  private:

    DrawableList * list;
    size_type      pos;

    iterator() = delete;

    iterator(DrawableList * dl, size_type n) : list(dl), pos(n) {
    }

  public:

    /** @brief Advance to the next element. */
    iterator & operator ++ () {
      ++pos;
      return *this;
    }

    /** @brief Dereference to the current Drawable. */
    Drawable & operator * () const {
      return *(list->items[pos]);
    }

    /** @brief Equality comparison. */
    bool operator == (const iterator & other) const {
      return list == other.list && pos == other.pos;
    }

    /** @brief Inequality comparison. */
    bool operator != (const iterator & other) const {
      return !(*this == other);
    }

    friend class DrawableList;
  };

  /**
   * @brief Immutable forward iterator over the Drawable elements.
   */
  class const_iterator {

  private:

    const DrawableList * list;
    size_type            pos;

    const_iterator() = delete;

    const_iterator(const DrawableList * dl, size_type n) : list(dl), pos(n) {
    }

  public:

    /** @brief Advance to the next element. */
    const_iterator & operator ++ () {
      ++pos;
      return *this;
    }

    /** @brief Dereference to the current Drawable. */
    const Drawable & operator * () const {
      return *(list->items[pos]);
    }

    /** @brief Equality comparison. */
    bool operator == (const const_iterator & other) const {
      return list == other.list && pos == other.pos;
    }

    /** @brief Inequality comparison. */
    bool operator != (const const_iterator & other) const {
      return !(*this == other);
    }

    friend class DrawableList;
  };

  /**
   * @brief Destroy all elements and empty the list.
   *
   * Each stored pointer is deleted via the virtual destructor of Drawable.
   */
  void clear() {
    auto remove = [] (Drawable * d) { delete d; };
    std::for_each(items.begin(), items.end(), remove);
    items.clear();
  }

  /** @brief Default constructor. Creates an empty list. */
  DrawableList() = default;

  /**
   * @brief Construct a list containing a single copy of @p item.
   * @param item Initial element (deep-copied via Drawable::copy()).
   */
  explicit DrawableList(const Drawable & item)
  : items({item.copy()}) {
  }

  /**
   * @brief Copy constructor. Deep-copies all elements from @p other.
   * @param other Source list.
   */
  DrawableList(const DrawableList & other) {
    items.reserve(other.size());
    for (auto const & orig : other) {
      items.push_back(orig.copy());
    }
  }

  /**
   * @brief Move constructor. Transfers ownership of all elements.
   * @param other Source list (left empty after the move).
   */
  DrawableList(DrawableList && other) {
    items = std::move(other.items);
  }

  /**
   * @brief Destructor. Deletes all stored elements.
   */
  virtual ~DrawableList() override {
    clear();
  }

  /**
   * @brief Copy-assignment. Deep-copies all elements from @p other.
   * @param other Source list.
   * @return Reference to this object.
   */
  DrawableList & operator = (const DrawableList & other) {
    if (this != &other) {
      clear();
      items.reserve(other.size());
      for (auto const & orig : other) {
        items.push_back(orig.copy());
      }
    }
    return *this;
  }

  /**
   * @brief Move-assignment. Transfers ownership of all elements.
   * @param other Source list (left empty after the move).
   * @return Reference to this object.
   */
  DrawableList & operator = (DrawableList && other) {
    if (this != &other) {
      clear();
      items = std::move(other.items);
    }
    return *this;
  }

  /**
   * @brief Non-const element access by index.
   * @param n Zero-based index.
   * @return Reference to the n-th Drawable.
   */
  Drawable & operator [] (size_type n) {
    return *(items[n]);
  }

  /**
   * @brief Const element access by index.
   * @param n Zero-based index.
   * @return Const reference to the n-th Drawable.
   */
  const Drawable & operator [] (size_type n) const {
    return *(items[n]);
  }

  /**
   * @brief Return the number of elements in the list.
   * @return Element count.
   */
  size_type size() const {
    return items.size();
  }

  /**
   * @brief Check whether the list is empty.
   * @return true if there are no elements.
   */
  bool empty() const {
    return items.empty();
  }

  /** @brief Return a mutable iterator to the first element. */
  iterator begin() {
    return iterator(this, 0);
  }

  /** @brief Return a const iterator to the first element. */
  const_iterator begin() const {
    return const_iterator(this, 0);
  }

  /** @brief Return a mutable iterator past the last element. */
  iterator end() {
    return iterator(this, items.size());
  }

  /** @brief Return a const iterator past the last element. */
  const_iterator end() const {
    return const_iterator(this, items.size());
  }

  /**
   * @brief Append a copy of @p item to the list.
   * @param item Element to copy and append (via Drawable::copy()).
   */
  void push_back(const Drawable & item) {
    items.push_back(item.copy());
  }

  /**
   * @brief Apply a transformation to every element in the list.
   * @param t Transformation to apply.
   * @return Reference to this list.
   */
  virtual DrawableList & transform(const Transformation & t) override {
    auto trans = [&t] (Drawable * object) { object->transform(t); };
    std::for_each(items.begin(), items.end(), trans);
    return *this;
  }

  /**
   * @brief Draw all elements onto @p canvas in order.
   * @param canvas Target canvas.
   * @return Reference to @p canvas.
   */
  virtual Canvas & draw(Canvas & canvas) const override {
    auto plot = [&canvas] (const Drawable * object) { canvas << *object; };
    std::for_each(items.begin(), items.end(), plot);
    return canvas;
  }

  /**
   * @brief Compute the bounding box that encloses all elements.
   * @return Union of all element bounding boxes.
   */
  virtual BoundingBox boundingBox() const override {
    BoundingBox bbox;
    auto unite = [&bbox] (const Drawable * object) { bbox += object->boundingBox(); };
    std::for_each(items.begin(), items.end(), unite);
    return bbox;
  }

  /**
   * @brief Create a heap-allocated deep copy of this list.
   * @return Pointer to a new DrawableList (caller takes ownership).
   */
  virtual DrawableList * copy() const override {
    return new DrawableList(*this);
  }
};

//#endif
