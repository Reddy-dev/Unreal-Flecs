// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"


#include "UObject/Interface.h"

#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"

#include "FlecsIteratorObjectInterface.generated.h"

class UFlecsWorld;

// This class does not need to be modified.
UINTERFACE(meta = (CannotImplementInterfaceInBlueprint))
class UFlecsIteratorObjectInterface : public UInterface
{
	GENERATED_BODY()
}; // class UFlecsIteratorObjectInterface

/**
 * Callback adapter used by Unreal-Flecs systems and observers.
 *
 * The default implementation dispatches callbacks in three levels:
 *
 * 1. RunIterator receives the unadvanced iterator and calls next().
 * 2. RunEachIterator is called once for each matched iterator result, which is
 *    typically a table or table slice.
 * 3. EachIterator is called once for each row in the current result.
 *
 * Override the level that matches the required granularity. In particular,
 * EachIterator implementations must process only InIndex and must not call
 * next() or iterate InIterator again.
 *
 * The iterator, field pointers, and references obtained from it are borrowed
 * Flecs storage. Do not retain them after the current callback. Structural
 * changes may invalidate component addresses and are normally deferred while
 * a system is running.
 */
class UNREALFLECS_API IFlecsIteratorObjectInterface
{
	GENERATED_BODY()

	// IF YOU IMPLEMENT THESE RIGHT, THEY SHOULD BE ABLE TO BE DEVIRTUALIZED
	
public:
	/**
	 * Receives the unadvanced iterator for one system or observer invocation.
	 *
	 * The default implementation advances the iterator with next() and forwards
	 * every matched result to RunEachIterator. Override this callback only when
	 * direct control over iterator advancement or whole-query processing is
	 * required. An override is responsible for consuming the iterator.
	 *
	 * @param InWorld World or worker-stage interface associated with InIterator.
	 * @param InIterator Unadvanced iterator, valid only for this callback.
	 */
	virtual void RunIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator);

	/**
	 * Receives one already-advanced iterator result.
	 *
	 * The default implementation iterates the rows in the current result and
	 * forwards each row index to EachIterator. Do not call next() here.
	 * Override this callback for table/batch-oriented processing.
	 *
	 * Query-term order determines field indices: the first term is field 0, the
	 * second is field 1, and so on. Only terms that provide data can be accessed
	 * as fields. Check InIterator.is_set(FieldIndex) before accessing an optional
	 * field.
	 *
	 * @param InWorld World or worker-stage interface associated with InIterator.
	 * @param InIterator Iterator positioned at the current matched result.
	 */
	virtual void RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator);

	/**
	 * Receives one row from the current iterator result.
	 *
	 * InIndex is a zero-based row index, despite being represented by FFlecsId;
	 * it is not the entity ID. Use it with field_at(FieldIndex, InIndex), or call
	 * InIterator.entity(InIndex) when an entity handle is required. Process only
	 * this row: do not loop InIterator and do not call next().
	 *
	 * Query-term order determines FieldIndex. For an optional term, call
	 * InIterator.is_set(FieldIndex) before field_at. References and pointers
	 * returned by field_at are valid only while processing the current iterator
	 * result and must not be retained after the callback.
	 *
	 * @code
	 * // BuildSystem appends Position as term 0 and optional Velocity as term 1.
	 * InBuilder.With<const FPosition>() ( can also use InBuilder.With<FPosition>().In() )
	 *     .With<FVelocity>().Optional();
	 *
	 * const FPosition& Position = InIterator.field_at<const FPosition>(0, InIndex);
	 * if (InIterator.is_set(1))
	 * {
	 *     FVelocity& Velocity = InIterator.field_at<FVelocity>(1, InIndex);
	 * }
	 * @endcode
	 *
	 * @param InWorld World or worker-stage interface associated with InIterator.
	 * @param InIterator Iterator positioned at the current matched result.
	 * @param InIndex Zero-based row index within the current result.
	 */
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex);

}; // class IFlecsIteratorObjectInterface
