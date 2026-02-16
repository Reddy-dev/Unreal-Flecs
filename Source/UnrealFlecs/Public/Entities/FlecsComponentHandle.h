// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsEntityHandle.h"
#include "FlecsMemberHandle.h"

#include "FlecsComponentHandle.generated.h"



/**
 *	@struct FFlecsComponentHandle
 *
 * @brief Equivalent to flecs::untyped_component and flecs::component<T>
 */
USTRUCT(BlueprintType, meta = (DisableSplitPin))
struct alignas(8) UNREALFLECS_API FFlecsComponentHandle : public FFlecsEntityHandle
{
	GENERATED_BODY()

	using FSelfType = FFlecsComponentHandle;

public:
	FFlecsComponentHandle() = default;

	SOLID_INLINE FFlecsComponentHandle(const flecs::entity& InEntity)
		: FFlecsEntityHandle(InEntity)
	{
	}

	SOLID_INLINE FFlecsComponentHandle(const FFlecsEntityHandle& InEntityHandle)
		: FFlecsEntityHandle(InEntityHandle)
	{
	}

	SOLID_INLINE FFlecsComponentHandle(flecs::world& InWorld, const FFlecsId& InId)
		: FFlecsEntityHandle(InWorld, InId)
	{
	}

	SOLID_INLINE FFlecsComponentHandle(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsId& InId)
		: FFlecsEntityHandle(InWorld, InId)
	{
	}

	SOLID_INLINE FFlecsComponentHandle operator=(const flecs::entity& InEntity)
	{
		FFlecsEntityHandle::operator=(InEntity);

		return *this;
	}

	SOLID_INLINE FFlecsComponentHandle operator=(const FFlecsEntityHandle& InEntityHandle)
	{
		FFlecsEntityHandle::operator=(InEntityHandle);

		return *this;
	}
	
