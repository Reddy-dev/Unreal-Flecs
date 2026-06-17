// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

#include "Worlds/FlecsWorldSubsystem.h"

#include "Interfaces/FlecsEntityInterface.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityHandleFunctionLibrary)

namespace UE::Flecs::ComponentValue
{
	void ValidateLayout(
		const FFlecsEntityView& Entity,
		const FFlecsId ComponentId,
		const FStructProperty& ValueProperty)
	{
#if DO_CHECK
		const flecs::world World = Entity.GetNativeFlecsWorld();
		const ecs_entity_t TypeId = ecs_get_typeid(World.c_ptr(), ComponentId);
		const ecs_type_info_t* TypeInfo = ecs_get_type_info(World.c_ptr(), TypeId);

		solid_checkf(TypeInfo, TEXT("Component id does not have value data"));
		solid_checkf(
			TypeInfo->size == ValueProperty.Struct->GetStructureSize(),
			TEXT("Component value size does not match the Blueprint struct size"));
#endif // DO_CHECK
	}

	void CopyToStruct(
		const FFlecsEntityView& Entity,
		const FFlecsId ComponentId,
		const FStructProperty& ValueProperty,
		void* ValueAddress)
	{
		const void* ComponentValue = Entity.TryGet(ComponentId);
		solid_checkf(ComponentValue, TEXT("Entity does not have the requested component value"));
		
		ValidateLayout(Entity, ComponentId, ValueProperty);
		ValueProperty.Struct->CopyScriptStruct(ValueAddress, ComponentValue);
	}

} // namespace UE::Flecs::ComponentValue

/*bool UEntityFunctionLibrary::HasEntityFromObject(UObject* Object)
{
	if UNLIKELY_IF(!ensureMsgf(Object, TEXT("Object is not valid")))
	{
		return false;
	}

	if (Object->Implements<UFlecsEntityInterface>())
	{
		return true;
	}

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		return Actor->FindComponentByClass<UFlecsEntityActorComponent>() != nullptr;
	}
        
	return false;
}

FFlecsEntityHandle UEntityFunctionLibrary::GetEntityFromObject(UObject* Object)
{
	solid_check(Object);

	if (Object->Implements<UFlecsEntityInterface>())
	{
		return CastChecked<IFlecsEntityInterface>(Object)->GetEntityHandle();
	}

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		if (const UFlecsEntityActorComponent* EntityActorComponent = Actor->FindComponentByClass<UFlecsEntityActorComponent>())
		{
			return EntityActorComponent->GetEntityHandle();
		}
	}

	return FFlecsEntityHandle::GetNullHandle();
}*/

FFlecsEntityView UFlecsEntityHandleFunctionLibrary::MakeFlecsEntityViewHandle(const FFlecsId Id,
	const UFlecsWorldInterfaceObject* World)
{
	solid_cassumef(World, TEXT("World is not valid"));
	return FFlecsEntityView(World, Id);
}

FFlecsEntityHandle UFlecsEntityHandleFunctionLibrary::MakeFlecsEntityHandle(const FFlecsId Id,
	const UFlecsWorldInterfaceObject* World)
{
	solid_cassumef(World, TEXT("World is not valid"));
	return FFlecsEntityHandle(World, Id);
}

bool UFlecsEntityHandleFunctionLibrary::ToBool_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id;
}

bool UFlecsEntityHandleFunctionLibrary::IsValid_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsValid();
}

bool UFlecsEntityHandleFunctionLibrary::IsValidBranch_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsValid();
}

bool UFlecsEntityHandleFunctionLibrary::IsAlive_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsAlive();
}

bool UFlecsEntityHandleFunctionLibrary::IsAliveBranch_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsAlive();
}

FFlecsId UFlecsEntityHandleFunctionLibrary::ToFlecsId_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.GetFlecsId();
}

bool UFlecsEntityHandleFunctionLibrary::IsUnrealFlecsWorld(const FFlecsEntityView& Id)
{
	return Id.IsUnrealFlecsWorld();
}

