// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <functional>

#include "flecs.h"

#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Concepts/SolidConcepts.h"
#include "Types/SolidEnumSelector.h"
#include "Types/SolidNotNull.h"

#include "FlecsEntityHandleTypes.h"
#include "FlecsEntityView.h"
#include "FlecsId.h"
#include "Logging/StructuredLog.h"

#include "FlecsEntityHandle.generated.h"

struct FFlecsEntityHandle;

struct FFlecsCollectionReference;
struct FFlecsCollectionInstancedReference;

class UFlecsWorldInterfaceObject;

/**
 * @struct FFlecsEntityHandle
 *
 * A handle for managing flecs entities in Unreal Engine with added blueprint support.
 * The structure provides several utility functions to interact with flecs entities,
 * including validation, component addition/removal, and direct data access.
 * This must be used with a valid `UFlecsWorldInterfaceObject` instance to function correctly.
 */
USTRUCT(BlueprintType, meta = (DisableSplitPin,
	HasNativeMake = "/Script/UnrealFlecs.FlecsEntityHandleFunctionLibrary.MakeFlecsEntityHandle"))
struct alignas(8) UNREALFLECS_API FFlecsEntityHandle : public FFlecsEntityView
{
	GENERATED_BODY()

	using FSelfType = FFlecsEntityHandle;

public:

	static NO_DISCARD SOLID_INLINE FFlecsEntityHandle GetNullHandle()
	{
		return FFlecsEntityHandle(flecs::entity::null());
	}

	static NO_DISCARD SOLID_INLINE FFlecsEntityHandle Invalid()
	{
		return FFlecsEntityHandle(flecs::entity::null());
	}

	 static NO_DISCARD FFlecsEntityHandle GetNullHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);

