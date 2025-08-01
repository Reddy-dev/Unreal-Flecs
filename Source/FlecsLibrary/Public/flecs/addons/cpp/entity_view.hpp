/**
 * @file addons/cpp/entity_view.hpp
 * @brief Entity class with only readonly operations.
 * 
 * This class provides readonly access to entities. Using this class to store 
 * entities in components ensures valid handles, as this class will always store
 * the actual world vs. a stage. The constructors of this class will never 
 * create a new entity.
 *
 * To obtain a mutable handle to the entity, use the "mut" function.
 */

#pragma once

/**
 * @ingroup cpp_entities
 * @{
 */

namespace flecs
{

/** Entity view.
 * Class with read operations for entities. Base for flecs::entity.
 * 
 * @ingroup cpp_entities
 */
struct entity_view : public id {

    entity_view() : flecs::id() { }

    /** Wrap an existing entity id.
     *
     * @param world The world in which the entity is created.
     * @param id The entity id.
     */
    explicit entity_view(flecs::world_t *world, flecs::id_t id)
        : flecs::id(world 
            ? const_cast<flecs::world_t*>(ecs_get_world(world))
            : nullptr
        , id ) { }

    /** Implicit conversion from flecs::entity_t to flecs::entity_view. */
    entity_view(entity_t id) 
        : flecs::id( nullptr, id ) { }

    /** Get entity id.
     * @return The integer entity id.
     */
    entity_t id() const {
        return id_;
    }

    /** Check if entity is valid.
     * An entity is valid if:
     * - its id is not 0
     * - the id contains a valid bit pattern for an entity
     * - the entity is alive (see is_alive())
     *
     * @return True if the entity is valid, false otherwise.
     * @see ecs_is_valid()
     */
    bool is_valid() const {
        return world_ && ecs_is_valid(world_, id_);
    }
  
    explicit operator bool() const {
        return is_valid();
    }

    /** Check if entity is alive.
     *
     * @return True if the entity is alive, false otherwise.
     * @see ecs_is_alive()
     */
    bool is_alive() const {
        return world_ && ecs_is_alive(world_, id_);
    }

    /** Return the entity name.
     *
     * @return The entity name.
     */
    flecs::string_view name() const {
        return flecs::string_view(ecs_get_name(world_, id_));
    }

    /** Return the entity symbol.
     *
     * @return The entity symbol.
     */
    flecs::string_view symbol() const {
        return flecs::string_view(ecs_get_symbol(world_, id_));
    }

    /** Return the entity path.
     *
     * @return The hierarchical entity path.
     */
    flecs::string path(const char *sep = "::", const char *init_sep = "::") const {
        return path_from(0, sep, init_sep);
    }   

    /** Return the entity path relative to a parent.
     *
     * @return The relative hierarchical entity path.
     */
    flecs::string path_from(flecs::entity_t parent, const char *sep = "::", const char *init_sep = "::") const {
        char *path = ecs_get_path_w_sep(world_, parent, id_, sep, init_sep);
        return flecs::string(path);
    }

    /** Return the entity path relative to a parent.
     *
     * @return The relative hierarchical entity path.
     */
    template <typename Parent>
    flecs::string path_from(const char *sep = "::", const char *init_sep = "::") const {
        return path_from(_::type<Parent>::id(world_), sep, init_sep);
    }

    bool enabled() const {
        return !ecs_has_id(world_, id_, flecs::Disabled);
    }

    /** Get the entity's type.
     *
     * @return The entity's type.
     */
    flecs::type type() const;

    /** Get the entity's table.
     *
     * @return Returns the entity's table.
     */
    flecs::table table() const;

    /** Get table range for the entity.
     * Returns a range with the entity's row as offset and count set to 1. If
     * the entity is not stored in a table, the function returns a range with
     * count 0.
     *
     * @return Returns the entity's table range.
     */
    flecs::table_range range() const;

    /** Iterate (component) ids of an entity.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::id id)
     * @endcode
     *
     * @param func The function invoked for each id.
     */
    template <typename Func>
    void each(const Func& func) const;

    /** Iterate matching pair ids of an entity.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::id id)
     * @endcode
     *
     * @param func The function invoked for each id.
     */
    template <typename Func>
    void each(flecs::id_t first, flecs::id_t second, const Func& func) const;