	template <typename T>
	NO_DISCARD SOLID_INLINE flecs::component<T> GetTypedComponent() const
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
		return flecs::component<T>(GetUntypedComponent());
	}

	NO_DISCARD SOLID_INLINE flecs::untyped_component GetUntypedComponent() const
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
		return GetUntypedComponent_Unsafe();
	}

	SOLID_INLINE operator flecs::untyped_component() const
	{
		return GetUntypedComponent();
	}

	template <typename T>
	SOLID_INLINE operator flecs::component<T>() const
	{
		return GetTypedComponent<T>();
	}

	NO_DISCARD SOLID_INLINE flecs::Component GetComponentData() const
	{
		return Get<flecs::Component>();
	}

	SOLID_INLINE const FSelfType& SetComponentData(const flecs::Component& InComponentData) const
	{
		Set<flecs::Component>(InComponentData);
		return *this;
	}

	SOLID_INLINE const FSelfType& SetComponentData(const int32 InSize, const int32 InAlignment) const
	{
		solid_check(InSize > 0 && InAlignment > 0);
		
		SetComponentData(flecs::Component{ .size = InSize, .alignment = InAlignment });
		return *this;
	}

	SOLID_INLINE void SetHooks(flecs::type_hooks_t& InHooks) const
	{
		GetUntypedComponent().set_hooks(InHooks);
	}
	
	SOLID_INLINE void ModifyHooksLambda(const TFunctionRef<void(flecs::type_hooks_t&)>& InHooksFunction) const
	{
		flecs::type_hooks_t Hooks = GetHooks();
		Invoke(InHooksFunction, Hooks);
		GetUntypedComponent().set_hooks(Hooks);
	}

	NO_DISCARD SOLID_INLINE flecs::type_hooks_t GetHooks() const
	{
		return GetUntypedComponent().get_hooks();
	}
	
	SOLID_INLINE const FSelfType& SetConstructor(const ecs_xtor_t& InConstructor) const
	{
		ModifyHooksLambda([InConstructor](flecs::type_hooks_t& Hooks)
		{
			Hooks.ctor = InConstructor;
		});
		
		return *this;
	}

	SOLID_INLINE const FSelfType& SetDestructor(const ecs_xtor_t& InDestructor) const
	{
		ModifyHooksLambda([InDestructor](flecs::type_hooks_t& Hooks)
		{
			Hooks.dtor = InDestructor;
		});
		
		return *this;
	}

	SOLID_INLINE const FSelfType& SetCopy(const ecs_copy_t& InCopy) const
	{
		ModifyHooksLambda([InCopy](flecs::type_hooks_t& Hooks)
		{
			Hooks.copy = InCopy;
		});
		
		return *this;
	}

	SOLID_INLINE const FSelfType& SetMove(const ecs_move_t& InMove) const
	{
		ModifyHooksLambda([InMove](flecs::type_hooks_t& Hooks)
		{
			Hooks.move = InMove;
		});
		
		return *this;
	}

	SOLID_INLINE const FSelfType& SetCompare(const ecs_cmp_t& InCompare) const
	{
		ModifyHooksLambda([InCompare](flecs::type_hooks_t& Hooks)
		{
			Hooks.cmp = InCompare;
		});
		
		return *this;
	}

	SOLID_INLINE const FSelfType& SetEquals(const ecs_equals_t& InEquals) const
	{
		ModifyHooksLambda([InEquals](flecs::type_hooks_t& Hooks)
		{
			Hooks.equals = InEquals;
		});
		
		return *this;
	}
	
	/*SOLID_INLINE const FSelfType& OnAdd(const ecs_iter_action_t& InOnAdd) const
	{
		ModifyHooksLambda([InOnAdd](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_add == nullptr, 
				TEXT("OnAdd hook is already set for this component. Only one OnAdd hook can be set per component."));
			
			Hooks.on_add = InOnAdd;
		});
		
		return *this;
	}*/

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnAdd(TFunc&& InOnAddFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc>;
		ModifyHooksLambda([this, InOnAddFunction = Forward(InOnAddFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_add == nullptr, 
				TEXT("OnAdd hook is already set for this component. Only one OnAdd hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_add = FDelegateType::run_add;
			BindingContext->on_add = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnAddFunction));
			BindingContext->free_on_add = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}
	
	/*SOLID_INLINE const FSelfType& OnRemove(const ecs_iter_action_t& InOnRemove) const
	{
		ModifyHooksLambda([InOnRemove](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_remove == nullptr, 
				TEXT("OnRemove hook is already set for this component. Only one OnRemove hook can be set per component."));
			
			Hooks.on_remove = InOnRemove;
		});
		
		return *this;
	}*/

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnRemove(TFunc&& InOnRemoveFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc>;
		ModifyHooksLambda([this, InOnRemoveFunction = Forward(InOnRemoveFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_remove == nullptr, 
				TEXT("OnRemove hook is already set for this component. Only one OnRemove hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_remove = FDelegateType::run_remove;
			BindingContext->on_remove = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnRemoveFunction));
			BindingContext->free_on_remove = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}
	
	/*SOLID_INLINE const FSelfType& OnSet(const ecs_iter_action_t& InOnSet) const
	{
		ModifyHooksLambda([InOnSet](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_set == nullptr, 
				TEXT("OnSet hook is already set for this component. Only one OnSet hook can be set per component."));
			
			Hooks.on_set = InOnSet;
		});
		
		return *this;
	}*/

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnSet(TFunc&& InOnSetFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc>;
		
		ModifyHooksLambda([this, InOnSetFunction = Forward(InOnSetFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_set == nullptr, 
				TEXT("OnSet hook is already set for this component. Only one OnSet hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_set = FDelegateType::run_set;
			BindingContext->on_set = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnSetFunction));
			BindingContext->free_on_set = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnReplace(TFunc&& InOnReplaceFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc>;
		
		ModifyHooksLambda([this, InOnReplaceFunction = Forward(InOnReplaceFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_replace == nullptr, 
				TEXT("OnReplace hook is already set for this component. Only one OnReplace hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_replace = FDelegateType::run_replace;
			BindingContext->on_replace = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnReplaceFunction));
			BindingContext->free_on_replace = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}

	NO_DISCARD SOLID_INLINE uint32 GetSize() const
	{
		solid_check(GetComponentData().size > 0);
		return GetComponentData().size;
	}

	NO_DISCARD SOLID_INLINE uint32 GetAlignment() const
	{
		solid_check(GetComponentData().alignment > 0);
		return GetComponentData().alignment;
	}

	NO_DISCARD SOLID_INLINE FFlecsMemberHandle GetLastMember() const
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
		return FFlecsMemberHandle(ecs_cpp_last_member(GetNativeFlecsWorld(), GetEntity()));
	}
	
	SOLID_INLINE const FSelfType& AddMember(const FFlecsId InTypeId,
								const FFlecsId InUnitId,
								const FString& InName,
	                            const uint32 InCount = 0) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member(InTypeId,
		                             InUnitId,
		                             StringCast<char>(*InName).Get(),
		                             InCount);
		return *this;
	}
	
	SOLID_INLINE const FSelfType& AddMember(const FFlecsId InTypeId,
								const FFlecsId InUnitId,
								const FString& InName,
								const uint32 InCount,
								const uint64 InSizeOffset) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member(InTypeId,
									 InUnitId,
									 StringCast<char>(*InName).Get(),
									 InCount,
									 InSizeOffset);
		return *this;
	}

	SOLID_INLINE const FSelfType& AddMember(const FFlecsId InTypeId, const FString& InName, const uint32 InCount = 0) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member(InTypeId, StringCast<char>(*InName).Get(), InCount);
		return *this;
	}

	SOLID_INLINE const FSelfType& AddMember(const FFlecsId InTypeId,
	                                          const FString& InName,
	                                          const uint32 InCount,
	                                          const uint64 InSizeOffset) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
		             TEXT("Cannot add member to component while in deferred mode."));
			
		GetUntypedComponent().member(InTypeId,
		                             StringCast<char>(*InName).Get(),
		                             InCount,
		                             InSizeOffset);
		return *this;
	}

	template <typename T>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const uint32 InCount = 0) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<T>(StringCast<char>(*InName).Get(), InCount);
		return *this;
	}

	template <typename T>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const uint32 InCount, const uint64 InSizeOffset) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<T>(StringCast<char>(*InName).Get(), InCount, InSizeOffset);
		return *this;
	}

	template <typename T>
	SOLID_INLINE const FSelfType& AddMember(const FFlecsId InUnitId, const FString& InName, const uint32 InCount = 0) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<T>(InUnitId, StringCast<char>(*InName).Get(), InCount);
		return *this;
	}

	template <typename TMember, typename TUnit>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const uint32 InCount = 0) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<TMember, TUnit>(StringCast<char>(*InName).Get(), InCount);
		return *this;
	}

	template <typename TMember, typename TUnit>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const uint32 InCount, const uint64 InSizeOffset) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<TMember, TUnit>(StringCast<char>(*InName), InCount, InSizeOffset);
		return *this;
	}

	template <typename TMember, typename TComponent>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const TMember TComponent::*MemberPtr) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<TMember, TComponent>(StringCast<char>(*InName).Get(), MemberPtr);
		return *this;
	}

	template <typename TUnit, typename TMember, typename TComponent>
	SOLID_INLINE const FSelfType& AddMember(const FString& InName, const TMember TComponent::*MemberPtr) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add member to component while in deferred mode."));
		
		GetUntypedComponent().member<TUnit, TMember, TComponent>(StringCast<char>(*InName).Get(), MemberPtr);
		return *this;
	}

	template <typename TConstant = uint32>
	SOLID_INLINE const FSelfType& AddConstant(const FString& InName, const TConstant& InValue) const
	{
		solid_checkf(!GetNativeFlecsWorld().is_deferred(),
					 TEXT("Cannot add constant to component while in deferred mode."));
		
		GetUntypedComponent().constant<TConstant>(StringCast<char>(*InName).Get(), InValue);
		return *this;
	}