bool UFlecsEntityHandleFunctionLibrary::IsInStage(const FFlecsEntityView& Id)
{
	return Id.IsInStage();
}

FFlecsId UFlecsEntityHandleFunctionLibrary::GetFlecsId(const FFlecsEntityView& Entity)
{
	return Entity.GetFlecsId();
}

UFlecsWorldInterfaceObject* UFlecsEntityHandleFunctionLibrary::GetFlecsWorld(const FFlecsEntityView& Entity)
{
	return Entity.GetFlecsWorld();
}

UWorld* UFlecsEntityHandleFunctionLibrary::GetOuterWorld(const FFlecsEntityView& Entity)
{
	return Entity.GetOuterWorld();
}

FString UFlecsEntityHandleFunctionLibrary::ToString_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.ToString();
}

bool UFlecsEntityHandleFunctionLibrary::EqualEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B)
{
	return A == B;
}

bool UFlecsEntityHandleFunctionLibrary::NotEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B)
{
	return A != B;
}

bool UFlecsEntityHandleFunctionLibrary::EqualEqual_FlecsEntityHandle(const FFlecsEntityHandle& A,
	const FFlecsEntityHandle& B)
{
	return A == B;
}

bool UFlecsEntityHandleFunctionLibrary::NotEqual_FlecsEntityHandle(const FFlecsEntityHandle& A,
	const FFlecsEntityHandle& B)
{
	return A != B;
}

FFlecsEntityView UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityHandleToView(const FFlecsEntityHandle& Entity)
{
	return FFlecsEntityView(Entity);
}

FFlecsId UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityViewToId(const FFlecsEntityView& Entity)
{
	return Entity;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityHandleToId(const FFlecsEntityHandle& Entity)
{
	return Entity;
}

FFlecsEntityHandle UFlecsEntityHandleFunctionLibrary::ToMut_FlecsEntityViewWithWorld(const FFlecsEntityView& Entity,
                                                                                     const UFlecsWorldInterfaceObject* World)
{
	solid_cassumef(World, TEXT("World is not valid"));
	return Entity.ToMut<FFlecsEntityHandle>(World->GetNativeFlecsWorld());
}

bool UFlecsEntityHandleFunctionLibrary::IsComponent(const FFlecsEntityView& Entity)
{
	return Entity.IsComponent();
}

bool UFlecsEntityHandleFunctionLibrary::IsTag(const FFlecsEntityView& Entity)
{
	return Entity.IsTag();
}

bool UFlecsEntityHandleFunctionLibrary::IsValueComponent(const FFlecsEntityView& Entity)
{
	return Entity.IsValueComponent();
}

bool UFlecsEntityHandleFunctionLibrary::IsEnum(const FFlecsEntityView& Entity)
{
	return Entity.IsEnum();
}

bool UFlecsEntityHandleFunctionLibrary::HasParent(const FFlecsEntityView& Entity)
{
	return Entity.HasParent();
}

FFlecsEntityHandle UFlecsEntityHandleFunctionLibrary::GetParent(const FFlecsEntityView& Entity)
{
	return Entity.GetParent<FFlecsEntityHandle>();
}

bool UFlecsEntityHandleFunctionLibrary::IsPrefab(const FFlecsEntityView& Entity)
{
	return Entity.IsPrefab();
}

bool UFlecsEntityHandleFunctionLibrary::IsEnabled(const FFlecsEntityView& Entity)
{
	return Entity.IsEnabled();
}

bool UFlecsEntityHandleFunctionLibrary::HasName(const FFlecsEntityView& Entity)
{
	return Entity.HasName();
}

FString UFlecsEntityHandleFunctionLibrary::GetName(const FFlecsEntityView& Entity)
{
	return Entity.GetName();
}

FString UFlecsEntityHandleFunctionLibrary::GetPath(const FFlecsEntityView& Entity, const FString& Separator,
	const FString& RootSeparator)
{
	return Entity.GetPath(StringCast<char>(*Separator).Get(), StringCast<char>(*RootSeparator).Get());
}

void UFlecsEntityHandleFunctionLibrary::DestroyEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Destroy();
}