    /** Iterate targets for a given relationship.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::entity target)
     * @endcode
     *
     * @param rel The relationship for which to iterate the targets.
     * @param func The function invoked for each target.
     */
    template <typename Func>
    void each(const flecs::entity_view& rel, const Func& func) const;

    /** Iterate targets for a given relationship.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::entity target)
     * @endcode
     *
     * @tparam First The relationship for which to iterate the targets.
     * @param func The function invoked for each target.     
     */
    template <typename First, typename Func>
    void each(const Func& func) const { 
        return each(_::type<First>::id(world_), func);
    }

    /** Iterate children for entity.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::entity target)
     * @endcode
     *
     * @param rel The relationship to follow.
     * @param func The function invoked for each child.     
     */
    template <typename Func>
    void children(flecs::entity_t rel, Func&& func) const {
        /* When the entity is a wildcard, this would attempt to query for all
         * entities with (ChildOf, *) or (ChildOf, _) instead of querying for
         * the children of the wildcard entity. */
        if (id_ == flecs::Wildcard || id_ == flecs::Any) {
            /* This is correct, wildcard entities don't have children */
            return;
        }

        flecs::world world(world_);

        if (rel == flecs::ChildOf) {
            ecs_iter_t it = ecs_children(world_, id_);
            while (ecs_children_next(&it)) {
                _::each_delegate<Func>(FLECS_MOV(func)).invoke(&it);
            }
        } else {
            ecs_iter_t it = ecs_each_id(world_, ecs_pair(rel, id_));
            while (ecs_each_next(&it)) {
                _::each_delegate<Func>(FLECS_MOV(func)).invoke(&it);
            }
        }
    }

    /** Iterate children for entity.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::entity target)
     * @endcode
     *
     * @tparam Rel The relationship to follow.
     * @param func The function invoked for each child.     
     */
    template <typename Rel, typename Func>
    void children(Func&& func) const {
        children(_::type<Rel>::id(world_), FLECS_MOV(func));
    }

    /** Iterate children for entity.
     * The function parameter must match the following signature:
     *
     * @code
     * void(*)(flecs::entity target)
     * @endcode
     * 
     * This operation follows the ChildOf relationship.
     *
     * @param func The function invoked for each child.     
     */
    template <typename Func>
    void children(Func&& func) const {
        children(flecs::ChildOf, FLECS_MOV(func));
    }


    /* try_get */

