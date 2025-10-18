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

#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkType.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkTable` wrapper with fluent API.
 *
 * This simplifies the process of creating a multi-column table. Using the VTK
 * object-oriented interface directly is a bit verbose since APIs are not
 * fluent and because of some of the design decisions made. By using templates
 * and some more functional programming paradigms, the `vtk_table` provides a
 * simpler fluent API, with self-management of column data.
 *
 * For example, to create a table with float columns, one can use:
 *
 * @code{.cc}
 * // lambda for filling rows (other invocables can be used too). i is the row
 * // index and n_rows gives the number of rows in the table to fill. note that
 * // one can use a single-argument lambda that only takes the row index.
 * auto make_row = [](auto i, auto n_rows)
 * {
 *   // points in (-1.2, 1.2). we avoid the left endpoint by using the sequence
 *   // of points {0.5 / n_rows, ... (n_rows - 0.5) / n_rows} before we do the
 *   // affine transform to ensure points are in the desired interval
 *   auto x = -1.2 + 2.4 * (0.5 + i) / n_rows;
 *   // row is composed of x, sin(x), cos(x), tan(x)
 *   return std::make_tuple(x, std::sin(x), std::cos(x), std::tan(x));
 * };
 * // number of points to generate (also number of rows in table)
 * constexpr auto n_points = 50u;
 * // create table with float columns and fill rows using make_row
 * auto table = vtk_table{}
 *   .column<vtkFloatArray>("x")
 *   .column<vtkFloatArray>("sin(x)")
 *   .column<vtkFloatArray>("cos(x)")
 *   .column<vtkFloatArray>("tan(x)")
 *   .rows(n_points, make_row)
 *   ();
 * @endcode
 *
 * The table has implicit conversions to `vtkTable*` for interop with standard
 * VTK functions and allows member access of the underlying `vtkTable` since it
 * also provides an overloaded `operator->`.
 *
 * @tparam P Parent type
 */
template <typename P = void>
class vtk_table : public vtk_skeleton<vtk_table<P>, vtkTable, P> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_table<P>, vtkTable, P);

  /**
   * Return the number of rows in the table.
   */
  auto rows() const
  {
    return object()->GetNumberOfRows();
  }

  /**
   * Return the number of columns in the table.
   */
  auto columns() const
  {
    return object()->GetNumberOfColumns();
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
    // create new column, set name, and add to table
    // note: no leak on scope exit because AddColumn() increments the ref count
    vtkNew<T> col;
    col->SetName(name);
    col->DebugOn();
    object()->AddColumn(col);
    return *this;
  }

  /**
   * Return a pointer to the column given its position.
   *
   * @param col Column index
   */
  auto column(vtkIdType col) const
  {
    return object()->GetColumn(col);
  }

  /**
   * Return a pointer to the column given its name.
   *
   * @param name Unique column name
   */
  auto column(const char* name) const
  {
    return object()->GetColumnByName(name);
  }

  /**
   * Set the number of rows in the table.
   *
   * This should be called after at least one column is added to the table.
   *
   * @todo Consider removing this as it is kind of confusing.
   *
   * @param n_rows Number of rows
   */
  auto& rows(vtkIdType n_rows)
  {
    object()->SetNumberOfRows(n_rows);
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
   * @note The specified row should already exist in the table.
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
   * Fill the table rows using the provided invocable.
   *
   * This should be called after `rows(vtkIdType)` is called. `f` takes the row
   * index and should return a tuple with number of elements equal to the
   * number of columns currently in the table.
   *
   * @note Element types must match those in the `vtkTable`.
   *
   * @tparam F Unary callable returning a tuple of row elements
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

  /**
   * Fill the table rows using the provided invocable.
   *
   * This should be called after `rows(vtkIdType)` is called. `f` takes the row
   * index and total number of rows and should return a tuple with number of
   * elements equal to the number of columns currently in the table.
   *
   * @note Element types must match those in the `vtkTable`.
   *
   * @tparam F Binary callable returning a tuple of row elements
   *
   * @param f Callable to use for filling row elements
   */
  template <typename F>
  auto& rows(F&& f, constraint_t<binary_row_callable<F>::value> = 0)
  {
    for (decltype(rows()) i = 0; i < rows(); i++)
      row(i, std::invoke(f, i, rows()));
    return *this;
  }

  /**
   * Fill the table rows using the provided invocable.
   *
   * After adding columns to the table, call this member to set the number of
   * table rows and then populate each row using the invocable.
   *
   * @note We cannot reverse `n_rows` and `f` as otherwise the `rows(F&&)`
   *  overload is selected (due to the optional argument).
   *
   * @tparam F Unary or binary callable returning a tuple of row elements
   *
   * @param n_rows Number of desired table row
   * @param f Callable to use for filling row elements
   */
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

private:
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
    ([this, i, &vals] { object()->SetValue(i, Is, std::get<Is>(vals)); }(), ...);
    return *this;
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_TABLE_H_
