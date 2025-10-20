/**
 * @file vtk_actor.h
 * @author Derek Huabg
 * @brief C++ header for `vtkActor` or `vtkContextActor` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_ACTOR_H_
#define PDMATH_VTK_ACTOR_H_

#include <vtkAbstractContextItem.h>
#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkNew.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkContextScene` wrapper with fluent API.
 *
 * @tparam P Parent type
 */
template <typename P = void>
class vtk_context_scene :
  public vtk_skeleton<vtk_context_scene<P>, vtkContextScene, P> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_context_scene<P>, vtkContextScene, P);

  /**
   * Add a context item to the scene.
   *
   * For example, this could be a `vtkChartXY`.
   *
   * @param item Item to add to the scene
   */
  auto& add(vtkAbstractContextItem* item)
  {
    object()->AddItem(item);
    return *this;
  }

  /**
   * Add a new context item to the scene.
   *
   * This returns the appropriate `vtk_skeleton` derived type that corresponds
   * to the provided VTK prop type using `vtk_skeleton_traits`.
   *
   * @tparam T VTK object type
   */
  template <typename T>
  auto add(constraint_t<std::is_base_of_v<vtkAbstractContextItem, T>> = 0)
  {
    vtkNew<T> item;
    // note: reference increment means no use-after-free error on scope exit
    add(item);
    return vtk_skeleton_type_t<T, vtk_context_scene>{item, this};
  }
};

/**
 * Partial specialization for `vtk_skeleton_type`.
 *
 * @tparam P Parent type
 */
template <typename P>
struct vtk_skeleton_type<vtkContextScene, P> {
  using type = vtk_context_scene<P>;
};

/**
 * `vtkContextActor` wrapper with fluent API.
 *
 * @tparam P Parent type
 */
template <typename P = void>
class vtk_context_actor
  : public vtk_skeleton<vtk_context_actor<P>, vtkContextActor, P> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_context_actor<P>, vtkContextActor, P);

  /**
   * Set the actor's scene.
   *
   * @param scn Scene to use for the actor
   */
  auto& scene(vtkContextScene* scn)
  {
    object()->SetScene(scn);
    return *this;
  }

  /**
   * Get the actor's scene.
   *
   * This returns the `vtk_skeleton` derived type for `vtkContextScene`.
   */
  auto scene()
  {
    auto scn = object()->GetScene();
    return vtk_skeleton_type_t<vtkContextScene, vtk_context_actor>{scn, this};
  }
};

/**
 * Partial specialization for `vtk_skeleton_type`.
 *
 * @tparam P Parent type
 */
template <typename P>
struct vtk_skeleton_type<vtkContextActor, P> {
  using type = vtk_context_actor<P>;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_ACTOR_H_