    /** Get component value.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, if_t< is_actual<T>::value > = 0>
    const T* try_get() const {
        auto comp_id = _::type<T>::id(world_);
        ecs_assert(_::type<T>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<const T*>(ecs_get_id(world_, id_, comp_id));
    }

    /** Get component value.
     * Overload for when T is not the same as the actual type, which happens
     * when using pair types.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, typename A = actual_type_t<T>, 
        if_t< flecs::is_pair<T>::value > = 0>
    const A* try_get() const {
        auto comp_id = _::type<T>::id(world_);
        ecs_assert(_::type<A>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<const A*>(ecs_get_id(world_, id_, comp_id));
    }

    /** Get a pair.
     * This operation gets the value for a pair from the entity.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template <typename First, typename Second, typename P = pair<First, Second>, 
        typename A = actual_type_t<P>, if_not_t< flecs::is_pair<First>::value > = 0>
    const A* try_get() const {
        return this->try_get<P>();
    }

    /** Get a pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     */
    template<typename First, typename Second, if_not_t< is_enum<Second>::value> = 0>
    const First* try_get(Second second) const {
        auto first = _::type<First>::id(world_);
        ecs_assert(_::type<First>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<const First*>(
            ecs_get_id(world_, id_, ecs_pair(first, second)));
    }

    /** Get a pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param constant the enum constant.
     */
    template<typename First, typename Second, if_t< is_enum<Second>::value && !std::is_same<First, Second>::value > = 0>
    const First* try_get(Second constant) const {
        const auto& et = enum_type<Second>(this->world_);
        flecs::entity_t target = et.entity(constant);
        return try_get<First>(target);
    }

    /** Get component value (untyped).
     * 
     * @param comp The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    const void* try_get(flecs::id_t comp) const {
        return ecs_get_id(world_, id_, comp);
    }

    /** Get a pair (untyped).
     * This operation gets the value for a pair from the entity. If neither the
     * first nor the second part of the pair are components, the operation 
     * will fail.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     */
    const void* try_get(flecs::entity_t first, flecs::entity_t second) const {
        return ecs_get_id(world_, id_, ecs_pair(first, second));
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam Second the second element of a pair.
     * @param first The first part of the pair.
     */
    template<typename Second>
    const Second* try_get_second(flecs::entity_t first) const {
        auto second = _::type<Second>::id(world_);
        ecs_assert( ecs_get_type_info(world_, ecs_pair(first, second)) != NULL,
            ECS_INVALID_PARAMETER, "pair is not a component");
        ecs_assert( ecs_get_type_info(world_, ecs_pair(first, second))->component == second,
            ECS_INVALID_PARAMETER, "type of pair is not Second");
        ecs_assert(_::type<Second>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<const Second*>(
            ecs_get_id(world_, id_, ecs_pair(first, second)));
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template<typename First, typename Second>
    const Second* try_get_second() const {
        return try_get<pair_object<First, Second>>();
    }


    /* get */

    /** Get component value.
     * 
     * @tparam T The component to get.
     * @return Ref to the component value, panics if the entity does not
     *         have the component.
     */
    template <typename T, if_t< is_actual<T>::value > = 0>
    const T& get() const {
        const T *r = try_get<T>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return *r;
    }

    /** Get component value.
     * Overload for when T is not the same as the actual type, which happens
     * when using pair types.
     * 
     * @tparam T The component to get.
     * @return Ref to the component value, panics if the entity does not
     *         have the component.
     */
    template <typename T, typename A = actual_type_t<T>, 
        if_t< flecs::is_pair<T>::value > = 0>
    const A& get() const {
        const A *r = try_get<T>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return *r;
    }
    
    /** Get a pair.
     * This operation gets the value for a pair from the entity.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     * @return Ref to the component value, panics if the entity does not
     *         have the component.
     */
    template <typename First, typename Second, typename P = pair<First, Second>, 
        typename A = actual_type_t<P>, if_not_t< flecs::is_pair<First>::value > = 0>
    const A& get() const {
        return this->get<P>();
    }

    /** Get a pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     * @return Ref to the component value, panics if the entity does not
     *         have the component.
     */
    template<typename First, typename Second, if_not_t< is_enum<Second>::value> = 0>
    const First& get(Second second) const {
        const First *r = try_get<First>(second);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return *r;
    }

    /** Get a pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param constant the enum constant.
     * @return Ref to the component value, panics if the entity does not
     *         have the component.
     */
    template<typename First, typename Second, if_t< is_enum<Second>::value && !std::is_same<First, Second>::value > = 0>
    const First& get(Second constant) const {
        const auto& et = enum_type<Second>(this->world_);
        flecs::entity_t target = et.entity(constant);
        return get<First>(target);
    }

    /** Get component value (untyped).
     * 
     * @param comp The component to get.
     * @return Pointer to the component value, panics if the entity does not
     *         have the component.
     */
    const void* get(flecs::id_t comp) const {
        const void *r = ecs_get_id(world_, id_, comp);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return r;
    }

    /** Get a pair (untyped).
     * This operation gets the value for a pair from the entity. If neither the
     * first nor the second part of the pair are components, the operation 
     * will fail.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     * @return Pointer to the component value, panics if the entity does not
     *         have the component.
     */
    const void* get(flecs::entity_t first, flecs::entity_t second) const {
        const void *r = ecs_get_id(world_, id_, ecs_pair(first, second));
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return r;
    }

    /** Get 1..N components.
     * This operation accepts a callback with as arguments the components to
     * retrieve. The callback will only be invoked when the entity has all
     * the components.
     *
     * This operation is faster than individually calling get for each component
     * as it only obtains entity metadata once.
     * 
     * While the callback is invoked the table in which the components are
     * stored is locked, which prevents mutations that could cause invalidation
     * of the component references. Note that this is not an actual lock: 
     * invalid access causes a runtime panic and so it is still up to the 
     * application to ensure access is protected.
     * 
     * The component arguments must be references and can be either const or
     * non-const. When all arguments are const, the function will read-lock the
     * table (see ecs_read_begin). If one or more arguments are non-const the
     * function will write-lock the table (see ecs_write_begin).
     * 
     * Example:
     *
     * @code
     * e.get([](Position& p, Velocity& v) { // write lock
     *   p.x += v.x;
     * });
     * 
     * e.get([](const Position& p) {        // read lock
     *   std::cout << p.x << std::endl;
     * });
     * @endcode
     *
     * @param func The callback to invoke.
     * @return True if the entity has all components, false if not.
     */
    template <typename Func, if_t< is_callable<Func>::value > = 0>
    bool get(const Func& func) const;

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam Second the second element of a pair.
     * @param first The first part of the pair.
     */
    template<typename Second>
    const Second& get_second(flecs::entity_t first) const {
        const Second *r = try_get_second<Second>(first);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return *r;
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template<typename First, typename Second>
    const Second& get_second() const {
        const Second *r = try_get<First, Second>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get: entity does not have component (use try_get)");
        return *r;
    }


    /* try_get_mut */

    /** Get mutable component value.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, if_t< is_actual<T>::value > = 0>
    T* try_get_mut() const {
        auto comp_id = _::type<T>::id(world_);
        ecs_assert(_::type<T>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<T*>(ecs_get_mut_id(world_, id_, comp_id));
    }

    /** Get mutable component value.
     * Overload for when T is not the same as the actual type, which happens
     * when using pair types.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, typename A = actual_type_t<T>, 
        if_t< flecs::is_pair<T>::value > = 0>
    A* try_get_mut() const {
        auto comp_id = _::type<T>::id(world_);
        ecs_assert(_::type<A>::size() != 0, ECS_INVALID_PARAMETER,
            "operation invalid for empty type");
        return static_cast<A*>(ecs_get_mut_id(world_, id_, comp_id));
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template <typename First, typename Second, typename P = pair<First, Second>, 
        typename A = actual_type_t<P>, if_not_t< flecs::is_pair<First>::value > = 0>
    A* try_get_mut() const {
        return this->try_get_mut<P>();
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     */
    template<typename First, typename Second, if_not_t< is_enum<Second>::value> = 0>
    First* try_get_mut(Second second) const {
        auto first = _::type<First>::id(world_);
        ecs_assert(_::type<First>::size() != 0, ECS_INVALID_PARAMETER, 
            "operation invalid for empty type");
        return static_cast<First*>(
            ecs_get_mut_id(world_, id_, ecs_pair(first, second)));
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param constant the enum constant.
     */
    template<typename First, typename Second, if_t< is_enum<Second>::value && !std::is_same<First, Second>::value > = 0>
    First* try_get_mut(Second constant) const {
        const auto& et = enum_type<Second>(this->world_);
        flecs::entity_t target = et.entity(constant);
        return get_mut<First>(target);
    }

    /** Get mutable component value (untyped).
     * 
     * @param comp The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    void* try_get_mut(flecs::id_t comp) const {
        return ecs_get_mut_id(world_, id_, comp);
    }

    /** Get a mutable pair (untyped).
     * This operation gets the value for a pair from the entity. If neither the
     * first nor the second part of the pair are components, the operation 
     * will fail.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     */
    void* try_get_mut(flecs::entity_t first, flecs::entity_t second) const {
        return ecs_get_mut_id(world_, id_, ecs_pair(first, second));
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam Second the second element of a pair.
     * @param first The first part of the pair.
     */
    template<typename Second>
    Second* try_get_mut_second(flecs::entity_t first) const {
        auto second = _::type<Second>::id(world_);
        ecs_assert( ecs_get_type_info(world_, ecs_pair(first, second)) != NULL,
            ECS_INVALID_PARAMETER, "pair is not a component");
        ecs_assert( ecs_get_type_info(world_, ecs_pair(first, second))->component == second,
            ECS_INVALID_PARAMETER, "type of pair is not Second");
        ecs_assert(_::type<Second>::size() != 0, ECS_INVALID_PARAMETER, 
            "operation invalid for empty type");
        return static_cast<Second*>(
            ecs_get_mut_id(world_, id_, ecs_pair(first, second)));
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template<typename First, typename Second>
    Second* try_get_mut_second() const {
        return get_mut<pair_object<First, Second>>();
    }


    /* get_mut */

    /** Get mutable component value.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, if_t< is_actual<T>::value > = 0>
    T& get_mut() const {
        T* r = try_get_mut<T>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get mutable component value.
     * Overload for when T is not the same as the actual type, which happens
     * when using pair types.
     * 
     * @tparam T The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    template <typename T, typename A = actual_type_t<T>, 
        if_t< flecs::is_pair<T>::value > = 0>
    A& get_mut() const {
        A* r = try_get_mut<T>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template <typename First, typename Second, typename P = pair<First, Second>, 
        typename A = actual_type_t<P>, if_not_t< flecs::is_pair<First>::value > = 0>
    A& get_mut() const {
        A* r = try_get_mut<First, Second>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     */
    template<typename First, typename Second, if_not_t< is_enum<Second>::value> = 0>
    First& get_mut(Second second) const {
        First* r = try_get_mut<First>(second);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get a mutable pair.
     * This operation gets the value for a pair from the entity. 
     *
     * @tparam First The first element of the pair.
     * @param constant the enum constant.
     */
    template<typename First, typename Second, if_t< is_enum<Second>::value && !std::is_same<First, Second>::value > = 0>
    First& get_mut(Second constant) const {
        const auto& et = enum_type<Second>(this->world_);
        flecs::entity_t target = et.entity(constant);
        return get_mut<First>(target);
    }

    /** Get mutable component value (untyped).
     * 
     * @param comp The component to get.
     * @return Pointer to the component value, nullptr if the entity does not
     *         have the component.
     */
    void* get_mut(flecs::id_t comp) const {
        void *r = ecs_get_mut_id(world_, id_, comp);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return r;
    }

    /** Get a mutable pair (untyped).
     * This operation gets the value for a pair from the entity. If neither the
     * first nor the second part of the pair are components, the operation 
     * will fail.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     */
    void* get_mut(flecs::entity_t first, flecs::entity_t second) const {
        void *r = ecs_get_mut_id(world_, id_, ecs_pair(first, second));
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return r;
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam Second the second element of a pair.
     * @param first The first part of the pair.
     */
    template<typename Second>
    Second& get_mut_second(flecs::entity_t first) const {
        Second *r = try_get_mut_second<Second>(first);
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get the second part for a pair.
     * This operation gets the value for a pair from the entity. The first
     * part of the pair should not be a component.
     *
     * @tparam First The first element of the pair.
     * @tparam Second the second element of a pair.
     */
    template<typename First, typename Second>
    Second* get_mut_second() const {
        Second *r = try_get_mut_second<First, Second>();
        ecs_assert(r != nullptr, ECS_INVALID_OPERATION, 
            "invalid get_mut: entity does not have component (use try_get_mut)");
        return *r;
    }

    /** Get enum constant for enum relationship. */
    template<typename Enum>
    Enum get_constant() const;

    template<typename TInt>
    TInt get_constant(flecs::entity_t type_id) const;
    
    /** Get target for a given pair.
     * This operation returns the target for a given pair. The optional
     * index can be used to iterate through targets, in case the entity has
     * multiple instances for the same relationship.
     *
     * @tparam First The first element of the pair.
     * @param index The index (0 for the first instance of the relationship).
     */
    template<typename First>
    flecs::entity target(int32_t index = 0) const;

    /** Get target for a given pair.
     * This operation returns the target for a given pair. The optional
     * index can be used to iterate through targets, in case the entity has
     * multiple instances for the same relationship.
     *
     * @param first The first element of the pair for which to retrieve the target.
     * @param index The index (0 for the first instance of the relationship).
     */
    flecs::entity target(flecs::entity_t first, int32_t index = 0) const;

    /** Get the target of a pair for a given relationship id.
     * This operation returns the first entity that has the provided id by following
     * the specified relationship. If the entity itself has the id then entity will
     * be returned. If the id cannot be found on the entity or by following the
     * relationship, the operation will return 0.
     * 
     * This operation can be used to lookup, for example, which prefab is providing
     * a component by specifying the IsA pair:
     * 
     * @code
     * // Is Position provided by the entity or one of its base entities?
     * ecs_get_target_for_id(world, entity, EcsIsA, ecs_id(Position))
     * @endcode
     * 
     * @param relationship The relationship to follow.
     * @param id The id to lookup.
     * @return The entity for which the target has been found.
     */
    flecs::entity target_for(flecs::entity_t relationship, flecs::id_t id) const;

    template <typename T>
    flecs::entity target_for(flecs::entity_t relationship) const;

    template <typename First, typename Second>
    flecs::entity target_for(flecs::entity_t relationship) const;

    /** Get depth for given relationship.
     *
     * @param rel The relationship.
     * @return The depth.
     */
    int32_t depth(flecs::entity_t rel) const {
        return ecs_get_depth(world_, id_, rel);
    }

    /** Get depth for given relationship.
     *
     * @tparam Rel The relationship.
     * @return The depth.
     */
    template<typename Rel>
    int32_t depth() const {
        return this->depth(_::type<Rel>::id(world_));
    }

    /** Get parent of entity.
     * Short for target(flecs::ChildOf).
     * 
     * @return The parent of the entity.
     */
    flecs::entity parent() const;
    
    /** Lookup an entity by name.
     * Lookup an entity in the scope of this entity. The provided path may
     * contain double colons as scope separators, for example: "Foo::Bar".
     *
     * @param path The name of the entity to lookup.
     * @param search_path When false, only the entity's scope is searched.
     * @return The found entity, or entity::null if no entity matched.
     */
    flecs::entity lookup(const char *path, bool search_path = false) const;

    /** Check if entity has the provided entity.
     *
     * @param e The entity to check.
     * @return True if the entity has the provided entity, false otherwise.
     */
    bool has(flecs::id_t e) const {
        return ecs_has_id(world_, id_, e);
    }     

    /** Check if entity has the provided component.
     *
     * @tparam T The component to check.
     * @return True if the entity has the provided component, false otherwise.
     */
    template <typename T>
    bool has() const {
        flecs::id_t cid = _::type<T>::id(world_);
        bool result = ecs_has_id(world_, id_, cid);
        if (result) {
            return result;
        }

        if (is_enum<T>::value) {
            return ecs_has_pair(world_, id_, cid, flecs::Wildcard);
        }

        return false;
    }

    /** Check if entity has the provided enum constant.
     *
     * @tparam E The enum type (can be deduced).
     * @param value The enum constant to check. 
     * @return True if the entity has the provided constant, false otherwise.
     */
    template <typename E, if_t< is_enum<E>::value > = 0>
    bool has(E value) const {
        auto r = _::type<E>::id(world_);
        auto o = enum_type<E>(world_).entity(value);
        ecs_assert(o, ECS_INVALID_PARAMETER,
            "Constant was not found in Enum reflection data."
            " Did you mean to use has<E>() instead of has(E)?");
        return ecs_has_pair(world_, id_, r, o);
    }

    /** Check if entity has the provided pair.
     *
     * @tparam First The first element of the pair.
     * @tparam Second The second element of the pair.
     * @return True if the entity has the provided component, false otherwise.
     */
    template <typename First, typename Second>
    bool has() const {
        return this->has<First>(_::type<Second>::id(world_));
    }

    /** Check if entity has the provided pair.
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     * @return True if the entity has the provided component, false otherwise.
     */
    template<typename First, typename Second, if_not_t< is_enum<Second>::value > = 0>
    bool has(Second second) const {
        auto comp_id = _::type<First>::id(world_);
        return ecs_has_id(world_, id_, ecs_pair(comp_id, second));
    }

    /** Check if entity has the provided pair.
     *
     * @tparam Second The second element of the pair.
     * @param first The first element of the pair.
     * @return True if the entity has the provided component, false otherwise.
     */
    template <typename Second>
    bool has_second(flecs::entity_t first) const {
        return this->has(first, _::type<Second>::id(world_));
    }

    /** Check if entity has the provided pair.
     *
     * @tparam First The first element of the pair.
     * @param value The enum constant.
     * @return True if the entity has the provided component, false otherwise.
     */
    template<typename First, typename E, if_t< is_enum<E>::value && !std::is_same<First, E>::value > = 0>
    bool has(E value) const {
        const auto& et = enum_type<E>(this->world_);
        flecs::entity_t second = et.entity(value);
        return has<First>(second);
    }

    /** Check if entity has the provided pair.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     * @return True if the entity has the provided component, false otherwise.
     */
    bool has(flecs::id_t first, flecs::id_t second) const {
        return ecs_has_id(world_, id_, ecs_pair(first, second));
    }

    /** Check if entity owns the provided entity.
     * An entity is owned if it is not shared from a base entity.
     *
     * @param e The entity to check.
     * @return True if the entity owns the provided entity, false otherwise.
     */
    bool owns(flecs::id_t e) const {
        return ecs_owns_id(world_, id_, e);
    }

    /** Check if entity owns the provided pair.
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     * @return True if the entity owns the provided component, false otherwise.
     */
    template <typename First>
    bool owns(flecs::id_t second) const {
        auto comp_id = _::type<First>::id(world_);
        return owns(ecs_pair(comp_id, second));
    }

    /** Check if entity owns the provided pair.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     * @return True if the entity owns the provided component, false otherwise.
     */
    bool owns(flecs::id_t first, flecs::id_t second) const {
        return owns(ecs_pair(first, second));
    }

    /** Check if entity owns the provided component.
     * An component is owned if it is not shared from a base entity.
     *
     * @tparam T The component to check.
     * @return True if the entity owns the provided component, false otherwise.
     */
    template <typename T>
    bool owns() const {
        return owns(_::type<T>::id(world_));
    }

    /** Check if entity owns the provided pair.
     * An pair is owned if it is not shared from a base entity.
     *
     * @tparam First The first element of the pair.
     * @tparam Second The second element of the pair.
     * @return True if the entity owns the provided pair, false otherwise.
     */
    template <typename First, typename Second>
    bool owns() const {
        return owns(
            _::type<First>::id(world_),
            _::type<Second>::id(world_));
    }

    /** Test if id is enabled.
     *
     * @param id The id to test.
     * @return True if enabled, false if not.
     */
    bool enabled(flecs::id_t id) const {
        return ecs_is_enabled_id(world_, id_, id);
    }

    /** Test if component is enabled.
     *
     * @tparam T The component to test.
     * @return True if enabled, false if not.
     */
    template<typename T>
    bool enabled() const {
        return this->enabled(_::type<T>::id(world_));
    }

    /** Test if pair is enabled.
     *
     * @param first The first element of the pair.
     * @param second The second element of the pair.
     * @return True if enabled, false if not.
     */
    bool enabled(flecs::id_t first, flecs::id_t second) const {
        return this->enabled(ecs_pair(first, second));
    }

    /** Test if pair is enabled.
     *
     * @tparam First The first element of the pair.
     * @param second The second element of the pair.
     * @return True if enabled, false if not.
     */
    template <typename First>
    bool enabled(flecs::id_t second) const {
        return this->enabled(_::type<First>::id(world_), second);
    }

    /** Test if pair is enabled.
     *
     * @tparam First The first element of the pair.
     * @tparam Second The second element of the pair.
     * @return True if enabled, false if not.
     */
    template <typename First, typename Second>
    bool enabled() const {
        return this->enabled<First>(_::type<Second>::id(world_));
    }

    flecs::entity clone(bool clone_value = true, flecs::entity_t dst_id = 0) const;

    /** Return mutable entity handle for current stage 
     * When an entity handle created from the world is used while the world is
     * in staged mode, it will only allow for readonly operations since 
     * structural changes are not allowed on the world while in staged mode.
     * 
     * To do mutations on the entity, this operation provides a handle to the
     * entity that uses the stage instead of the actual world.
     *
     * Note that staged entity handles should never be stored persistently, in
     * components or elsewhere. An entity handle should always point to the
     * main world.
     *
     * Also note that this operation is not necessary when doing mutations on an
     * entity outside of a system. It is allowed to do entity operations 
     * directly on the world, as long as the world is not in staged mode.
     *
     * @param stage The current stage.
     * @return An entity handle that allows for mutations in the current stage.
     */
    flecs::entity mut(const flecs::world& stage) const;

    /** Same as mut(world), but for iterator.
     * This operation allows for the construction of a mutable entity handle
     * from an iterator.
     *
     * @param it An iterator that contains a reference to the world or stage.
     * @return An entity handle that allows for mutations in the current stage.
     */
    flecs::entity mut(const flecs::iter& it) const;

    /** Same as mut(world), but for entity.
     * This operation allows for the construction of a mutable entity handle
     * from another entity. This is useful in each() functions, which only 
     * provide a handle to the entity being iterated over.
     *
     * @param e Another mutable entity.
     * @return An entity handle that allows for mutations in the current stage.
     */
    flecs::entity mut(const flecs::entity_view& e) const;

#   ifdef FLECS_JSON
#   include "mixins/json/entity_view.inl"
#   endif
#   ifdef FLECS_DOC
#   include "mixins/doc/entity_view.inl"
#   endif
#   ifdef FLECS_ALERTS
#   include "mixins/alerts/entity_view.inl"
#   endif

#   include "mixins/enum/entity_view.inl"
#   include "mixins/event/entity_view.inl"

private:
    flecs::entity set_stage(world_t *stage);
};

}

/** @} */