public:
	using FFlecsEntityView::FFlecsEntityView;
	
	FFlecsEntityHandle() = default;
	
	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Add(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().add(FFlecsCommonHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& Add(this const TSelf& InSelf, const UEnum* EnumType, const uint64 InValue)
	{
		const FFlecsEntityHandle EnumEntity = InSelf.template ObtainComponentTypeEnum<FFlecsEntityHandle>(EnumType);

		const FFlecsId ValueEntity = InSelf. template GetEnumConstant<FFlecsId>(EnumType, InValue);
		solid_check(ValueEntity.IsValid());
		
		InSelf.AddPair(EnumEntity, ValueEntity);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Add(this const TSelf& InSelf, const FGameplayTagContainer& InTags)
	{
		for (const FGameplayTag& Tag : InTags)
		{
			InSelf.Add(Tag);
		}

		return InSelf;
	}
	
	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& Add(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template add<T>();
		return InSelf;
	}

	template <typename T, typename TSelf>
	requires (std::is_enum<T>::value)
	SOLID_INLINE const TSelf& Add(this const TSelf& InSelf, const T InValue)
	{
		InSelf.GetEntity().template add<T>(InValue);
		return InSelf;
	}
	
	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template remove<T>();
		return InSelf;
	}

	template <typename T, typename TSelf>
	requires (std::is_enum_v<T>)
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf)
	{
		InSelf.template RemovePair<T>(flecs::Wildcard);
		return InSelf;
	}
	
	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().remove(FFlecsEntityHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf, const UEnum* EnumType)
	{
		InSelf.RemovePair(FFlecsEntityHandle::GetInputId(InSelf, EnumType), flecs::Wildcard);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf, const UEnum* EnumType, const int64 InValue)
	{
		const FFlecsEntityHandle EnumEntity = InSelf.template ObtainComponentTypeEnum<FFlecsEntityHandle>(EnumType);
		solid_check(EnumEntity.IsValid());
		solid_check(EnumEntity.IsEnum());

		const FFlecsId EnumConstant = InSelf.template GetEnumConstant<FFlecsId>(EnumType, InValue);
		
		InSelf.RemovePair(EnumEntity, EnumConstant);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf, const FSolidEnumSelector& EnumSelector)
	{
		return InSelf.Remove(EnumSelector.Class, EnumSelector.Value);
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Remove(this const TSelf& InSelf, const FGameplayTagContainer& InTags)
	{
		for (const FGameplayTag& Tag : InTags)
		{
			if (!InSelf.Has(Tag))
			{
				continue;
			}
			
			InSelf.Remove(Tag);
		}

		return InSelf;
	}
	
	template <typename T, typename TSelf>
	requires (!std::is_same_v<std::decay_t<T>, FInstancedStruct>)
	SOLID_INLINE const TSelf& Set(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().template set<T>(InValue);
		return InSelf;
	}

	template <typename T, typename TSelf>
	requires (std::is_move_constructible_v<T> && !std::is_lvalue_reference_v<T>)
	SOLID_INLINE const TSelf& Set(this const TSelf& InSelf, T&& InValue)  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		InSelf.GetEntity().set(FLECS_FWD(InValue));
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& Set(this const TSelf& InSelf, const FFlecsId InId, const uint32 InSize, const void* InValue)
	{
		InSelf.GetEntity().set_ptr(InId, InSize, InValue);
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputDataTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Set(this const TSelf& InSelf, const T& InTypeValue, const void* InData)
	{
		const FFlecsId InId = FFlecsEntityHandle::GetInputId(InSelf, InTypeValue);

		if constexpr (std::is_convertible_v<T, const UScriptStruct*>)
		{
			InSelf.Set(InId, InTypeValue->GetStructureSize(), InData);
		}
		else
		{
			InSelf.GetEntity().set_ptr(InId, InData);
		}
		
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Set(this const TSelf& InSelf, const FInstancedStruct& InValue)
	{
		InSelf.Set(FFlecsEntityHandle::GetInputId(InSelf, InValue.GetScriptStruct()),
			InValue.GetScriptStruct()->GetStructureSize(),
			InValue.GetMemory());
		return InSelf;
	}
	
	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& Assign(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().template assign<T>(InValue);
		return InSelf;
	}

	template <typename T, typename TSelf>
	requires (std::is_move_constructible_v<T> && !std::is_lvalue_reference_v<T>)
	SOLID_INLINE const TSelf& Assign(this const TSelf& InSelf, T&& InValue)
	{
		InSelf.GetEntity().template assign<T>(FLECS_FWD(InValue));
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& Assign(this const TSelf& InSelf, const FFlecsId InEntity, const uint32 InSize, const void* InValue)
	{
		InSelf.GetEntity().set_ptr(InEntity, InSize, InValue);
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputDataTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Assign(this const TSelf& InSelf, const T& InTypeValue, const void* InValue)
	{
		const FFlecsId InId = FFlecsEntityHandle::GetInputId(InSelf, InTypeValue);
		
		if constexpr (std::is_convertible_v<T, const UScriptStruct*>)
		{
			InSelf.Assign(InId, InTypeValue->GetStructureSize(), InValue);
		}
		else
		{
			InSelf.GetEntity().set_ptr(InId, InValue);
		}
		
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Assign(this const TSelf& InSelf, const FInstancedStruct& InValue)
	{
		InSelf.Assign(InValue.GetScriptStruct(), InValue.GetMemory());
		return InSelf;
	}

	template <typename TFunction, typename TSelf>
	requires (flecs::is_callable<TFunction>::value)
	SOLID_INLINE const TSelf& Insert(this const TSelf& InSelf, const TFunction& InFunction)
	{
		InSelf.GetEntity().insert(InFunction);
		return InSelf;
	}

	template <typename T, typename ... Args, typename TActual = flecs::actual_type_t<T>, typename TSelf>
	SOLID_INLINE const TSelf& Emplace(this const TSelf& InSelf, Args&& ... InArgs)
	{
		InSelf.GetEntity().template emplace<T>(std::forward<Args>(InArgs)...);
		return InSelf;
	}

	template <typename TFirst, typename TSecond, typename ... Args, typename TActual = flecs::pair<TFirst, TSecond>, typename TSelf>
	requires (std::is_same<TFirst, TActual>::value)
	SOLID_INLINE const TSelf& EmplaceFirst(this const TSelf& InSelf, Args&& ... InArgs)
	{
		InSelf.GetEntity().template emplace<TFirst, TSecond>(std::forward<Args>(InArgs)...);
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename ... Args, typename TSelf>
	SOLID_INLINE const TSelf& EmplaceFirst(this const TSelf& InSelf, const TSecond& InSecondType, Args&& ... InArgs)
	{
		InSelf.GetEntity().template emplace_first<TFirst>(FFlecsEntityHandle::GetInputId(InSelf, InSecondType), std::forward<Args>(InArgs)...);
		return InSelf;
	}

	template <typename TFirst, typename TSecond, typename ... Args, typename TActual = flecs::pair<TFirst, TSecond>, typename TSelf>
	requires (std::is_same<TSecond, TActual>::value)
	SOLID_INLINE const TSelf& EmplaceSecond(this const TSelf& InSelf, Args&& ... InArgs)
	{
		InSelf.GetEntity().template emplace<TFirst, TSecond>(std::forward<Args>(InArgs)...);
		return InSelf;
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst, typename ... Args, typename TSelf>
	SOLID_INLINE const TSelf& EmplaceSecond(this const TSelf& InSelf, const TFirst& InFirstType, Args&& ... InArgs)
	{
		InSelf.GetEntity().template emplace_second<TSecond>(FFlecsEntityHandle::GetInputId(InSelf, InFirstType), std::forward<Args>(InArgs)...);
		return InSelf;
	}

	template <typename T>
	NO_DISCARD SOLID_INLINE flecs::ref<T> GetFlecsRef() const
	{
		solid_checkf(Has<T>(),
			TEXT("Entity does not have component with type %hs"), nameof(T).data());
		
		return GetEntity().get_ref<T>();
	}

	template <UE::Flecs::TFlecsEntityFunctionInputDataTypeConcept T>
	NO_DISCARD SOLID_INLINE flecs::untyped_ref GetFlecsRef(const T& InTypeValue) const
	{
		return GetEntity().get_ref(FFlecsEntityHandle::GetInputId(*this, InTypeValue));
	}

	SOLID_INLINE void Clear() const
	{
		GetEntity().clear();
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& Enable(this const TSelf& InSelf)
	{
		InSelf.GetEntity().enable();
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& Disable(this const TSelf& InSelf)
	{
		InSelf.GetEntity().disable();
		return InSelf;
	}

	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& Enable(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template enable<T>();
		return InSelf;
	}
	
	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Enable(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().enable(FFlecsEntityHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}
	
	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& Disable(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template disable<T>();
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& Disable(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().disable(FFlecsEntityHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}

	SOLID_INLINE bool Toggle() const
	{
		IsEnabled() ? Disable() : Enable();
		return IsEnabled();
	}

	template <typename T>
	SOLID_INLINE bool Toggle() const
	{
		IsEnabled<T>() ? Disable<T>() : Enable<T>();
		return IsEnabled<T>();
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T>
	SOLID_INLINE bool Toggle(const T& InValue) const
	{
		GetEntity().enable(FFlecsEntityHandle::GetInputId(*this, InValue), !IsEnabled(InValue));
		return IsEnabled(InValue);
	}

	/**
	 * @brief Delete an entity.
	 * Entities have to be deleted explicitly, and are not deleted when the
	 * entity object goes out of scope.
	 *
	 * @see ecs_delete()
	 */
	SOLID_INLINE void Destroy() const
	{
		GetEntity().destruct();
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetName(this const TSelf& InSelf, const FString& InName)
	{
		InSelf.GetEntity().set_name(StringCast<char>(*InName).Get());
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetName(this const TSelf& InSelf, const FAnsiStringView InName)
	{
		InSelf.GetEntity().set_name(InName.GetData());
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& ClearName(this const TSelf& InSelf)
	{
		InSelf.GetEntity().set_name(nullptr);
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& SetAlias(this const TSelf& InSelf, const FString& InAlias)
	{
		InSelf.GetEntity().set_alias(StringCast<char>(*InAlias).Get());
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& ClearAlias(this const TSelf& InSelf)
	{
		InSelf.GetEntity().set_alias(nullptr);
		return InSelf;
	}


#if defined(FLECS_DOC)
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& SetDocBrief(this const TSelf& InSelf, const FString& InDocBrief)
	{
		InSelf.GetEntity().set_doc_brief(StringCast<char>(*InDocBrief).Get());
		return InSelf;
	}

	// @TODO: make a variation for passing in an unreal color type?
	template <typename TSelf>
	SOLID_INLINE const TSelf& SetDocColor(this const TSelf& InSelf, const FString& Link)
	{
		InSelf.GetEntity().set_doc_color(StringCast<char>(*Link).Get());
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetDocName(this const TSelf& InSelf, const FString& InDocName)
	{
		InSelf.GetEntity().set_doc_name(StringCast<char>(*InDocName).Get());
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetDocLink(this const TSelf& InSelf, const FString& InDocLink)
	{
		InSelf.GetEntity().set_doc_link(StringCast<char>(*InDocLink).Get());
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetDocDetails(this const TSelf& InSelf, const FString& InDocDetails)
	{
		InSelf.GetEntity().set_doc_detail(StringCast<char>(*InDocDetails).Get());
		return InSelf;
	}

#endif // #if defined(FLECS_DOC)
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& SetChildOf(this const TSelf& InSelf, const FFlecsId InParent)
	{
		solid_checkf(!InSelf.template Has<flecs::Parent>(),
			TEXT("Entity already has an exclusive parent. Use SetParent to change the parent or change the existing parent component to a ChildOf"));
		InSelf.GetEntity().child_of(InParent);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetParent(this const TSelf& InSelf, const FFlecsId InParent)
	{
		//solid_checkf(!HasPair(flecs::ChildOf, flecs::Wildcard), TEXT("Entity already has a ChildOf relationship."));
		
		InSelf.Set(flecs::Parent{InParent});
		return InSelf;
	}

	NO_DISCARD SOLID_INLINE flecs::untyped_component GetUntypedComponent() const
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
		return flecs::untyped_component(GetNativeFlecsWorld(), GetEntity());
	}

	// Does not check if the entity is a component
	NO_DISCARD SOLID_INLINE flecs::untyped_component GetUntypedComponent_Unsafe() const
	{
		return flecs::untyped_component(GetNativeFlecsWorld(), GetEntity());
	}

	template <typename T>
	SOLID_INLINE void Emit() const
	{
		GetEntity().emit<T>();
	}

	template <typename T>
	SOLID_INLINE void Emit(const T& InValue) const
	{
		GetEntity().emit<T>(InValue);
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T>
	SOLID_INLINE void Emit(const T& InValue) const
	{
		GetEntity().emit(FFlecsEntityHandle::GetInputId(*this, InValue));
	}

	template <typename T>
	SOLID_INLINE void Enqueue() const
	{
		GetEntity().enqueue<T>();
	}

	template <typename T>
	SOLID_INLINE void Enqueue(const T& InValue) const
	{
		GetEntity().enqueue<T>(InValue);
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T>
	SOLID_INLINE void Enqueue(const T& InValue) const
	{
		GetEntity().enqueue(FFlecsEntityHandle::GetInputId(*this, InValue));
	}

	template <typename TEvent, typename FunctionType, typename TSelf>
	SOLID_INLINE const TSelf& Observe(this const TSelf& InSelf, FunctionType&& InFunction)
	{
		InSelf.GetEntity().template observe<TEvent>(std::forward<FunctionType>(InFunction));
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename FunctionType, typename TSelf>
	SOLID_INLINE const TSelf& Observe(this const TSelf& InSelf, const T& InValue, FunctionType&& InFunction)
	{
		InSelf.GetEntity().observe(FFlecsEntityHandle::GetInputId(InSelf, InValue), std::forward<FunctionType>(InFunction));
		return InSelf;
	}
	
	SOLID_INLINE flecs::entity operator->() const
	{
		return GetEntity();
	}

	SOLID_INLINE void ResetHandle()
	{
		Entity = flecs::entity::null();
	}

	SOLID_INLINE FFlecsEntityHandle& operator=(TYPE_OF_NULLPTR)
	{
		ResetHandle();
		return *this;
	}

	SOLID_INLINE FString FromJson(const FString& InJson) const
	{
		return GetEntity().from_json(StringCast<char>(*InJson).Get());
	}

	template <typename TFirst, typename TSecond, typename TSelf>
	SOLID_INLINE const TSelf& AddPair(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template add<TFirst, TSecond>();
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& AddPair(this const TSelf& InSelf, const TSecond& InSecond)
	{
		InSelf.GetEntity().template add<TFirst>(FFlecsEntityHandle::GetInputId(InSelf, InSecond));
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst,
		UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& AddPair(this const TSelf& InSelf, const TFirst& InFirst, const TSecond& InSecond)
	{
		InSelf.GetEntity().add(FFlecsEntityHandle::GetInputId(InSelf, InFirst),
			FFlecsEntityHandle::GetInputId(InSelf, InSecond));
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& AddPair(this const TSelf& InSelf, const FFlecsId InFirst, UEnum* InSecond, const int64 InValue)
	{
		const FFlecsEntityHandle EnumEntity = InSelf.template ObtainComponentTypeEnum<FFlecsEntityHandle>(InSecond);
		solid_check(EnumEntity.IsValid());
		solid_check(EnumEntity.IsEnum());

		const FFlecsId EnumConstant = InSelf.template GetEnumConstant<FFlecsId>(InSecond, InValue);
		
		InSelf.AddPair(InFirst, EnumConstant);
		return InSelf;
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst, typename TSelf>
	SOLID_INLINE const TSelf& AddPairSecond(this const TSelf& InSelf, const TFirst& InFirst)
	{
		InSelf.GetEntity().template add_second<TSecond>(FFlecsEntityHandle::GetInputId(InSelf, InFirst));
		return InSelf;
	}

	template <typename First, typename Second, typename TSelf>
	SOLID_INLINE const TSelf& RemovePair(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template remove<First, Second>();
		return InSelf;
	}

	template <typename First, UE::Flecs::TFlecsEntityFunctionInputTypeConcept Second, typename TSelf>
	SOLID_INLINE const TSelf& RemovePair(this const TSelf& InSelf, const Second& InSecond)
	{
		InSelf.GetEntity().template remove<First>(FFlecsEntityHandle::GetInputId(InSelf, InSecond));
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept First,
		UE::Flecs::TFlecsEntityFunctionInputTypeConcept Second, typename TSelf>
	SOLID_INLINE const TSelf& RemovePair(this const TSelf& InSelf, const First& InFirst, const Second& InSecond)
	{
		InSelf.GetEntity().remove(FFlecsEntityHandle::GetInputId(InSelf, InFirst),
			FFlecsEntityHandle::GetInputId(InSelf, InSecond));
		return InSelf;
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst, typename TSelf>
	SOLID_INLINE const TSelf& RemovePairSecond(this const TSelf& InSelf, const TFirst& InFirst)
	{
		InSelf.GetEntity().template remove_second<TSecond>(FFlecsEntityHandle::GetInputId(InSelf, InFirst));
		return InSelf;
	}

	// @TODO: add r-value set apis for pairs
	
	template <typename TFirst, typename TSecond, typename TActual = typename flecs::pair<TFirst, TSecond>::type, typename TSelf>
	SOLID_INLINE const TSelf& SetPair(this const TSelf& InSelf, const TActual& InValue)
	{
		InSelf.GetEntity().template set<TFirst, TSecond>(InValue);
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& SetPair(this const TSelf& InSelf, const TSecond& InSecondType, const TFirst& InValue)
	{
		InSelf.GetEntity().template set<TFirst>(FFlecsEntityHandle::GetInputId(InSelf, InSecondType), InValue);
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& SetPair(this const TSelf& InSelf, const TSecond& InSecondType, const void* InValue)
	{
		// @TODO: check for Type being registered
		
		InSelf.Set(FFlecsId::MakePair(flecs::_::type<TFirst>::id(InSelf.GetNativeFlecsWorld()), FFlecsEntityHandle::GetInputId(InSelf, InSecondType)),
				InValue);
			
		return InSelf;
	}

	// @TODO: handle PairIsTag
	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept First,
		UE::Flecs::TFlecsEntityFunctionInputTypeConcept Second, typename TSelf>
	SOLID_INLINE const TSelf& SetPair(this const TSelf& InSelf, const First& InFirstTypeValue, const void* InValue, const Second& InSecondTypeValue)
	{
		InSelf.Set(FFlecsId::MakePair(
			FFlecsEntityHandle::GetInputId(InSelf, InFirstTypeValue),
			FFlecsEntityHandle::GetInputId(InSelf, InSecondTypeValue)),
				InValue);
		
		return InSelf;
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept First, typename TActual = TSecond, typename TSelf>
	SOLID_INLINE const TSelf& SetPairSecond(this const TSelf& InSelf, const First& InFirstType, const TActual& InValue)
	{
		InSelf.GetEntity().template set_second<TSecond>(FFlecsEntityHandle::GetInputId(InSelf, InFirstType), InValue);
		return InSelf;
	}

	template <typename TFirst, typename TSecond, typename TActual = flecs::pair<TFirst, TSecond>::type, typename TSelf>
	SOLID_INLINE const TSelf& AssignPair(this const TSelf& InSelf, const TActual& InValue)
	{
		InSelf.GetEntity().template assign<TFirst, TSecond>(InValue);
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& AssignPair(this const TSelf& InSelf, const TSecond& InSecondType, const TFirst& InValue)
	{
		InSelf.GetEntity().template assign<TFirst>(FFlecsEntityHandle::GetInputId(InSelf, InSecondType), InValue);
		return InSelf;
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond, typename TSelf>
	SOLID_INLINE const TSelf& AssignPair(this const TSelf& InSelf, const TSecond& InSecondType, const void* InValue)
	{
		solid_checkf(InSelf.template HasPair<TFirst>(InSecondType),
			TEXT("Entity does not have pair"));

		InSelf.Assign(FFlecsId::MakePair(flecs::_::type<TFirst>::id(InSelf.GetNativeFlecsWorld()),
			FFlecsEntityHandle::GetInputId(InSelf, InSecondType)),
				InValue);
		
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept First,
		UE::Flecs::TFlecsEntityFunctionInputTypeConcept Second, typename TSelf>
	SOLID_INLINE const TSelf& AssignPair(this const TSelf& InSelf, const First& InFirstTypeValue, const void* InValue, const Second& InSecondTypeValue)
	{
		InSelf.Assign(FFlecsId::MakePair(
			FFlecsEntityHandle::GetInputId(InSelf, InFirstTypeValue),
			FFlecsEntityHandle::GetInputId(InSelf, InSecondTypeValue)),
				InValue);
		return InSelf;
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst, typename TSelf>
	SOLID_INLINE const TSelf& AssignPairSecond(this const TSelf& InSelf, const TFirst& InFirstType, const TSecond& InValue)
	{
		InSelf.GetEntity().template assign_second<TSecond>(FFlecsEntityHandle::GetInputId(InSelf, InFirstType), InValue);
		return InSelf;
	}
	
	template <typename TFirst, typename TSecond>
	SOLID_INLINE void ModifiedPair() const
	{
		GetEntity().modified<TFirst, TSecond>();
	}

	template <typename TFirst, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond>
	SOLID_INLINE void ModifiedPair(const TSecond& InSecondTypeValue) const
	{
		GetEntity().modified<TFirst>(FFlecsEntityHandle::GetInputId(*this, InSecondTypeValue));
	}
	
	template <typename TComponent>
	SOLID_INLINE void Modified() const
	{
		GetEntity().modified<TComponent>();
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept TComponent>
	SOLID_INLINE void Modified(const TComponent& InValueValue) const
	{
		GetEntity().modified(FFlecsEntityHandle::GetInputId(*this, InValueValue));
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst,
		UE::Flecs::TFlecsEntityFunctionInputTypeConcept TSecond>
	SOLID_INLINE void ModifiedPair(const TFirst& InFirstTypeValue, const TSecond& InSecondTypeValue) const
	{
		GetEntity().modified(FFlecsEntityHandle::GetInputId(*this, InFirstTypeValue),
			FFlecsEntityHandle::GetInputId(*this, InSecondTypeValue));
	}

	template <typename TSecond, UE::Flecs::TFlecsEntityFunctionInputTypeConcept TFirst>
	SOLID_INLINE void ModifiedPairSecond(const TFirst& InFirstTypeValue) const
	{
		GetEntity().modified<TSecond>(FFlecsEntityHandle::GetInputId(*this, InFirstTypeValue));
	}

	template <typename T>
	NO_DISCARD SOLID_INLINE T& Obtain() const
	{
		return GetEntity().obtain<T>();
	}

	template <UE::Flecs::TFlecsEntityFunctionInputDataTypeConcept T>
	NO_DISCARD SOLID_INLINE void* Obtain(const T& InTypeValue) const
	{
		return GetEntity().obtain(FFlecsEntityHandle::GetInputId(*this, InTypeValue));
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& AddPrefab(this const TSelf& InSelf, const FFlecsId InPrefab)
	{
		InSelf.GetEntity().is_a(InPrefab);
		return InSelf;
	}

	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& AddPrefab(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template is_a<T>();
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& RemovePrefab(this const TSelf& InSelf, const FFlecsId InPrefab)
	{
		InSelf.RemovePair(flecs::IsA, InPrefab);
		return InSelf;
	}

	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& RemovePrefab(this const TSelf& InSelf)
	{
		InSelf.template RemovePairSecond<T>(flecs::IsA);
		return InSelf;
	}
	
	template <typename TSelf>
	SOLID_INLINE const TSelf& SetIsA(this const TSelf& InSelf, const FFlecsId InPrefab)
	{
		InSelf.GetEntity().is_a(InPrefab);
		return InSelf;
	}

	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& SetIsA(this const TSelf& InSelf)
	{
		InSelf.GetEntity().template is_a<T>();
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& SetIsA(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.GetEntity().is_a(FFlecsEntityHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}

	template <typename T, typename TSelf>
	SOLID_INLINE const TSelf& AddWith(this const TSelf& InSelf)
	{
		InSelf.template AddPairSecond<T>(flecs::With);
		return InSelf;
	}

	template <UE::Flecs::TFlecsEntityFunctionInputTypeConcept T, typename TSelf>
	SOLID_INLINE const TSelf& AddWith(this const TSelf& InSelf, const T& InValue)
	{
		InSelf.AddPair(flecs::With, FFlecsEntityHandle::GetInputId(InSelf, InValue));
		return InSelf;
	}

	template <typename TFunction, typename TSelf>
	SOLID_INLINE const TSelf& Scope(this const TSelf& InSelf, const TFunction& InFunction)
	{
		InSelf.GetEntity().scope(InFunction);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetChildOrder(this const TSelf& InSelf, FFlecsId* InOrderArray, const int32 InOrderCount)
	{
		solid_cassumef(InOrderArray != nullptr || InOrderCount == 0,
			TEXT("InOrder cannot be null if InOrderCount is greater than zero"));
		solid_cassumef(InOrderCount >= 0, TEXT("InOrderCount cannot be negative"));
		
		InSelf.GetEntity().set_child_order(reinterpret_cast<flecs::id_t*>(InOrderArray), InOrderCount);
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& SetChildOrder(this const TSelf& InSelf, const TArrayView<FFlecsId> InOrderArray)
	{
		return InSelf.SetChildOrder(InOrderArray.GetData(), InOrderArray.Num());
	}

	NO_DISCARD SOLID_INLINE FFlecsEntityView ToView() const
	{
		return FFlecsEntityView(GetEntity().view());
	}

	const FSelfType& AddCollection(const FFlecsId InCollection, const FInstancedStruct& InParams = FInstancedStruct()) const;

	template <Solid::TScriptStructConcept TCollectionParams, typename TSelf>
	SOLID_INLINE const TSelf& AddCollection(this const TSelf& InSelf, const FFlecsId InCollection, const TCollectionParams& InParams)
	{
		InSelf.AddCollection(InCollection, FInstancedStruct::Make<TCollectionParams>(InParams));
		return InSelf;
	}

	template <typename TSelf>
	SOLID_INLINE const TSelf& AddCollection(this const TSelf& InSelf, UClass* InCollection, const FInstancedStruct& InParams = FInstancedStruct())
	{
		InSelf.AddCollection(InSelf.ObtainTypeClass(InCollection), InParams);
		return InSelf;
	}

	template <Solid::TScriptStructConcept TCollectionParams, typename TSelf>
	SOLID_INLINE const TSelf& AddCollection(this const TSelf& InSelf, UClass* InCollection, const TCollectionParams& InParams)
	{
		InSelf.AddCollection(InCollection, FInstancedStruct::Make<TCollectionParams>(InParams));
		return InSelf;
	}

	template <Solid::TStaticClassConcept T, typename TSelf>
	SOLID_INLINE const TSelf& AddCollection(this const TSelf& InSelf, const FInstancedStruct& InParams = FInstancedStruct())
	{
		InSelf.AddCollection(T::StaticClass(), InParams);
		return InSelf;
	}

	template <Solid::TStaticClassConcept T, Solid::TScriptStructConcept TCollectionParams, typename TSelf>
	SOLID_INLINE const TSelf& AddCollection(this const TSelf& InSelf, const TCollectionParams& InParams)
	{
		InSelf.AddCollection(T::StaticClass(), FInstancedStruct::Make<TCollectionParams>(InParams));
		return InSelf;
	}

	const FSelfType& AddCollection(const FFlecsCollectionReference& InCollectionRef, const FInstancedStruct& InParams = FInstancedStruct()) const;
	const FSelfType& AddCollection(const FFlecsCollectionInstancedReference& InCollectionRef) const;

	// Note this doesnt remove overridden components
	const FSelfType& RemoveCollection(const FFlecsId InCollection) const;

	// Note this doesnt remove overridden components
	template <typename TSelf>
	SOLID_INLINE const TSelf& RemoveCollection(this const TSelf& InSelf, UClass* InCollection)
	{
		InSelf.RemoveCollection(InSelf.ObtainTypeClass(InCollection));
		return InSelf;
	}

	// Note this doesnt remove overridden components
	template <Solid::TStaticClassConcept T, typename TSelf>
	SOLID_INLINE const TSelf& RemoveCollection(this const TSelf& InSelf)
	{
		InSelf.RemoveCollection(T::StaticClass());
		return InSelf;
	}
	
protected:
	
}; // struct FFlecsEntityHandle

template <>
struct TStructOpsTypeTraits<FFlecsEntityHandle> : public TStructOpsTypeTraitsBase2<FFlecsEntityHandle>
{
	enum
	{
		WithIdenticalViaEquality = true,
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsEntityHandle>
