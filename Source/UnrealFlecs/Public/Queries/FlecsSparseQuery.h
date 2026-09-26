// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

/**
 * C++ query type that iterates directly over sparse component storage.
 *
 * Every component type must have Flecs' compile-time `dont_fragment` trait,
 * must not use `on_instantiate::inherit`, and must already be registered in
 * the owning world. This type has no reflected representation because Flecs
 * selects and validates it from those C++ traits.
 *
 * @tparam TComponents Component field types passed to `flecs::sparse_query`.
 */
template <typename ...TComponents>
using TFlecsSparseQuery = flecs::sparse_query<TComponents...>;
