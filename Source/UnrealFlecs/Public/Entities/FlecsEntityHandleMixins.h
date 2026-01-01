// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsHandleTraits.h"
#include "FlecsHandleTraits.inl"
#include "FlecsId.h"
#include "FlecsEntityHandleTypes.h"

template<typename TDerived>
struct TFlecsHandleCRTPBase
{
public:
	using DerivedType = TDerived;
	
	using SelfType = TFlecsHandleSelfTrait<TDerived>;
	using ViewType = TFlecsHandleViewTrait<TDerived>;
	using MutableType = TFlecsHandleMutableTrait<TDerived>;
	
	NO_DISCARD FORCEINLINE ViewType ToView() const
	{
		return AsView();
	}
	
	NO_DISCARD FORCEINLINE MutableType ToMutable() const
	{
		return AsMutable();
	}
	
protected:
	NO_DISCARD FORCEINLINE SelfType& GetSelfRef()
	{
		return static_cast<SelfType&>(*this);
	}
	
	NO_DISCARD FORCEINLINE const SelfType& GetSelfRef() const
	{
		return static_cast<const SelfType&>(*this);
	}
	
	NO_DISCARD FORCEINLINE ViewType AsView() const
	{
		return ViewType(GetSelfRef().GetNativeFlecsWorld(), GetSelfRef().GetFlecsId());
	}
	
	NO_DISCARD FORCEINLINE MutableType AsMutable() const
	{
		return MutableType(GetSelfRef().GetNativeFlecsWorld(), GetSelfRef().GetFlecsId());
	}
		
}; // struct TFlecsHandleCRTPBase

template <typename TDerived>
struct TFlecsCloneOps : public TFlecsHandleCRTPBase<TDerived>
{
public:
	using TFlecsHandleCRTPBase<TDerived>::SelfType;

	template <Unreal::Flecs::TFlecsEntityHandleTypeConcept THandle = TDerived>
	NO_DISCARD SOLID_INLINE TDerived Clone_Impl(const bool bCloneValue = true, const FFlecsId DestinationId = 0) const
	{
		const flecs::entity_view NewView = this->GetSelfRef().AsView().clone(bCloneValue, DestinationId);
		
		return TDerived(this->GetSelfRef().GetNativeFlecsWorld(), FFlecsId(NewView));
	}
	
}; // struct TFlecsCloneOps

