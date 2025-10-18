/**
 * @file vtk_skeleton.h
 * @author Derek Huang
 * @brief C++ header for a VTK object skeleton template
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_SKELETON_H_
#define PDMATH_VTK_SKELETON_H_

#include <type_traits>

#include <vtkNew.h>

namespace pdmath {

/**
 * Skeleton template for a potentially-owned VTK object.
 *
 * This assists in implementing the fluent API provided by the pdmath VTK
 * support library by acting as a mini-expression template for declarative
 * construction of VTK GUI windows and object graphs. Essentially, via a
 * template argument, it enables modeling of these 2 cases:
 *
 * 1. Object is self-contained and owns its own data via `vtkNew`
 * 2. Object is owned by a parent (i.e. parent has an object reference)
 *
 * The reason we need this construct instead of using `vtkSharedPointer` is
 * because for the fluent, declarative API to work, we need to know at
 * compile-time whether a supporting skeleton object owns a VTK object with
 * `vtkNew` (of course, other VTK objects can increase the internal reference
 * count) or if the VTK object is owned by a parent object and the skeleton is
 * only being provided to support a fluent API.
 *
 * This can be demonstrated by two contrasting examples. First, consider a
 * standalone `vtk_table`:
 *
 * @code{.cc}
 * // assume make_row is an appropriately defined lambda
 * // vtk_table owns a vtkTable using vtkNew
 * auto table = vtk_table{}
 *   .column<vtkFloatArray>("x")
 *   .column<vtkFloatArray>("sin(x)")
 *   .column<vtkFloatArray>("cos(x)")
 *   .rows(100u, make_row)
 *   // std::move(*this) to initialize table
 *   ();
 * @endcode
 *
 * Now consider one that is created for a `vtkContextScene`:
 *
 * @code{.cc}
 * // ... owning vtk_* object
 *   // returns a vtk_table<parent_object>&
 *   .table()
 *     .column<vtkFloatArray>("x")
 *     .column<vtkFloatArray>("sin(x)")
 *     .column<vtkFloatArray>("cos(x)")
 *     .rows(100u, make_row)
 *   // return lvalue reference to the *parent*
 *   ()
 * // ... continue, e.g. add another table, etc.
 *   // std::move(*this) on the parent object
 *   ();
 * @endcode
 *
 * This also provides the `operator->` usable by the actual derived `T` type to
 * provide access to the VTK object and the `operator()` needed to mark the
 * "end" of a declarative configuration block.
 *
 * @tparam T CRTP derived type
 * @tparam V VTK object type
 * @tparam P Parent type, e.g. another `vtk_skeleton`, or `void` for owning
 */
template <typename T, typename V, typename P>
class vtk_skeleton {
public:
  // owning or non-owning
  static constexpr bool owning = std::is_void_v<P>;
  // parent object type or void
  using parent_type = P;
  // vtkNew if owning and raw pointer for object otherwise
  using object_ptr = std::conditional_t<owning, vtkNew<V>, V*>;

private:
  /**
   * Skeleton member state.
   *
   * If the ownership type is `void` then there is no parent pointer.
   *
   * @tparam O Owning parent type
   */
  template <typename O, typename = void>
  struct state {
    object_ptr object;
    O* parent;
  };

  /**
   * Partial specialization for owning skeletons.
   *
   * @note We use a partial specialization as explicit specializations of
   *  nested templates are not allows in C++.
   *
   * @par
   *
   * @note Since `vtkNew` is used this will be move-only.
   */
  template <typename O>
  struct state<O, O /*void*/> {
    object_ptr object;
  };

public:
  /**
   * Default ctor.
   *
   * This is conditionally provided so a dummy template parameter is used.
   */
  template <typename D = int>
  vtk_skeleton(std::enable_if_t<owning, D> = 0) : state_{} {}

  /**
   * Ctor.
   *
   * This is conditionally provided so a dummy template parameter is used.
   *
   * @param object VTK object to own
   * @param parent Parent object
   */
  template <typename D = int>
  vtk_skeleton(
    V* object,
    P* parent = nullptr,
    std::enable_if_t<!owning, D> = 0) noexcept
    : state_{object, parent}
  {}

  /**
   * Return a pointer to the VTK object.
   */
  auto object() const noexcept
  {
    if constexpr (owning)
      return state_.object.Get();  // vtkNew<T>
    else
      return state_.object;        // T*
  }

  /**
   * Return a pointer to the parent pointer.
   *
   * This member is conditionally provided so a dummy template parameter is
   * used to make it type-dependent so SFINAE will work.
   */
  template <typename D = int>
  auto parent(std::enable_if_t<!owning, D> = 0) const noexcept
  {
    return state_.parent;
  }

  /**
   * Enable implicit conversion to the VTK object pointer.
   */
  operator V*() const noexcept
  {
    return this->object();
  }

  /**
   * Provided member access to the pointed-to VTK object.
   */
  auto operator->() const noexcept
  {
    return this->object();  // add template parameter dependence
  }

  /**
   * Return reference to support method chaining.
   *
   * For owning skeletons this is `*this` as a `T` xvalue while for non-owning
   * skeletons this is an lvalue reference to the parent.
   */
  auto&& operator()()
    noexcept(!owning || noexcept(std::move(*static_cast<T*>(this))))
  {
    if constexpr (owning)
      return std::move(*static_cast<T*>(this));
    else
      return *this->parent();  // add template parameter dependence
  }

private:
  state<P> state_;
};

/**
 * Macro for `using` declarations to bring in skeleton type operators.
 *
 * This defines the `skeleton` type alias for the `vtk_skeleton` base and
 * brings the ctor, `operator->`, `operator()`, `object()`, and `parent()`
 * members of the skeleton type into scope for convenience.
 *
 * @param ... Type arguments to `vtk_skeleton`
 */
#define PDMATH_USING_VTK_SKELETON(...) \
  using skeleton = vtk_skeleton<__VA_ARGS__>; \
  using skeleton::skeleton; \
  using skeleton::operator->; \
  using skeleton::operator(); \
  using skeleton::object; \
  using skeleton::parent

}  // namespace pdmath

#endif  // PDMATH_VTK_SKELETON_H_