void UFlecsEntityHandleFunctionLibrary::ClearEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Clear();
}

void UFlecsEntityHandleFunctionLibrary::EnableEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Enable();
}

void UFlecsEntityHandleFunctionLibrary::DisableEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Disable();
}

bool UFlecsEntityHandleFunctionLibrary::ToggleEntity(const FFlecsEntityHandle& Entity)
{
	return Entity.Toggle();
}

void UFlecsEntityHandleFunctionLibrary::SetName(const FFlecsEntityHandle& Entity, const FString& Name)
{
	Entity.SetName(Name);
}

void UFlecsEntityHandleFunctionLibrary::SetParent(const FFlecsEntityHandle& Entity, const FFlecsEntityHandle& Parent,
	const bool bDontFragment)
{
	if (bDontFragment)
	{
		Entity.SetParent(Parent);
	}
	else
	{
		Entity.SetChildOf(Parent);
	}
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddId(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId)
{
	Entity.Add(ComponentId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddScriptStruct(const FFlecsEntityHandle& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	Entity.Add(ScriptStruct);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddEnum(const FFlecsEntityHandle& Entity,
	const UEnum* Enum)
{
	solid_cassumef(Enum, TEXT("Enum is not valid"));
	Entity.Add(FFlecsCommonHandle::GetInputId(Entity, Enum));
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddEnumConstant(const FFlecsEntityHandle& Entity,
	FSolidEnumSelector EnumConstant)
{
	solid_checkf(EnumConstant.Class, TEXT("Enum constant class is not valid"));

	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumConstant);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	Entity.Add(EnumConstantId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddGameplayTag(const FFlecsEntityHandle& Entity,
	FGameplayTag GameplayTag)
{
	Entity.Add(GameplayTag);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddSolidEnumSelector(const FFlecsEntityHandle& Entity,
	FSolidEnumSelector EnumSelector)
{
	solid_checkf(EnumSelector.Class, TEXT("Enum selector class is not valid"));

	const FFlecsId EnumId = FFlecsCommonHandle::GetInputId(Entity, EnumSelector.Class);
	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumSelector);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	Entity.AddPair(EnumId, EnumConstantId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_AddPairIds(const FFlecsEntityHandle& Entity,
	const FFlecsId RelationId,
	const FFlecsId TargetId)
{
	solid_checkf(RelationId.IsValid(), TEXT("Relation id is not valid"));
	solid_checkf(!RelationId.IsPair(), TEXT("Relation id must not be a pair"));
	solid_checkf(TargetId.IsValid(), TEXT("Target id is not valid"));
	solid_checkf(!TargetId.IsPair(), TEXT("Target id must not be a pair"));

	Entity.AddPair(RelationId, TargetId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_SetId(const FFlecsEntityHandle&,
	const FFlecsId,
	const int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsEntityHandleFunctionLibrary::execEntityHandle_SetId)
{
	P_GET_STRUCT_REF(FFlecsEntityHandle, Entity);
	P_GET_STRUCT(FFlecsId, ComponentId);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<const void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;
	
	solid_checkf(ComponentId.IsValid(), TEXT("Set component id is not valid"));

	Entity.Set(ComponentId, ValueProperty->Struct->GetStructureSize(), ValueAddress);

	P_NATIVE_END;
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_SetSolidEnumSelector(
	const FFlecsEntityHandle&,
	FSolidEnumSelector,
	const int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsEntityHandleFunctionLibrary::execEntityHandle_SetSolidEnumSelector)
{
	P_GET_STRUCT_REF(FFlecsEntityHandle, Entity);
	P_GET_STRUCT(FSolidEnumSelector, EnumSelector);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<const void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;
	
	solid_checkf(EnumSelector.Class, TEXT("Enum selector class is not valid"));

	const FFlecsId EnumId = FFlecsCommonHandle::GetInputId(Entity, EnumSelector.Class);
	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumSelector);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	Entity.Set(
		FFlecsId::MakePair(EnumId, EnumConstantId),
		ValueProperty->Struct->GetStructureSize(),
		ValueAddress);

	P_NATIVE_END;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityHandle_MakePairId(
	const FFlecsId RelationId,
	const FFlecsId TargetId)
{
	solid_checkf(RelationId.IsValid(), TEXT("Relation id is not valid"));
	solid_checkf(!RelationId.IsPair(), TEXT("Relation id must not be a pair"));
	solid_checkf(TargetId.IsValid(), TEXT("Target id is not valid"));
	solid_checkf(!TargetId.IsPair(), TEXT("Target id must not be a pair"));
	return FFlecsId::MakePair(RelationId, TargetId);
}

void UFlecsEntityHandleFunctionLibrary::EntityView_GetId(
	const FFlecsEntityView&,
	const FFlecsId,
	int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsEntityHandleFunctionLibrary::execEntityView_GetId)
{
	P_GET_STRUCT_REF(FFlecsEntityView, Entity);
	P_GET_STRUCT(FFlecsId, ComponentId);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;
	
	UE::Flecs::ComponentValue::CopyToStruct(
		Entity,
		ComponentId,
		*ValueProperty,
		ValueAddress);

	P_NATIVE_END;
}

void UFlecsEntityHandleFunctionLibrary::EntityView_GetScriptStruct(
	const FFlecsEntityView&,
	const UScriptStruct*,
	int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsEntityHandleFunctionLibrary::execEntityView_GetScriptStruct)
{
	P_GET_STRUCT_REF(FFlecsEntityView, Entity);
	P_GET_OBJECT(UScriptStruct, ScriptStruct);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;

	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	
	UE::Flecs::ComponentValue::CopyToStruct(
		Entity,
		FFlecsCommonHandle::GetInputId(Entity, ScriptStruct),
		*ValueProperty,
		ValueAddress);

	P_NATIVE_END;
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveId(const FFlecsEntityHandle& Entity,
	const FFlecsId ComponentId)
{
	Entity.Remove(ComponentId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveScriptStruct(const FFlecsEntityHandle& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	Entity.Remove(ScriptStruct);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveEnum(
	const FFlecsEntityHandle& Entity,
	const UEnum* Enum)
{
	solid_cassumef(Enum, TEXT("Enum is not valid"));
	Entity.Remove(FFlecsCommonHandle::GetInputId(Entity, Enum));
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveEnumConstant(const FFlecsEntityHandle& Entity,
	FSolidEnumSelector EnumConstant)
{
	solid_checkf(EnumConstant.Class, TEXT("Enum constant class is not valid"));

	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumConstant);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	Entity.Remove(EnumConstantId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveGameplayTag(const FFlecsEntityHandle& Entity,
	FGameplayTag GameplayTag)
{
	Entity.Remove(GameplayTag);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_RemoveSolidEnumSelector(const FFlecsEntityHandle& Entity,
	FSolidEnumSelector EnumSelector)
{
	solid_checkf(EnumSelector.Class, TEXT("Enum selector class is not valid"));
	Entity.Remove(EnumSelector.Class, EnumSelector.Value);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_ModifiedId(
	const FFlecsEntityHandle& Entity,
	const FFlecsId ComponentId)
{
	solid_checkf(ComponentId.IsValid(), TEXT("Modified component id is not valid"));
	Entity.Modified(ComponentId);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_ModifiedScriptStruct(
	const FFlecsEntityHandle& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	Entity.Modified(ScriptStruct);
}

void UFlecsEntityHandleFunctionLibrary::EntityHandle_ModifiedPairIds(
	const FFlecsEntityHandle& Entity,
	const FFlecsId RelationId,
	const FFlecsId TargetId)
{
	solid_checkf(RelationId.IsValid(), TEXT("Modified relation id is not valid"));
	solid_checkf(!RelationId.IsPair(), TEXT("Modified relation id must not be a pair"));
	solid_checkf(TargetId.IsValid(), TEXT("Modified target id is not valid"));
	solid_checkf(!TargetId.IsPair(), TEXT("Modified target id must not be a pair"));
	Entity.ModifiedPair(RelationId, TargetId);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasId(const FFlecsEntityView& Entity, const FFlecsId ComponentId)
{
	return Entity.Has(ComponentId);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasScriptStruct(
	const FFlecsEntityView& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	return Entity.Has(ScriptStruct);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasEnum(
	const FFlecsEntityView& Entity,
	const UEnum* Enum)
{
	solid_cassumef(Enum, TEXT("Enum is not valid"));
	return Entity.Has(FFlecsCommonHandle::GetInputId(Entity, Enum));
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasEnumConstant(
	const FFlecsEntityView& Entity,
	FSolidEnumSelector EnumConstant)
{
	solid_checkf(EnumConstant.Class, TEXT("Enum constant class is not valid"));
	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumConstant);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	return Entity.Has(EnumConstantId);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasGameplayTag(
	const FFlecsEntityView& Entity,
	FGameplayTag GameplayTag)
{
	return Entity.Has(GameplayTag);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasSolidEnumSelector(
	const FFlecsEntityView& Entity,
	FSolidEnumSelector EnumSelector)
{
	return Entity.Has(EnumSelector);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasPairIds(
	const FFlecsEntityView& Entity,
	const FFlecsId RelationId,
	const FFlecsId TargetId)
{
	solid_checkf(RelationId.IsValid(), TEXT("Relation id is not valid"));
	solid_checkf(!RelationId.IsPair(), TEXT("Relation id must not be a pair"));
	solid_checkf(TargetId.IsValid(), TEXT("Target id is not valid"));
	solid_checkf(!TargetId.IsPair(), TEXT("Target id must not be a pair"));

	return Entity.HasPair(RelationId, TargetId);
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasAnyIds(
	const FFlecsEntityView& Entity,
	const TArray<FFlecsId>& ComponentIds)
{
	for (const FFlecsId ComponentId : ComponentIds)
	{
		if (Entity.Has(ComponentId))
		{
			return true;
		}
	}

	return false;
}

bool UFlecsEntityHandleFunctionLibrary::EntityView_HasAllIds(
	const FFlecsEntityView& Entity,
	const TArray<FFlecsId>& ComponentIds)
{
	for (const FFlecsId ComponentId : ComponentIds)
	{
		if (!Entity.Has(ComponentId))
		{
			return false;
		}
	}

	return true;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityView_ResolveId(
	const FFlecsEntityView&,
	const FFlecsId Id)
{
	return Id;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityView_ResolveScriptStructId(
	const FFlecsEntityView& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	return FFlecsCommonHandle::GetInputId(Entity, ScriptStruct);
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityView_ResolveEnumId(
	const FFlecsEntityView& Entity,
	const UEnum* Enum)
{
	solid_cassumef(Enum, TEXT("Enum is not valid"));
	return FFlecsCommonHandle::GetInputId(Entity, Enum);
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityView_ResolveEnumConstantId(
	const FFlecsEntityView& Entity,
	FSolidEnumSelector EnumConstant)
{
	solid_checkf(EnumConstant.Class, TEXT("Enum constant class is not valid"));

	const FFlecsId EnumConstantId = Entity.GetEnumConstant<FFlecsId>(EnumConstant);
	solid_checkf(EnumConstantId.IsValid(), TEXT("Enum constant is not registered"));
	return EnumConstantId;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::EntityView_ResolveGameplayTagId(
	const FFlecsEntityView& Entity,
	FGameplayTag GameplayTag)
{
	return FFlecsCommonHandle::GetInputId(Entity, GameplayTag);
}
