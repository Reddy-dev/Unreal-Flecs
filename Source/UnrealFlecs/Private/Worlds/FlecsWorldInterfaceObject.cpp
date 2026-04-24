// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Worlds/FlecsWorldInterfaceObject.h"

#include "EntityRecords/FlecsEntityRecord.h"
#include "EntityRecords/FlecsEntityRecordComponent.h"
#include "Logs/FlecsCategories.h"
#include "Pipelines/FlecsGameLoopInterface.h"
#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorldInterfaceObject)

namespace
{
	inline void ScriptStructConstructor(void* Ptr, int32_t Count, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(Ptr != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		ScriptStruct->InitializeStruct(Ptr, Count);
	}

	inline void ScriptStructDestructor(void* Ptr, int32_t Count, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(Ptr != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		ScriptStruct->DestroyStruct(Ptr, Count);
	}

	// @TODO: maybe add a RESTRICT keyword?
	
	inline void ScriptStructCopy(void* Destination, const void* Source, int32_t Count, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(Destination != nullptr);
		solid_cassume(Source != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		ScriptStruct->CopyScriptStruct(Destination, Source, Count);
	}

	inline void ScriptStructMove(void* Destination, void* Source, int32_t Count, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(Destination != nullptr);
		solid_cassume(Source != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		Solid::MoveAssignScriptStruct(ScriptStruct, Destination, Source, Count);
	}

	inline void ScriptStructMoveConstruct(void* Destination, void* Source, int32_t Count, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(Destination != nullptr);
		solid_cassume(Source != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		Solid::MoveConstructScriptStruct(ScriptStruct, Destination, Source, Count);
	}

	// @TODO: implement
	inline int32 ScriptStructCompare(const void* A, const void* B, const ecs_type_info_t* TypeInfo)
	{
		// empty
		return 0;
	}
	
	inline bool ScriptStructEquals(const void* A, const void* B, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(A != nullptr);
		solid_cassume(B != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		const UScriptStruct* ScriptStruct = static_cast<UScriptStruct*>(TypeInfo->hooks.ctx);
		solid_check(IsValid(ScriptStruct));

		return ScriptStruct->CompareScriptStruct(A, B, PPF_None);
	}

	inline bool ScriptStructEqualsSimple(const void* A, const void* B, const ecs_type_info_t* TypeInfo)
	{
		solid_cassume(TypeInfo != nullptr);
		solid_cassume(A != nullptr);
		solid_cassume(B != nullptr);
		solid_cassume(TypeInfo->hooks.ctx != nullptr);

		return FMemory::Memcmp(A, B, TypeInfo->size) == 0;
	}
	
} // namespace

UFlecsWorldInterfaceObject::UFlecsWorldInterfaceObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlecsWorldInterfaceObject::~UFlecsWorldInterfaceObject()
{
}

UFlecsWorld* UFlecsWorldInterfaceObject::GetFlecsWorld() const
{
	UFlecsWorld* OuterWorld = GetTypedOuter<UFlecsWorld>();
	return CastChecked<UFlecsWorld>(const_cast<UFlecsWorldInterfaceObject*>(OuterWorld ? OuterWorld : this));
}

flecs::world UFlecsWorldInterfaceObject::GetNativeFlecsWorld() const
{
	return *GetNativeFlecsWorld_Internal();
}

bool UFlecsWorldInterfaceObject::IsStage() const
{
	return GetNativeFlecsWorld_Internal()->is_stage();
}

bool UFlecsWorldInterfaceObject::IsReadOnly() const
{
	return GetNativeFlecsWorld_Internal()->is_readonly();
}

bool UFlecsWorldInterfaceObject::ShouldQuit() const
{
	return GetNativeFlecsWorld_Internal()->should_quit();
}

bool UFlecsWorldInterfaceObject::BeginDefer() const
{
	return GetNativeFlecsWorld_Internal()->defer_begin();
}

FFlecsScopedDeferWindow UFlecsWorldInterfaceObject::DeferWindow() const
{
	return FFlecsScopedDeferWindow(this);
}

bool UFlecsWorldInterfaceObject::EndDefer() const
{
	return GetNativeFlecsWorld_Internal()->defer_end();
}

void UFlecsWorldInterfaceObject::ResumeDefer() const
{
	return GetNativeFlecsWorld_Internal()->defer_resume();
}

void UFlecsWorldInterfaceObject::SuspendDefer() const
{
	return GetNativeFlecsWorld_Internal()->defer_suspend();
}

bool UFlecsWorldInterfaceObject::IsDeferred() const
{
	return GetNativeFlecsWorld_Internal()->is_deferred();
}

bool UFlecsWorldInterfaceObject::IsDeferSuspended() const
{
	return GetNativeFlecsWorld_Internal()->is_defer_suspended();
}

double UFlecsWorldInterfaceObject::GetTimeScale() const
{
	return GetNativeFlecsWorld_Internal()->get_info()->time_scale;
}

bool UFlecsWorldInterfaceObject::DoesExist(const FFlecsId InId) const
{
	return GetNativeFlecsWorld_Internal()->exists(InId);
}

bool UFlecsWorldInterfaceObject::IsAlive(const FFlecsId InId) const
{
	return GetNativeFlecsWorld_Internal()->is_alive(InId);
}

bool UFlecsWorldInterfaceObject::IsValidId(const FFlecsId InId) const
{
	return GetNativeFlecsWorld_Internal()->is_valid(InId);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetAlive(const FFlecsId InId) const
{
	return FFlecsEntityHandle(GetNativeFlecsWorld_Internal()->get_alive(InId));
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::MakeAlive(const FFlecsId InId) const
{
	return FFlecsEntityHandle(GetNativeFlecsWorld_Internal()->make_alive(InId));
}

double UFlecsWorldInterfaceObject::GetDeltaTime() const
{
	return GetNativeFlecsWorld_Internal()->delta_time();
}

void UFlecsWorldInterfaceObject::Merge() const
{
	GetNativeFlecsWorld_Internal()->merge();
}

void* UFlecsWorldInterfaceObject::GetContext() const
{
	return GetNativeFlecsWorld_Internal()->get_ctx();
}

FFlecsTypeMapComponent* UFlecsWorldInterfaceObject::GetTypeMapComponent() const
{
	return static_cast<FFlecsTypeMapComponent*>(GetNativeFlecsWorld_Internal()->get_binding_ctx());
}

FFlecsId UFlecsWorldInterfaceObject::SetScope(const FFlecsId InScope) const
{
	return GetNativeFlecsWorld_Internal()->set_scope(InScope);
}

FFlecsId UFlecsWorldInterfaceObject::GetScope() const
{
	return GetNativeFlecsWorld_Internal()->get_scope();
}

FFlecsId UFlecsWorldInterfaceObject::ClearScope() const
{
	return GetNativeFlecsWorld_Internal()->set_scope(FFlecsId::Null());
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::LookupEntity(const FString& Name,
	const FString& Separator, const FString& RootSeparator,
	const bool bRecursive) const
{
	return GetNativeFlecsWorld_Internal()->lookup(StringCast<char>(*Name).Get(),
						StringCast<char>(*Separator).Get(),
						StringCast<char>(*RootSeparator).Get(),
						bRecursive);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::LookupEntityBySymbol_Internal(const FString& Symbol,
	const bool bLookupAsPath, const bool bRecursive) const
{
	return FFlecsEntityHandle(this, ecs_lookup_symbol(
		GetNativeFlecsWorld_Internal()->world_,
		StringCast<char>(*Symbol).Get(),
		bLookupAsPath,
		bRecursive));
}

int32 UFlecsWorldInterfaceObject::Count(const FFlecsId InComponentId) const
{
	return GetNativeFlecsWorld_Internal()->count(InComponentId);
}

int32 UFlecsWorldInterfaceObject::CountPair(const FFlecsId InFirstId, const FFlecsId InSecondId) const
{
	return GetNativeFlecsWorld_Internal()->count(InFirstId, InSecondId);
}

bool UFlecsWorldInterfaceObject::IsIdInUse(const FFlecsId InId) const
{
	return ecs_id_in_use(GetNativeFlecsWorld_Internal()->c_ptr(), InId);
}

FFlecsId UFlecsWorldInterfaceObject::GetTypeId(const FFlecsId InId) const
{
	return ecs_get_typeid(GetNativeFlecsWorld_Internal()->c_ptr(), InId);
}

bool UFlecsWorldInterfaceObject::IsIdType(const FFlecsId InId) const
{
	return GetTypeId(InId) != FFlecsId::Null();
}

bool UFlecsWorldInterfaceObject::IsIdTag(const FFlecsId InId) const
{
	return ecs_id_is_tag(GetNativeFlecsWorld_Internal()->c_ptr(), InId);
}

void UFlecsWorldInterfaceObject::RegisterMemberProperties(const TSolidNotNull<const UStruct*> InStruct,
	const FFlecsComponentHandle& InComponent) const
{
	solid_checkf(!IsDeferred(), TEXT("Cannot register member properties while world is deferred"));
	solid_checkf(InComponent.IsValid(), TEXT("Entity is nullptr"));

	for (TFieldIterator<FProperty> PropertyIt(InStruct, EFieldIterationFlags::IncludeSuper);
	     PropertyIt; ++PropertyIt)
	{
		TSolidNotNull<FProperty*> Property = *PropertyIt;
		
		if (Property->IsA<FBoolProperty>())
		{
			InComponent.AddMember<bool>(Property->GetName(),
			                            0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FByteProperty>())
		{
			InComponent.AddMember<uint8>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FInt16Property>())
		{
			InComponent.AddMember<int16>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FUInt16Property>())
		{
			InComponent.AddMember<uint16>(Property->GetName(),
			                              0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FIntProperty>())
		{
			InComponent.AddMember<int32>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FUInt32Property>())
		{
			InComponent.AddMember<uint32>(Property->GetName(),
			                              0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FInt64Property>())
		{
			InComponent.AddMember<int64>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FUInt64Property>())
		{
			InComponent.AddMember<uint64>(Property->GetName(),
			                              0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FFloatProperty>())
		{
			InComponent.AddMember<float>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FDoubleProperty>())
		{
			InComponent.AddMember<double>(Property->GetName(),
			                              0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FStrProperty>())
		{
			InComponent.AddMember<FString>(Property->GetName(),
			                               0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FNameProperty>())
		{
			InComponent.AddMember<FName>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FTextProperty>())
		{
			InComponent.AddMember<FText>(Property->GetName(),
			                             0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FObjectProperty>())
		{
			InComponent.AddMember<FObjectPtr>(Property->GetName(),
			                                  0, Property->GetOffset_ForInternal());

			if UNLIKELY_IF(IsIdInUse(InComponent))
			{
				continue;
			}
		}
		else if (Property->IsA<FWeakObjectProperty>())
		{
			InComponent.AddMember<FWeakObjectPtr>(Property->GetName(),
			                                      0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FSoftObjectProperty>())
		{
			InComponent.AddMember<FSoftObjectPtr>(Property->GetName(),
			                                      0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FClassProperty>())
		{
			InComponent.AddMember<TSubclassOf<UObject>>(Property->GetName(),
			                                            0, Property->GetOffset_ForInternal());
		}
		else if (Property->IsA<FStructProperty>())
		{
			FFlecsEntityHandle StructComponent;
			if (!HasScriptStruct(CastFieldChecked<FStructProperty>(Property)->Struct))
			{
				UE_LOGFMT(LogFlecsWorld, Log,
				          "Property Type Script struct {StructName} is not registered for entity {ComponentName}",
				          CastFieldChecked<FStructProperty>(Property)->Struct->GetStructCPPName(),
				          InComponent.GetName());
				continue;
			}
			else
			{
				StructComponent = GetScriptStructEntity(CastFieldChecked<FStructProperty>(Property)->Struct);
			}

			if (!StructComponent.Has<flecs::Type>())
			{
				UE_LOGFMT(LogFlecsWorld, Log,
				          "Property Type Script struct {StructName} does not have flecs::Type for entity {ComponentName}",
				          CastFieldChecked<FStructProperty>(Property)->Struct->GetStructCPPName(),
				          InComponent.GetName());
				continue;
			}
			 		
			InComponent.AddMember(StructComponent, Property->GetName(),
			                      0, Property->GetOffset_ForInternal());
		}
		else UNLIKELY_ATTRIBUTE
		{
			UE_LOGFMT(LogFlecsWorld, Log,
			          "Property Type: {PropertyName}({TypeName} is not supported for member serialization",
			          Property->GetNameCPP(),
			          Property->GetOwnerStruct()->GetName());
		}
	}
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterScriptStruct(const UScriptStruct* ScriptStruct,
                                                                    const bool bComponent, const bool bRegisterMemberProperties) const
{
	solid_cassume(ScriptStruct);
	solid_checkf(!IsDeferred(), TEXT("Registering script structs while deferred is not allowed"));

		const FFlecsId OldScope = ClearScope();

		solid_checkf(!GetTypeMapComponent()->ScriptStructMap.contains(ScriptStruct),
			TEXT("Script struct %s is already registered"), *ScriptStruct->GetStructCPPName());
		
		FFlecsComponentHandle ScriptStructComponent;

		const FString StructName = ScriptStruct->GetStructCPPName();
		const char* StructNameCStr = StringCast<char>(*StructName).Get(); // NOLINT(clang-diagnostic-dangling)

		// Register Member properties can't be deferred
		DeferEndLambda([this, ScriptStruct, &ScriptStructComponent, StructNameCStr, bComponent, &StructName, bRegisterMemberProperties]()
		{
			ScriptStructComponent = GetNativeFlecsWorld_Internal()->component(StructNameCStr);
			solid_check(ScriptStructComponent.IsValid());
			
			ScriptStructComponent.GetEntity().set_symbol(StructNameCStr);
			
			const TFieldIterator<FProperty> PropertyIt(ScriptStruct);
			const bool bIsTag = ScriptStruct->GetStructureSize() <= 1 && !PropertyIt;

			if (bComponent)
			{
				ScriptStructComponent.Set<flecs::Component>({
					.size = bIsTag ? 0 : ScriptStruct->GetStructureSize(),
					.alignment = bIsTag ? 0 : ScriptStruct->GetMinAlignment()
				});

				if (!bIsTag)
				{
					if (ScriptStruct->GetCppStructOps()->HasNoopConstructor())
					{
						UE_LOGFMT(LogFlecsComponent, Log,
							"Script struct {StructName} has a No-op constructor, this will not be used in flecs",
							ScriptStruct->GetName());
					}

					ScriptStructComponent.ModifyHooksLambda([ScriptStruct, &ScriptStructComponent](flecs::type_hooks_t& Hooks)
					{
						const bool bIsPOD = ScriptStruct->GetCppStructOps()->IsPlainOldData();
						
						const bool bHasCtor = !ScriptStruct->GetCppStructOps()->HasZeroConstructor();
						const bool bHasDtor = ScriptStruct->GetCppStructOps()->HasDestructor();
						const bool bHasCopy = ScriptStruct->GetCppStructOps()->HasCopy();
						const bool bHasMove = FSolidMoveableStructRegistry::Get().IsStructMoveAssignable(ScriptStruct);
						const bool bHasMoveCtor = FSolidMoveableStructRegistry::Get().IsStructMoveConstructible(ScriptStruct);

						const bool bHasIdentical = ScriptStruct->GetCppStructOps()->HasIdentical();
						
						Hooks.ctx = const_cast<UScriptStruct*>(ScriptStruct);  // NOLINT(cppcoreguidelines-pro-type-const-cast)
						
						if (bIsPOD)
						{
							Hooks.ctor = nullptr;
							Hooks.dtor = nullptr;
							Hooks.copy = nullptr;
							Hooks.move = nullptr;
						}
						else
						{
							if (bHasCtor)
							{
								Hooks.ctor = ScriptStructConstructor;
							}
							else
							{
								Hooks.ctor = nullptr;
							}
						
							if (bHasDtor)
							{
								Hooks.dtor = ScriptStructDestructor;
							}
							else
							{
								Hooks.dtor = nullptr;
							}

							if (bHasCopy)
							{
								Hooks.copy = ScriptStructCopy;
							}
							else
							{
								Hooks.copy = nullptr;
							}

							if (bHasMove)
							{
								Hooks.move = ScriptStructMove;
							}
							else
							{
								Hooks.move = nullptr;
							}

							if (bHasMoveCtor)
							{
								Hooks.move_ctor = ScriptStructMoveConstruct;
							}
							else
							{
								Hooks.move_ctor = nullptr;
							}

							if (!bHasCopy && !bHasMove)
							{
								ScriptStructComponent.Add(flecs::Sparse);
							
								UE_LOGFMT(LogFlecsComponent, Log,
									"Script struct {StructName} registered as Sparse component due to missing copy/move operations",
									ScriptStruct->GetName());
							}
						}

						if (!bHasIdentical && bIsPOD)
						{
							Hooks.equals = ScriptStructEqualsSimple;
						}
						else
						{
							Hooks.equals = ScriptStructEquals;
						}

						// @TODO: Implement this
						Hooks.cmp = nullptr;
					});
				}
			}

			std::string StructNameStdString(StructNameCStr, StructName.Len());
			
			if (!flecs::_::g_type_to_impl_data.contains(StructNameStdString))
			{
				flecs::_::type_impl_data NewData;  // NOLINT(cppcoreguidelines-pro-type-member-init)
				NewData.s_set_values = true;
				NewData.s_index = flecs_component_ids_index_get();
				NewData.s_size = bIsTag ? 0 : ScriptStruct->GetStructureSize();
				NewData.s_alignment = bIsTag ? 0 : ScriptStruct->GetMinAlignment();
				NewData.s_allow_tag = bIsTag;
				NewData.s_enum_registered = false;
				
				flecs::_::g_type_to_impl_data.emplace(StructNameStdString, NewData);
			}

			solid_check(flecs::_::g_type_to_impl_data.contains(StructNameStdString));
			flecs::_::type_impl_data& Data = flecs::_::g_type_to_impl_data.at(StructNameStdString);

			flecs_component_ids_set(GetNativeFlecsWorld(), Data.s_index, ScriptStructComponent);

			GetTypeMapComponent()->ScriptStructMap.emplace(ScriptStruct, ScriptStructComponent);
			
			ScriptStructComponent.Set<FFlecsScriptStructComponent>({ ScriptStruct });

			if (bRegisterMemberProperties)
			{
				RegisterMemberProperties(ScriptStruct, ScriptStructComponent);
			}
			
			UE::FlecsLibrary::GetTypeRegisteredDelegate().Broadcast(ScriptStructComponent);
		});

		SetScope(OldScope);
		return ScriptStructComponent;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterScriptEnum(const UEnum* ScriptEnum) const
{
	solid_cassume(ScriptEnum);
	solid_check(IsValid(ScriptEnum));
	
	solid_checkf(!IsDeferred(), TEXT("Registering script enums while deferred is not allowed"));

	if (HasScriptEnum(ScriptEnum))
	{
		UE_LOGFMT(LogFlecsWorld, Log,
				  "Script enum {EnumName} is already registered", ScriptEnum->GetName());
		return GetScriptEnumEntity(ScriptEnum);
	}

	solid_checkf(!ScriptEnum->HasAnyEnumFlags(EEnumFlags::Flags),
				 TEXT("Script enum %s is not supported, use RegisterScriptBitmask instead"),
				 *ScriptEnum->GetName());

	// if (ScriptEnum->HasAnyEnumFlags(EEnumFlags::Flags))
	// {
	// 	return RegisterComponentBitmaskType(ScriptEnum);
	// }
	// else
	// {
	// 	return RegisterComponentEnumType(ScriptEnum);
	// }
		
	return RegisterComponentEnumType(ScriptEnum);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterComponentEnumType(TSolidNotNull<const UEnum*> ScriptEnum) const
{
	solid_checkf(!IsDeferred(), TEXT("Registering script enums while deferred is not allowed"));
	const FFlecsId OldScope = ClearScope();

		solid_checkf(!GetTypeMapComponent()->ScriptEnumMap.contains(FFlecsScriptEnumComponent(ScriptEnum)),
			TEXT("Script enum %s is already registered"), *ScriptEnum->GetName());

		FFlecsComponentHandle ScriptEnumComponent;

		const FString EnumName = ScriptEnum->GetName();
		const char* EnumNameCStr = StringCast<char>(*EnumName).Get();  // NOLINT(clang-diagnostic-dangling)

		DeferEndLambda([this, ScriptEnum, &ScriptEnumComponent, &EnumNameCStr, &EnumName]()
		{
			ScriptEnumComponent = GetNativeFlecsWorld_Internal()->component(EnumNameCStr);
			solid_check(ScriptEnumComponent.IsValid());
			
			ScriptEnumComponent.GetEntity().set_symbol(EnumNameCStr);

			ScriptEnumComponent.Set<flecs::Component>({
				.size = sizeof(uint8),
				.alignment = alignof(uint8)
			});
			
			ScriptEnumComponent.GetLambda([](flecs::Enum& InEnumComponent)
			{
				InEnumComponent.underlying_type = flecs::U8;
			});

			const int32 EnumCount = ScriptEnum->NumEnums();

			const uint64 MaxEnumValue = ScriptEnum->GetMaxEnumValue();
			const bool bUint8 = MaxEnumValue < std::numeric_limits<uint8>::max();
			
			for (int32 EnumIndex = 0; EnumIndex < EnumCount; ++EnumIndex)
			{
				const int64 EnumValue = ScriptEnum->GetValueByIndex(EnumIndex);
				solid_cassume(EnumValue >= 0);
				
				const FString EnumValueName = ScriptEnum->GetNameStringByIndex(EnumIndex);

				if (std::cmp_equal(MaxEnumValue, EnumValue))
				{
					continue;
				}

				if (bUint8)
				{
					ScriptEnumComponent.AddConstant<uint8>(EnumValueName, static_cast<uint8>(EnumValue));
				}
				else
				{
					ScriptEnumComponent.AddConstant<uint64>(EnumValueName, static_cast<uint64>(EnumValue));
				}
			}

			std::string EnumNameStdString(EnumNameCStr, EnumName.Len());
			if (!flecs::_::g_type_to_impl_data.contains(EnumNameStdString))
			{
				flecs::_::type_impl_data NewData{};
				NewData.s_set_values = true;
				NewData.s_index = flecs_component_ids_index_get();
				NewData.s_size = sizeof(uint8);
				NewData.s_alignment = alignof(uint8);
				NewData.s_allow_tag = false;
				NewData.s_enum_registered = false;
				
				flecs::_::g_type_to_impl_data.emplace(EnumNameStdString, NewData);
			}

			solid_check(flecs::_::g_type_to_impl_data.contains(EnumNameStdString));
			
			auto& [s_set_values, s_index, s_size, s_alignment, s_allow_tag, s_enum_registered]
				= flecs::_::g_type_to_impl_data.at(EnumNameStdString);
			
			flecs_component_ids_set(GetNativeFlecsWorld(), s_index, ScriptEnumComponent);
			GetTypeMapComponent()->ScriptEnumMap.emplace(ScriptEnum, ScriptEnumComponent);
			
			ScriptEnumComponent.Set<FFlecsScriptEnumComponent>(FFlecsScriptEnumComponent(ScriptEnum));
			
			UE::FlecsLibrary::GetTypeRegisteredDelegate().Broadcast(ScriptEnumComponent);
		});

		SetScope(OldScope);
		return ScriptEnumComponent;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterScriptClassType(TSolidNotNull<UClass*> ScriptClass) const
{
	const FFlecsId OldScope = ClearScope();

	const FString ClassName = ScriptClass->GetPrefixCPP() + ScriptClass->GetName();

	if (HasScriptClass(ScriptClass))
	{
		UE_LOGFMT(LogFlecsWorld, Log,
			"Script class {ClassName} is already registered", ClassName);
		return GetScriptClassEntity(ScriptClass);
	}

	FFlecsEntityHandle ScriptClassEntity;

	const char* ClassNameCStr = StringCast<char>(*ClassName).Get(); // NOLINT(clang-diagnostic-dangling)

	DeferEndLambda([this, ScriptClass, &ScriptClassEntity, ClassNameCStr, &ClassName]()
	{
		ScriptClassEntity = CreateEntity(ClassName);
		solid_check(ScriptClassEntity.IsValid());

		ScriptClassEntity.GetEntity().set_symbol(ClassNameCStr);
		GetTypeMapComponent()->ScriptClassMap.emplace(ScriptClass, ScriptClassEntity);

		ScriptClassEntity.Set<FFlecsScriptClassComponent>(FFlecsScriptClassComponent(ScriptClass));

		//RegisterMemberProperties(ScriptClass, ScriptClassComponent);

		std::string ClassNameStdString(ClassNameCStr, ClassName.Len());

		if (!flecs::_::g_type_to_impl_data.contains(ClassNameStdString))
		{
			flecs::_::type_impl_data NewData;  // NOLINT(cppcoreguidelines-pro-type-member-init)]
			NewData.s_set_values = true;
			NewData.s_index = flecs_component_ids_index_get();
			NewData.s_size = 0;
			NewData.s_alignment = 0;
			NewData.s_allow_tag = true;
			NewData.s_enum_registered = false;
				
			flecs::_::g_type_to_impl_data.emplace(ClassNameStdString, NewData);
		}

		solid_check(flecs::_::g_type_to_impl_data.contains(ClassNameStdString));

		flecs::_::type_impl_data& Data = flecs::_::g_type_to_impl_data.at(ClassNameStdString);
			
		flecs_component_ids_set(GetNativeFlecsWorld(), Data.s_index, ScriptClassEntity);
		GetTypeMapComponent()->ScriptClassMap.emplace(ScriptClass, ScriptClassEntity);
	});

	SetScope(OldScope);
		
	return ScriptClassEntity;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterComponentType(
	const TSolidNotNull<const UScriptStruct*> ScriptStruct, const bool bRegisterMemberProperties) const
{
	solid_checkf(!IsDeferred(), TEXT("Cannot register component while deferred"));
	
	if (HasScriptStruct(ScriptStruct))
	{
		return GetScriptStructEntity(ScriptStruct);
	}

	return RegisterScriptStruct(ScriptStruct, true, bRegisterMemberProperties);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::RegisterComponentType(const TSolidNotNull<const UEnum*> ScriptEnum) const
{
	solid_checkf(!IsDeferred(), TEXT("Cannot register component while deferred"));
	
	if (HasScriptEnum(ScriptEnum))
	{
		return GetScriptEnumEntity(ScriptEnum);
	}

	return RegisterScriptEnum(ScriptEnum);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetScriptStructEntity(const UScriptStruct* ScriptStruct) const
{
	solid_cassume(ScriptStruct)

	solid_checkf(HasScriptStruct(ScriptStruct),
		TEXT("Script struct %s is not registered"), *ScriptStruct->GetStructCPPName());
		
	const FFlecsId Component = GetTypeMapComponent()->ScriptStructMap.at(ScriptStruct);
	solid_checkf(IsAlive(Component), TEXT("Entity is not alive"));
		
	return GetAlive(Component);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetScriptEnumEntity(const UEnum* ScriptEnum) const
{
	solid_cassume(ScriptEnum);
	
	solid_checkf(HasScriptEnum(ScriptEnum),
		TEXT("Script enum %s is not registered"), *ScriptEnum->GetName());
		
	const FFlecsId Component = GetTypeMapComponent()->ScriptEnumMap.at(ScriptEnum);
	solid_checkf(IsAlive(Component), TEXT("Entity is not alive"));
	return GetAlive(Component);
}

bool UFlecsWorldInterfaceObject::HasScriptStruct(const UScriptStruct* ScriptStruct) const
{
	solid_cassume(ScriptStruct);
		
	if (GetTypeMapComponent()->ScriptStructMap.contains(ScriptStruct))
	{
		const FFlecsId Component = GetTypeMapComponent()->ScriptStructMap.at(ScriptStruct);
		return IsValidId(Component);
	}
		
	return false;
}

bool UFlecsWorldInterfaceObject::HasScriptEnum(const UEnum* ScriptEnum) const
{
	solid_cassume(ScriptEnum);
		
	if (GetTypeMapComponent()->ScriptEnumMap.contains(ScriptEnum))
	{
		const FFlecsId Component = GetTypeMapComponent()->ScriptEnumMap.at(ScriptEnum);
		return IsValidId(Component);
	}

	return false;
}

bool UFlecsWorldInterfaceObject::HasScriptClass(const TSubclassOf<UObject> InClass) const
{
	solid_check(InClass);
		
	if (GetTypeMapComponent()->ScriptClassMap.contains(FFlecsScriptClassComponent(InClass)))
	{
		const FFlecsId Component = GetTypeMapComponent()->ScriptClassMap.at(FFlecsScriptClassComponent(InClass));
		return IsValidId(Component);
	}

	return false;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetScriptClassEntity(const TSubclassOf<UObject> InClass) const
{
	solid_check(InClass);
		
	const FFlecsId Component = GetTypeMapComponent()->ScriptClassMap.at(FFlecsScriptClassComponent(InClass));
	solid_checkf(IsAlive(Component), TEXT("Entity is not alive"));
		
	return GetAlive(Component);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreateEntity(const FString& Name, const FString& Separator,
	const FString& RootSeparator) const
{
	return GetNativeFlecsWorld_Internal()->entity(StringCast<char>(*Name).Get(), 
						StringCast<char>(*Separator).Get(),
						StringCast<char>(*RootSeparator).Get());
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::ObtainTypedEntity(const TSolidNotNull<UClass*> InClass) const
{
	const FFlecsEntityHandle EntityHandle = GetNativeFlecsWorld_Internal()->entity(RegisterScriptClassType(InClass));
	return EntityHandle;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreateEntityWithId(const FFlecsId InId) const
{
	solid_checkf(!IsAlive(InId), TEXT("Entity with ID %s is already alive"), *InId.ToString());
	return MakeAlive(InId);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreateEntityWithRecord(const FFlecsEntityRecord& InRecord,
                                                                      const FString& Name) const
{
	const FFlecsEntityHandle Entity = CreateEntity(Name);
	InRecord.ApplyRecordToEntity(this, Entity);
	return Entity;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreateEntityWithRecordWithId(const FFlecsEntityRecord& InRecord,
	const FFlecsId InId) const
{
	const FFlecsEntityHandle Entity = CreateEntityWithId(InId);
	InRecord.ApplyRecordToEntity(this, Entity);
	return Entity;
}

void UFlecsWorldInterfaceObject::DestroyEntityByName(const FString& InName) const
{
	solid_checkf(!InName.IsEmpty(), TEXT("Name is empty"));

	const FFlecsEntityHandle Handle = LookupEntity(InName);
		
	if LIKELY_IF(Handle.IsValid())
	{
		Handle.Destroy();
	}
	else
	{
		UE_LOGFMT(LogFlecsWorld, Warning, "Entity {EntityName} not found", InName);
	}
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetTagEntity(const FGameplayTag& Tag) const
{
	solid_checkf(Tag.IsValid(), TEXT("Tag is not valid"));

	solid_checkf(GetFlecsWorld()->TagEntityMap.contains(Tag), TEXT("Tag %s is not registered"), *Tag.ToString());
	solid_checkf(IsAlive(GetFlecsWorld()->TagEntityMap.at(Tag).GetId()), TEXT("Tag entity is not alive"));
		
	return GetAlive(GetFlecsWorld()->TagEntityMap.at(Tag).GetId());
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreatePrefabWithRecord(const FFlecsEntityRecord& InRecord,
	const FString& Name) const
{
	const FFlecsEntityHandle PrefabEntity = GetNativeFlecsWorld_Internal()->prefab(StringCast<char>(*Name).Get());
	solid_checkf(PrefabEntity.IsPrefab(), TEXT("Entity is not a prefab"));
		
	InRecord.ApplyRecordToEntity(this, PrefabEntity);
	
	PrefabEntity.Set<FFlecsEntityRecordComponent>(
	{
		.EntityRecord = InRecord
	});
		
#if WITH_EDITOR

	if (!Name.IsEmpty())
	{
		PrefabEntity.SetDocName(Name);
	}

#endif // WITH_EDITOR
		
	return PrefabEntity;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreatePrefab(const FString& Name) const
{
	return GetNativeFlecsWorld_Internal()->prefab(StringCast<char>(*Name).Get());
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreatePrefab(const TSolidNotNull<UClass*> InClass) const
{
	const FFlecsEntityHandle PrefabEntity = ObtainTypedEntity(InClass)
		.Add(flecs::Prefab);
		
	return PrefabEntity;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::CreatePrefabWithRecord(const FFlecsEntityRecord& InRecord,
	const TSolidNotNull<UClass*> InClass) const
{
	const FFlecsEntityHandle PrefabEntity = CreatePrefab(InClass);
	solid_checkf(PrefabEntity.IsPrefab(), TEXT("Entity is not a prefab"));

	InRecord.ApplyRecordToEntity(this, PrefabEntity);
	PrefabEntity.Set<FFlecsEntityRecordComponent>(
	{
		.EntityRecord = InRecord
	});
		
	return PrefabEntity;
}

FFlecsQueryBuilder UFlecsWorldInterfaceObject::CreateQueryBuilder(const FString& InName) const
{
	return FFlecsQueryBuilder(this, InName);
}

FFlecsQueryBuilder UFlecsWorldInterfaceObject::CreateQueryBuilderWithEntity(const FFlecsEntityHandle& InEntity) const
{
	solid_checkf(InEntity.IsValid(), TEXT("Entity is not valid"));

	return FFlecsQueryBuilder(this, InEntity);
}

FFlecsQuery UFlecsWorldInterfaceObject::CreateQuery(const FFlecsQueryDefinition& InDefinition, const FString& InName) const
{
	flecs::query_builder<> Builder = flecs::query_builder<>(GetNativeFlecsWorld(), StringCast<char>(*InName).Get());
	FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
	InDefinition.Apply(this, BuilderView);
	return FFlecsQuery(Builder.build());
}

FFlecsQuery UFlecsWorldInterfaceObject::CreateQueryWithEntity(const FFlecsQueryDefinition& InDefinition,
	const FFlecsEntityHandle& InEntity) const
{
	flecs::query_builder<> Builder = flecs::query_builder<>(GetNativeFlecsWorld(), InEntity);
	FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
	InDefinition.Apply(this, BuilderView);
	return FFlecsQuery(Builder.build());
}

FFlecsTimerHandle UFlecsWorldInterfaceObject::CreateTimer(const FString& Name) const
{
	return GetNativeFlecsWorld_Internal()->timer(StringCast<char>(*Name).Get());
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetWorldEntity() const
{
	return GetNativeFlecsWorld_Internal()->entity(flecs::World);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetPipeline() const
{
	return GetNativeFlecsWorld_Internal()->get_pipeline();
}

int32 UFlecsWorldInterfaceObject::GetStageCount() const
{
	return GetNativeFlecsWorld_Internal()->get_stage_count();
}

bool UFlecsWorldInterfaceObject::HasGameLoop(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren) const
{
	solid_check(InGameLoop);

	for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : GetFlecsWorld()->GameLoopInterfaces)
	{
		if (GameLoopInterface.GetObject()->GetClass() == InGameLoop)
		{
			return true;
		}
		else if (bAllowChildren && GameLoopInterface.GetObject()->GetClass()->IsChildOf(InGameLoop))
		{
			return true;
		}
	}
		
	return false;
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetGameLoopEntity(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren) const
{
	solid_check(InGameLoop);
	
	for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : GetFlecsWorld()->GameLoopInterfaces)
	{
		if (GameLoopInterface.GetObject()->GetClass() == InGameLoop ||
			(bAllowChildren && GameLoopInterface.GetObject()->GetClass()->IsChildOf(InGameLoop)))
		{
			return GameLoopInterface->GetEntityHandle();
		}
	}

	// @TODO: Log warning?
	return FFlecsEntityHandle();
}

UObject* UFlecsWorldInterfaceObject::GetGameLoop(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren) const
{
	solid_check(IsValid(InGameLoop));
		
	for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : GetFlecsWorld()->GameLoopInterfaces)
	{
		if (GameLoopInterface.GetObject()->GetClass() == InGameLoop ||
			(bAllowChildren && GameLoopInterface.GetObject()->GetClass()->IsChildOf(InGameLoop)))
		{
			return GameLoopInterface.GetObject();
		}
	}

	// @TODO: Log warning?
	return nullptr;
}

int32 UFlecsWorldInterfaceObject::GetThreads() const
{
	return GetNativeFlecsWorld_Internal()->get_threads();
}

bool UFlecsWorldInterfaceObject::UsingTaskThreads() const
{
	return GetNativeFlecsWorld_Internal()->using_task_threads();
}

UObject* UFlecsWorldInterfaceObject::GetRegisteredFlecsObject(const TSubclassOf<UObject> InClass) const
{
	solid_check(InClass);
	
	if (!GetFlecsWorld()->RegisteredObjectTypes.Contains(InClass))
	{
		return nullptr;
	}

	return GetFlecsWorld()->RegisteredObjectTypes[InClass].GetObject();
}

UObject* UFlecsWorldInterfaceObject::GetRegisteredFlecsObjectChecked(const TSubclassOf<UObject> InClass) const
{
	solid_check(InClass);
	
	solid_checkf(GetFlecsWorld()->RegisteredObjectTypes.Contains(InClass),
		TEXT("Class %s is not registered as a flecs object"), *InClass->GetName());

	return GetFlecsWorld()->RegisteredObjectTypes[InClass].GetObject();
}

bool UFlecsWorldInterfaceObject::IsFlecsObjectRegistered(const TSubclassOf<UObject> InClass) const
{
	solid_check(InClass);
	return GetFlecsWorld()->RegisteredObjectTypes.Contains(InClass);
}

FFlecsEntityHandle UFlecsWorldInterfaceObject::GetFlecsModule(const FName& InModuleName) const
{
	FFlecsEntityHandle ModuleEntity = LookupEntity(InModuleName.ToString());
	
	if UNLIKELY_IF(!ModuleEntity.IsValid() || !ModuleEntity.Has(flecs::Module))
	{
		UE_LOGFMT(LogFlecsWorld, Warning,
			"Module {ModuleName} does not exist or is not a valid flecs module",
			*InModuleName.ToString());
		
		return FFlecsEntityHandle::GetNullHandle();
	}
	
	return ModuleEntity;
}