private:
	NO_DISCARD SOLID_INLINE flecs::untyped_component GetUntypedComponent_Unsafe() const
	{
		return flecs::untyped_component(GetNativeFlecsWorld(), GetEntity());
	}
	
protected:
	
	using FBindingContextType = flecs::_::component_binding_ctx;
	
	NO_DISCARD FORCEINLINE FBindingContextType* GetBindingContext(flecs::type_hooks_t& InHooks) const
	{
		FBindingContextType* BindingContext = static_cast<FBindingContextType*>(InHooks.binding_ctx);
		
		if (!BindingContext)
		{
			BindingContext = FLECS_NEW(FBindingContextType);
			InHooks.binding_ctx = BindingContext;
			InHooks.binding_ctx_free = flecs::_::free_obj<FBindingContextType>;
		}
		
		return BindingContext;
	}
	
}; // struct FFlecsComponentHandle

// @TODO: Impl this
template <typename TComponent>
struct TFlecsComponentHandle : public FFlecsComponentHandle
{
	using FSelfType = TFlecsComponentHandle<TComponent>;
	
public:
	SOLID_INLINE TFlecsComponentHandle() = default;

	SOLID_INLINE TFlecsComponentHandle(const flecs::entity& InEntity)
		: FFlecsComponentHandle(InEntity)
	{
	}

