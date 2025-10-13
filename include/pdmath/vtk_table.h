/**
 * @file vtk_table.h
 * @author Derek Huang
 * @brief C++ header for `vtkTable` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_TABLE_H_
#define PDMATH_VTK_TABLE_H_

#include <cassert>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <vtkAbstractArray.h>
#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkType.h>

#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * `vtkTable` wrapper with fluent API.
 *
 * This simplifies the process of creating a multi-column table.
 *
 * @note This object is move-only since `vtkNew` is used to manage allocations.
 */
class vtk_table {
public:
  /**
   * Return the number of rows in the table.
   */
  auto rows() const
  {
    return table_->GetNumberOfRows();
  }

  /**
   * Return the number of columns in the table.
   */
  auto columns() const
  {
    return table_->GetNumberOfColumns();
  }

  /**
   * Access a member of the container `vtkTable`.
   */
  auto operator->() const noexcept
  {
    return table_.Get();
  }

  /**
   * Implicitly convert into `vtkTable*`.
   */
  operator vtkTable*() const noexcept
  {
    return table_;
  }

  /**
   * Add a new column of the given type and name.
   *
   * This should be a `vtkAbstractArray` subtype and is managed with `vtkNew`.
   *
   * @tparam T `vtkAbstractArray` derived type
   *
   * @param name Unique column name
   */
  template <typename T>
  auto& column(
    const char* name,
    constraint_t<std::is_base_of_v<vtkAbstractArray, T>> = 0)
  {
    assert(name && "name must be valid");
    // create new column + set name
    vtkNew<T> col;
    col->SetName(name);
    // add to table + insert into column vector
    table_->AddColumn(col);
    columns_.push_back(std::move(col));
    return *this;
  }

  /**
   * Return a pointer to the column given its position.
   *
   * @param col Column index
   */
  auto column(vtkIdType col) const
  {
    return table_->GetColumn(col);
  }

  /**
   * Return a pointer to the column given its name.
   *
   * @param name Unique column name
   */
  auto column(const char* name) const
  {
    return table_->GetColumnByName(name);
  }

  /**
   * Set the number of rows in the table.
   *
   * This should be called after at least one column is added.
   *
   * @note Call this *after* adding all the new table columns.
   *
   * @todo Consider removing this as it is kind of confusing.
   *
   * @param n_rows Number of rows
   */
  auto& rows(vtkIdType n_rows)
  {
    table_->SetNumberOfRows(n_rows);
    return *this;
  }

private:
  /**
   * Indicate if a type is a `std::tuple` specialization or not.
   *
   * @tparam T type
   */
  template <typename T>
  struct is_tuple : std::false_type {};

  /**
   * Partial specialization for `std::tuple`.
   *
   * @tparam Ts Tuple types
   */
  template <typename... Ts>
  struct is_tuple<std::tuple<Ts...>> : std::true_type {};

  /**
   * Traits type to indicate a unary callable that can be used to fill a row.
   *
   * @tparam T type
   */
  template <typename T, typename = void>
  struct unary_row_callable : std::false_type {};

  template <typename T>
  struct unary_row_callable<
    T,
    std::enable_if_t<
      std::is_invocable_v<T, vtkIdType /*row index*/> &&
      is_tuple<std::invoke_result_t<T, vtkIdType>>::value
    > > : std::true_type {};

  /**
   * Traits type to indicate a binary callable that can be used to fill a row.
   *
   * @tparam T type
   */
  template <typename T, typename = void>
  struct binary_row_callable : std::false_type {};

  template <typename T>
  struct binary_row_callable<
    T,
    std::enable_if_t<
      std::is_invocable_v<T, vtkIdType /*row index*/, vtkIdType /*row count*/> &&
      is_tuple<std::invoke_result_t<T, vtkIdType, vtkIdType>>::value
    > > : std::true_type {};

public:

  /**
   * Fill a given row with the incoming values.
   *
   * @tparam Ts Tuple types
   *
   * @param i Row index
   * @param vals Tuple of values to set row with
   */
  template <typename... Ts>
  auto& row(vtkIdType i, const std::tuple<Ts...>& vals)
  {
    return row(i, vals, std::index_sequence_for<Ts...>{});
  }

  /**
   * Invoke the given callable on each row.
   *
   * This callable takes the row index and should return a tuple with number of
   * elements equal to the number of columns currently in the table.
   *
   * @note Element types must match those in the `vtkTable`.
   *
   * @tparam F Unary callable returning a tuple of elements for the row
   *
   * @param f Callable to use for filling row elements
   */
  template <typename F>
  auto& rows(F&& f, constraint_t<unary_row_callable<F>::value> = 0)
  {
    // iterate through rows
    for (decltype(rows()) i = 0; i < rows(); i++)
      row(i, std::invoke(f, i));
    return *this;
  }

  // TODO: document more
  template <typename F>
  auto& rows(F&& f, constraint_t<binary_row_callable<F>::value> = 0)
  {
    for (decltype(rows()) i = 0; i < rows(); i++)
      row(i, std::invoke(f, i, rows()));
    return *this;
  }

  // TODO: document more. sets number of rows + fills them with the callable
  // note: cannot reverse n_rows, f as otherwise a different overload is picked
  template <typename F>
  auto& rows(
    vtkIdType n_rows,
    F&& f,
    constraint_t<binary_row_callable<F>::value> = 0)
  {
    rows(n_rows);
    rows(std::forward<F>(f));
    return *this;
  }

  /**
   * Self-move to enable initialization via fluent API.
   */
  auto operator()()
  {
    return std::move(*this);
  }

private:
  vtkNew<vtkTable> table_;                         // table object
  std::vector<vtkNew<vtkAbstractArray>> columns_;  // table columns

  /**
   * Fill a given row with the incoming values.
   *
   * @tparam Ts Tuple types
   * @tparam Is Indices 0 through `sizeof...(Ts)` - 1
   *
   * @param i Row index
   * @param vals Values to set row with
   */
  template <typename... Ts, std::size_t... Is>
  auto& row(
    vtkIdType i,
    const std::tuple<Ts...>& vals,
    std::index_sequence<Is...>)
  {
    // safety checks
    static_assert(sizeof...(Ts) == sizeof...(Is));
    assert(sizeof...(Ts) == columns());
    // fold over tuple types to set row values
    ([this, i, &vals] { table_->SetValue(i, Is, std::get<Is>(vals)); }(), ...);
    return *this;
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_TABLE_H_
