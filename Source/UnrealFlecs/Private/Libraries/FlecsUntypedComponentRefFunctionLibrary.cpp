// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Libraries/FlecsUntypedComponentRefFunctionLibrary.h"

#include "UObject/UnrealType.h"

#include "Entities/FlecsCommonHandle.h"
#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsUntypedComponentRefFunctionLibrary)

namespace UE::Flecs::UntypedComponentRef
{
	void ValidateLayout(
		const FFlecsUntypedComponentRef& Reference,
		const FStructProperty& ValueProperty)
	{
#if DO_CHECK
		const flecs::world World = Reference.GetEntity().GetNativeFlecsWorld();
		const ecs_entity_t TypeId = ecs_get_typeid(World.c_ptr(), Reference.GetComponentId());
		const ecs_type_info_t* TypeInfo = ecs_get_type_info(World.c_ptr(), TypeId);

		solid_checkf(TypeInfo, TEXT("Component id does not have value data"));
		solid_checkf(
			TypeInfo->size == ValueProperty.Struct->GetStructureSize(),
			TEXT("Component value size does not match the Blueprint struct size"));
#endif // DO_CHECK
	}

	void CopyToStruct(
		const FFlecsUntypedComponentRef& Reference,
		const FStructProperty& ValueProperty,
		void* ValueAddress)
	{
		const void* ComponentValue =
			const_cast<FFlecsUntypedComponentRef&>(Reference).TryGetComponentValue();
		solid_checkf(ComponentValue, TEXT("Untyped component reference is not valid"));

		ValidateLayout(Reference, ValueProperty);
		ValueProperty.Struct->CopyScriptStruct(ValueAddress, ComponentValue);
	}

	void CopyFromStruct(
		const FFlecsUntypedComponentRef& Reference,
		const FStructProperty& ValueProperty,
		const void* ValueAddress)
	{
		void* ComponentValue =
			const_cast<FFlecsUntypedComponentRef&>(Reference).TryGetComponentValue();
		solid_checkf(ComponentValue, TEXT("Untyped component reference is not valid"));

		ValidateLayout(Reference, ValueProperty);
		ValueProperty.Struct->CopyScriptStruct(ComponentValue, ValueAddress);
		Reference.GetEntity().Modified(Reference.GetComponentId());
	}
	
} // namespace UE::Flecs::UntypedComponentRef

FFlecsUntypedComponentRef UFlecsUntypedComponentRefFunctionLibrary::EntityView_GetUntypedComponentRefId(
	const FFlecsEntityView& Entity,
	const FFlecsId ComponentId)
{
	solid_checkf(Entity.TryGet(ComponentId), TEXT("Entity does not have the requested component value"));
	return FFlecsUntypedComponentRef(FFlecsEntityHandle(Entity.GetEntity()), ComponentId);
}

FFlecsUntypedComponentRef UFlecsUntypedComponentRefFunctionLibrary::EntityView_GetUntypedComponentRefScriptStruct(
	const FFlecsEntityView& Entity,
	const UScriptStruct* ScriptStruct)
{
	solid_cassumef(ScriptStruct, TEXT("ScriptStruct is not valid"));
	return EntityView_GetUntypedComponentRefId(
		Entity,
		FFlecsCommonHandle::GetInputId(Entity, ScriptStruct));
}

bool UFlecsUntypedComponentRefFunctionLibrary::IsValid_FlecsUntypedComponentRef(
	const FFlecsUntypedComponentRef& Reference)
{
	return const_cast<FFlecsUntypedComponentRef&>(Reference).IsValid();
}

bool UFlecsUntypedComponentRefFunctionLibrary::ToBool_FlecsUntypedComponentRef(
	const FFlecsUntypedComponentRef& Reference)
{
	return const_cast<FFlecsUntypedComponentRef&>(Reference).IsValid();
}

void UFlecsUntypedComponentRefFunctionLibrary::UntypedComponentRef_GetValue(
	const FFlecsUntypedComponentRef&,
	int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsUntypedComponentRefFunctionLibrary::execUntypedComponentRef_GetValue)
{
	P_GET_STRUCT_REF(FFlecsUntypedComponentRef, Reference);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;

	UE::Flecs::UntypedComponentRef::CopyToStruct(
		Reference,
		*ValueProperty,
		ValueAddress);

	P_NATIVE_END;
}

void UFlecsUntypedComponentRefFunctionLibrary::UntypedComponentRef_SetValue(
	const FFlecsUntypedComponentRef&,
	const int32&)
{
	solid_check(false);
}

DEFINE_FUNCTION(UFlecsUntypedComponentRefFunctionLibrary::execUntypedComponentRef_SetValue)
{
	P_GET_STRUCT_REF(FFlecsUntypedComponentRef, Reference);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const TSolidNotNull<const FStructProperty*> ValueProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	const TSolidNotNull<const void*> ValueAddress = Stack.MostRecentPropertyAddress;

	P_FINISH;
	P_NATIVE_BEGIN;

	UE::Flecs::UntypedComponentRef::CopyFromStruct(
		Reference,
		*ValueProperty,
		ValueAddress);

	P_NATIVE_END;
}

void UFlecsUntypedComponentRefFunctionLibrary::UntypedComponentRef_Modified(
	const FFlecsUntypedComponentRef& Reference)
{
	Reference.GetEntity().Modified(Reference.GetComponentId());
}
