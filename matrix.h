#pragma once

//#ifndef MATRIX_H
//#define MATRIX_H

#include <vector>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>

/**
 * @brief Generic 2D matrix stored in row-major order.
 *
 * Provides element access by (row, col) index with both checked (at()) and
 * unchecked (operator()) variants, as well as forward iterators over all
 * elements in row-major order.
 *
 * @tparam T Element type. Must be default-constructible.
 */
template <typename T>
class Matrix {

public:

  /** @brief Element type. */
  typedef T      value_type;

  /** @brief Size / index type. */
  typedef size_t size_type;

  /** @brief Mutable iterator over elements in row-major order. */
  typedef typename std::vector<value_type>::iterator       iterator;

  /** @brief Immutable iterator over elements in row-major order. */
  typedef typename std::vector<value_type>::const_iterator const_iterator;

private:

  size_type               num_rows;
  size_type               num_cols;
  std::vector<value_type> data; ///< Single contiguous vector for elements in row-major order.

public:

  Matrix() = delete;

  /**
   * @brief Construct a matrix of given dimensions with default-initialized elements.
   * @param rows Number of rows.
   * @param cols Number of columns.
   */
  constexpr Matrix(size_type rows, size_type cols)
  : num_rows(rows), num_cols(cols), data(rows * cols) {
  }

  /**
   * @brief Construct a matrix filled with a constant value.
   * @param rows  Number of rows.
   * @param cols  Number of columns.
   * @param value Fill value for all elements.
   */
  constexpr Matrix(size_type rows, size_type cols, const value_type & value)
  : num_rows(rows), num_cols(cols), data(rows * cols, value) {
  }

  /**
   * @brief Construct from an initializer list of elements in row-major order.
   * @param rows   Number of rows.
   * @param cols   Number of columns.
   * @param values Elements in row-major order; count must equal rows * cols.
   * @throw std::invalid_argument if the number of values does not match rows * cols.
   */
  constexpr Matrix(size_type rows, size_type cols, std::initializer_list<value_type> values)
  : num_rows(rows), num_cols(cols), data(values) {
    if (data.size() != num_rows*num_cols) {
      throw std::invalid_argument("Bad matrix dimensions");
    }
  }

  Matrix(const Matrix &) = default;
  Matrix(Matrix &&) = default;
  ~Matrix() = default;
  Matrix & operator = (const Matrix &) = default;
  Matrix & operator = (Matrix &&) = default;

  /**
   * @brief Return the number of rows.
   * @return Row count.
   */
  size_type rows() const {
    return num_rows;
  }

  /**
   * @brief Return the number of columns.
   * @return Column count.
   */
  size_type cols() const {
    return num_cols;
  }

  /**
   * @brief Return the total number of elements (rows * cols).
   * @return Element count.
   */
  size_type size() const {
    return data.size();
  }

  /**
   * @brief Non-const element access without bounds checking.
   * @param row Row index.
   * @param col Column index.
   * @return Reference to the element at (row, col).
   */
  value_type & operator() (size_type row, size_type col) {
    return data[row * num_cols + col];
  }

  /**
   * @brief Const element access without bounds checking.
   * @param row Row index.
   * @param col Column index.
   * @return Const reference to the element at (row, col).
   */
  const value_type & operator() (size_type row, size_type col) const {
    return data[row * num_cols + col];
  }

  /**
   * @brief Bounds-checked non-const element access.
   * @param row Row index.
   * @param col Column index.
   * @return Reference to the element at (row, col).
   * @throw std::out_of_range if row or col is out of bounds.
   */
  value_type & at(size_type row, size_type col) {

    if (row >= num_rows || col >= num_cols) {
      throw std::out_of_range("Matrix index out of bounds");
    }

    return data[row * num_cols + col];
  }

  /**
   * @brief Bounds-checked const element access.
   * @param row Row index.
   * @param col Column index.
   * @return Const reference to the element at (row, col).
   * @throw std::out_of_range if row or col is out of bounds.
   */
  const value_type & at(size_type row, size_type col) const {

    if (row >= num_rows || col >= num_cols) {
      throw std::out_of_range("Matrix index out of bounds");
    }

    return data[row * num_cols + col];
  }

  /** @brief Return a mutable iterator to the first element. */
  iterator begin() {
    return data.begin();
  }

  /** @brief Return a mutable iterator past the last element. */
  iterator end() {
    return data.end();
  }

  /** @brief Return a const iterator to the first element. */
  const_iterator begin() const {
    return data.begin();
  }

  /** @brief Return a const iterator past the last element. */
  const_iterator end() const {
    return data.end();
  }

  /** @brief Return a const iterator to the first element. */
  const_iterator cbegin() const {
    return begin();
  }

  /** @brief Return a const iterator past the last element. */
  const_iterator cend() const {
    return end();
  }
};

/**
 * @brief Stream output operator for Matrix.
 *
 * Prints elements in row-major order: elements on the same row are separated
 * by spaces, rows are separated by newlines.
 *
 * @tparam T Element type.
 * @param os  Output stream.
 * @param mat Matrix to print.
 * @return Reference to output stream.
 */
template <typename T>
inline std::ostream & operator << (std::ostream & os, const Matrix<T> & mat) {

  for (typename Matrix<T>::size_type i = 0; i < mat.rows(); ++i) {
    os << (i ? "\n" : "");
    for (typename Matrix<T>::size_type j = 0; j < mat.cols(); ++j) {
      os << (j ? " " : "") << mat(i, j);
    }
  }

  return os;
}

/**
 * @brief Stream input operator for Matrix.
 *
 * Reads elements in row-major order into an existing matrix.
 * The matrix dimensions must already be set; the operator simply fills the
 * existing storage without resizing.
 *
 * @tparam T Element type.
 * @param is  Input stream.
 * @param mat Matrix to fill.
 * @return Reference to input stream.
 */
template <typename T>
inline std::istream & operator >> (std::istream & is, Matrix<T> & mat) {

  for (auto & item : mat) {
    is >> item;
  }

  return is;
}

//#endif