	SOLID_INLINE TFlecsComponentHandle(const FFlecsEntityHandle& InEntityHandle)
		: FFlecsComponentHandle(InEntityHandle)
	{
	}

	SOLID_INLINE TFlecsComponentHandle(const FFlecsComponentHandle& InComponentHandle)
		: FFlecsComponentHandle(InComponentHandle)
	{
	}

	SOLID_INLINE TFlecsComponentHandle(flecs::world& InWorld, const FFlecsId& InId)
		: FFlecsComponentHandle(InWorld, InId)
	{
	}

	SOLID_INLINE TFlecsComponentHandle(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsId& InId)
		: FFlecsComponentHandle(InWorld, InId)
	{
	}

	SOLID_INLINE TFlecsComponentHandle operator=(const flecs::entity& InEntity)
	{
		FFlecsComponentHandle::operator=(InEntity);
		return *this;
	}

	SOLID_INLINE TFlecsComponentHandle operator=(const FFlecsEntityHandle& InEntityHandle)
	{
		FFlecsComponentHandle::operator=(InEntityHandle);
		return *this;
	}

	SOLID_INLINE TFlecsComponentHandle operator=(const FFlecsComponentHandle& InComponentHandle)
	{
		FFlecsComponentHandle::operator=(InComponentHandle);
		return *this;
	}

	NO_DISCARD SOLID_INLINE flecs::component<TComponent> GetTypedComponent() const
	{
		solid_checkf(IsComponent(), TEXT("Entity is not a component"));
		return flecs::component<TComponent>(GetUntypedComponent());
	}

	SOLID_INLINE operator flecs::component<TComponent>() const
	{
		return GetTypedComponent();
	}

	SOLID_INLINE flecs::opaque<TComponent> Opaque(const FFlecsId InId) const
	{
		return GetTypedComponent().opaque(InId);
	}

	template <typename TElement>
	SOLID_INLINE flecs::opaque<TComponent> Opaque(const FFlecsId InId) const
	{
		return GetTypedComponent().template opaque<TElement>(InId);
	}

	template <typename TFunction>
	SOLID_INLINE const FSelfType& Opaque(TFunction&& Func) const
	{
		GetTypedComponent().template opaque<TComponent>(std::forward<TFunction>(Func));
		return *this;
	}
	
	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnAdd(TFunc&& InOnAddFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc, TComponent>;
		
		ModifyHooksLambda([this, InOnAddFunction = Forward(InOnAddFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_add == nullptr, 
				TEXT("OnAdd hook is already set for this component. Only one OnAdd hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_add = FDelegateType::run_add;
			BindingContext->on_add = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnAddFunction));
			BindingContext->free_on_add = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnRemove(TFunc&& InOnRemoveFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc, TComponent>;
		
		ModifyHooksLambda([this, InOnRemoveFunction = Forward(InOnRemoveFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_remove == nullptr, 
				TEXT("OnRemove hook is already set for this component. Only one OnRemove hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_remove = FDelegateType::run_remove;
			BindingContext->on_remove = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnRemoveFunction));
			BindingContext->free_on_remove = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnSet(TFunc&& InOnSetFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc, TComponent>;
		
		ModifyHooksLambda([this, InOnSetFunction = Forward(InOnSetFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_set == nullptr, 
				TEXT("OnSet hook is already set for this component. Only one OnSet hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_set = FDelegateType::run_set;
			BindingContext->on_set = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnSetFunction));
			BindingContext->free_on_set = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}

	template <typename TFunc>
	SOLID_INLINE const FSelfType& OnReplace(TFunc&& InOnReplaceFunction) const
	{
		using FDelegateType = flecs::_::each_delegate<TFunc, TComponent>;
		
		ModifyHooksLambda([this, InOnReplaceFunction = Forward(InOnReplaceFunction)](flecs::type_hooks_t& Hooks)
		{
			solid_checkf(Hooks.on_replace == nullptr, 
				TEXT("OnReplace hook is already set for this component. Only one OnReplace hook can be set per component."));
			
			const TSolidNotNull<FBindingContextType*> BindingContext = GetBindingContext(Hooks);
			
			Hooks.on_replace = FDelegateType::run_replace;
			BindingContext->on_replace = FLECS_NEW(FDelegateType)(FLECS_FWD(InOnReplaceFunction));
			BindingContext->free_on_replace = flecs::_::free_obj<FDelegateType>;
		});
		
		return *this;
	}
	
	
}; // struct TFlecsComponentHandle
