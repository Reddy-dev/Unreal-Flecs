// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppDeclaratorNeverUsed
#include "Worlds/FlecsWorld.h"

#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetBundleData.h"
#include "AssetRegistry/AssetData.h"

#include "Math/TransformCalculus2D.h"
#include "Math/Color.h"
#include "Math/MathFwd.h"

#include "StructUtils/InstancedStruct.h"
#include "StructUtils/InstancedStructContainer.h"
#include "StructUtils/SharedStruct.h"

#include "Kismet/GameplayStatics.h"

#include "Worlds/FlecsWorldSubsystem.h"

#include "Logs/FlecsCategories.h"

#include "EntityRecords/FlecsEntityRecord.h"

#include "Components/FlecsAddReferencedObjectsTrait.h"
#include "Components/FlecsBeginPlayComponent.h"
#include "Components/FlecsUObjectComponent.h"
#include "Entities/FlecsEntityRange.h"
#include "Entities/FlecsTableHandle.h"
#include "EntityRecords/FlecsEntityRecordComponent.h"

#include "GameFramework/WorldSettings.h"

#include "General/FlecsDeveloperSettings.h"
#include "General/FlecsGameplayTagManagerEntity.h"
#include "General/FlecsObjectRegistrationInterface.h"
#include "General/FlecsObjectRegistrationProviderBase.h"
#include "Interfaces/IPluginManager.h"

#include "Worlds/FlecsStage.h"

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include "Queries/FlecsQueryBuilder.h"

#include "Pipelines/FlecsGameLoopInterface.h"
#include "Pipelines/FlecsGameLoopTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsWorld)

// static bool GFlecs_bRegisterMemberTypeStructs = false;
// FAutoConsoleVariable CVarFlecsRegisterMemberTypeStructs(
// 	TEXT("Flecs.RegisterMemberTypeStructs"),
// 	GFlecs_bRegisterMemberTypeStructs,
// 	TEXT("Register the member type structs as components if not previously registered."),
// 	ECVF_Default);

DECLARE_STATS_GROUP(TEXT("FlecsWorld"), STATGROUP_FlecsWorld, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("FlecsWorld::Progress"),
	STAT_FlecsWorldProgress, STATGROUP_FlecsWorld);

DECLARE_CYCLE_STAT(TEXT("FlecsWorld::Progress::ProgressModule"),
	STAT_FlecsWorldProgressModule, STATGROUP_FlecsWorld);

UFlecsWorld::UFlecsWorld(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const FString ObjectName = GetName();
	const char* ObjectNameCStr = StringCast<char>(*ObjectName).Get();
	char* argv[] = { const_cast<char*>(ObjectNameCStr) };
		
	World = flecs::world(1, argv);
}

UFlecsWorld::~UFlecsWorld()
{
	for (TTuple<FName, TObjectPtr<UFlecsEntityRange>>& Pair : EntityRanges)
	{
		if (UFlecsEntityRange* EntityRange = Pair.Get<1>())
		{
			EntityRange->InvalidateNativeEntityRange();
		}
	}
	
	if LIKELY_IF(World)
	{
		World.release();
		World.world_ = nullptr;
	}

	FCoreUObjectDelegates::GarbageCollectComplete.RemoveAll(this);
}

UFlecsWorld* UFlecsWorld::GetDefaultWorld(const TSolidNotNull<const UObject*> WorldContextObject)
{
	const TSolidNotNull<const UWorld*> GameWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	
	const TSolidNotNull<const UFlecsWorldSubsystem*> WorldSubsystem = GameWorld->GetSubsystemChecked<UFlecsWorldSubsystem>();
	
	return WorldSubsystem->GetDefaultWorld();
}

void UFlecsWorld::WorldStart()
{
	UE_LOGFMT(LogFlecsWorld, Log, "Flecs World started: {WorldObjectName}", *GetName());

	InitializeFlecsRegistrationObjects();

	const TSolidNotNull<const UFlecsDeveloperSettings*> FlecsDeveloperSettings = GetDefault<UFlecsDeveloperSettings>();
	
	if (FlecsDeveloperSettings->bDeleteEmptyTablesOnGC)
	{
		DeleteEmptyTablesGCDelegateHandle = FCoreUObjectDelegates::GarbageCollectComplete
			.AddWeakLambda(this, [this, FlecsDeveloperSettings]()
			{
				if UNLIKELY_IF(!bIsInitialized)
				{
					return;
				}
				
				UE_LOGFMT(LogFlecsWorld, Verbose,
							  "Flecs World {WorldName} Deleting empty tables on GC",
							  *GetName());

				const int32 DeletedTables = DeleteEmptyTables(
					FlecsDeveloperSettings->TimeBudget,
					FlecsDeveloperSettings->ClearGeneration,  // NOLINT(clang-diagnostic-implicit-int-conversion)
					FlecsDeveloperSettings->DeleteGeneration); // NOLINT(clang-diagnostic-implicit-int-conversion)
				
				UE_LOGFMT(LogFlecsWorld, Verbose,
							  "Flecs World {WorldName} Deleted {DeletedTableCount} empty tables on GC",
							  *GetName(),
							  DeletedTables);
			});
	}
}

void UFlecsWorld::WorldBeginPlay()
{
	Add<FFlecsBeginPlayComponent>();
}

void UFlecsWorld::InitializeDefaultComponents() const
{
	//World.component<FFlecsEntityHandle>()
	//	.disable();
		
	RegisterComponentType<FString>(false)
		.Opaque(flecs::String)
		.serialize([](const flecs::serializer* Serializer, const FString* Data)
		 {
			 const TCHAR* CharArray = Data->GetCharArray().GetData();
			 return Serializer->value(flecs::String, &CharArray);
		 })
		 .assign_string([](FString* Data, const char* String)
		 {
			 *Data = String;
		 });
	
	/*World.component<FString>()
	     .opaque(flecs::String)
	     .serialize([](const flecs::serializer* Serializer, const FString* Data)
	     {
		     const TCHAR* CharArray = Data->GetCharArray().GetData();
		     return Serializer->value(flecs::String, &CharArray);
	     })
	     .assign_string([](FString* Data, const char* String)
	     {
		     *Data = String;
	     });*/

	RegisterComponentType<FName>(false)
	     .Opaque(flecs::String)
	     .serialize([](const flecs::serializer* Serializer, const FName* Data)
	     {
		     const FString String = Data->ToString();
		     const TCHAR* CharArray = String.GetCharArray().GetData();
	     	
		     return Serializer->value(flecs::String, &CharArray);
	     })
	     .assign_string([](FName* Data, const char* String)
	     {
		     *Data = FName(String);
	     });

	RegisterComponentType<FText>(false)
	     .Opaque(flecs::String)
	     .serialize([](const flecs::serializer* Serializer, const FText* Data)
	     {
		     const FString String = Data->ToString();
		     const TSolidNotNull<const TCHAR*> CharArray = String.GetCharArray().GetData();
	     	
		     return Serializer->value(flecs::String, &CharArray);
	     })
	     .assign_string([](FText* Data, const char* String)
	     {
		     *Data = FText::FromString(String);
	     });

	RegisterComponentType<std::string>(false)
		 .Opaque(flecs::String)
		 .serialize([](const flecs::serializer* Serializer, const std::string* Data)
		 {
			 const char* CharArray = Data->c_str();
			 return Serializer->value(flecs::String, &CharArray);
		 })
		 .assign_string([](std::string* Data, const char* String)
		 {
			 *Data = String;
		 });

	RegisterComponentType<FGameplayTag>(true, false)
	     .Opaque(flecs::Entity)
	     .serialize([](const flecs::serializer* Serializer, const FGameplayTag* Data)
	     {
		     const FFlecsId TagEntity = ecs_lookup_path_w_sep(
			     Serializer->world,
			     flecs::component<FFlecsGameplayTagManagerEntity>(const_cast<flecs::world_t*>(Serializer->world)),
			     StringCast<char>(*Data->ToString()).Get(),
			     ".",
			     nullptr,
			     true);
		 		
		     return Serializer->value(flecs::Entity, &TagEntity);
	     });
		
	RegisterComponentType<FObjectPtr>(false)
	     .Opaque(flecs::Uptr)
	     .serialize([](const flecs::serializer* Serializer, const FObjectPtr* Data)
	     {
		     const UObject* Object = Data->Get();
		     return Serializer->value(flecs::Uptr, std::addressof(Object));
	     });
		
	RegisterComponentType<FWeakObjectPtr>(false)
	     .Opaque(flecs::Uptr)
	     .serialize([](const flecs::serializer* Serializer, const FWeakObjectPtr* Data)
	     {
		     const UObject* Object = Data->Get();
		     return Serializer->value(flecs::Uptr, std::addressof(Object));
	     })
	     .assign_null([](FWeakObjectPtr* Data)
	     {
		     Data->Reset();
	     });

	RegisterComponentType<FSoftObjectPtr>(false)
	     .Opaque(flecs::Uptr)
	     .serialize([](const flecs::serializer* Serializer, const FSoftObjectPtr* Data)
	     {
		     const UObject* Object = Data->Get();
		     return Serializer->value(flecs::Uptr, std::addressof(Object));
	     })
	     .assign_null([](FSoftObjectPtr* Data)
	     {
		     Data->Reset();
	     });

	RegisterComponentType<TSubclassOf<UObject>>(false)
	     .Opaque(flecs::Uptr)
	     .serialize([](const flecs::serializer* Serializer, const TSubclassOf<UObject>* Data)
	     {
		     const UClass* Class = Data->Get();
		     return Serializer->value(flecs::Uptr, std::addressof(Class));
	     })
	     .assign_null([](TSubclassOf<UObject>* Data)
	     {
		     *Data = nullptr;
	     });

	/*
	World.component<FScriptArray>()
	     .opaque<flecs::Vector>(flecs::meta::VectorType)
	     .serialize_element([](const flecs::serializer* Serializer, void* ElementPtr, size_t ElementSize) -> int
	     {
		     
	     });*/
	/* World.component<FScriptMap>()
	     .opaque<flecs::Map>(flecs::meta::MapType); */

	RegisterUnrealTypes();
	
	Scope(GetFlecsModule(FName("UnrealFlecs")), [this]()
	{
		RegisterComponentType<FFlecsAddReferencedObjectsTrait>(true, false)
			.Add(flecs::Trait);
	});
	
	const TSolidNotNull<UFlecsTypeRegistryEngineSubsystem*> FlecsTypeRegistry
		= GEngine->GetEngineSubsystem<UFlecsTypeRegistryEngineSubsystem>();
	
	FlecsTypeRegistry->RegisterAllTypes(this);
}

void UFlecsWorld::InitializeFlecsRegistrationObjects()
{
	for (const UFlecsObjectRegistrationProviderBase* Provider : UFlecsObjectRegistrationProviderBase::IterateProviders())
	{
		solid_cassume(Provider);
		
		for (const TSubclassOf<UObject>& RegisteredClass : Provider->GetClassesToRegister())
		{
			if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(RegisteredClass), 
				TEXT("Invalid class in Flecs registration provider: %s"), *GetNameSafe(RegisteredClass)))
			{
				continue;
			}
		
			if UNLIKELY_IF(IsFlecsObjectRegistered(RegisteredClass))
			{
				UE_LOGFMT(LogFlecsWorld, Error,
						  "Flecs World {WorldName} Object class {ClassName} is already registered, skipping",
						  *GetName(),
						  *RegisteredClass->GetName());
				continue;
			}
		
			RegisterFlecsObject(RegisteredClass);
		}
	}
}

void UFlecsWorld::CallBeginPlayForRegisteredObjects()
{
	for (const TScriptInterface<IFlecsObjectRegistrationInterface>& RegisteredObject : RegisteredObjects)
	{
		const TSolidNotNull<const UObject*> RegisteredObjectUObject = RegisteredObject.GetObject();
		const TSolidNotNull<IFlecsObjectRegistrationInterface*> RegisteredInterface = RegisteredObject.GetInterface();
		
		ExecuteInRegistrationScope(RegisteredObjectUObject, RegisteredInterface,
			[this, RegisteredInterface]()
			{
				RegisteredInterface->FlecsWorldBeginPlay(this);
			});
	}
}

void UFlecsWorld::ExecuteInRegistrationScope(
	const TSolidNotNull<const UObject*> InObject,
	const TSolidNotNull<const IFlecsObjectRegistrationInterface*> InObjectRegistrationInterface,
	TFunctionRef<void()> InFunction)
{
	EUnrealFlecsRegistrationScopeType ScopeType = InObjectRegistrationInterface->GetRegistrationScopeType();
	
	FString ResolvedName = InObjectRegistrationInterface->GetScopeName();
	if (ResolvedName.IsEmpty())
	{
		auto [OutName, OutScopeType] 
			= UE::Flecs::Registration::ResolveScopeTypeName(InObject, ScopeType);
		ResolvedName = OutName;
		ScopeType = OutScopeType;
	}
	
	const FFlecsId ScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(this, 
		ResolvedName, ScopeType);
	
	FFlecsEntityHandle ScopeEntity;
	
	if (ScopeId.IsValid())
	{
		ScopeEntity = ScopeId.ToHandle<FFlecsEntityHandle>(GetNativeFlecsWorld());
	}
	else
	{
		ScopeEntity = FFlecsEntityHandle::GetNullHandle();
	}

	const FFlecsId OldScope = ScopeEntity.IsValid() ? SetScope(ScopeEntity) : FFlecsId::Null();

	InFunction();

	SetScope(OldScope);
}

void UFlecsWorld::RegisterUnrealTypes() const
{
	RegisterComponentType<FGameplayTagContainer>(false);
	
	RegisterComponentType<FVector>(false);
	RegisterComponentType<FQuat>(false);
	RegisterComponentType<FRotator>(false);
	RegisterComponentType<FTransform>(false);
	
	RegisterComponentType<FBox>(false);
	RegisterComponentType<FBoxSphereBounds>(false);
	RegisterComponentType<FCapsuleShape>(false);
	RegisterComponentType<FRay>(false);
	RegisterComponentType<FPlane>(false);
	RegisterComponentType<FMatrix>(false);

	RegisterComponentType<FVector4>(false);
		
	RegisterComponentType<FVector2D>(false);
	RegisterComponentType<FQuat2D>(false);
	RegisterComponentType<FTransform2D>(false);
	RegisterComponentType<FBox2D>(false);
	
	RegisterComponentType<FIntVector>(false);
	RegisterComponentType<FIntVector4>(false);
	RegisterComponentType<FIntPoint>(false);
	RegisterComponentType<FIntRect>(false);
	
	RegisterComponentType<FRandomStream>(false);

	RegisterComponentType<FColor>(false);
	RegisterComponentType<FLinearColor>(false);

	RegisterComponentType<FPrimaryAssetType>(false);
	RegisterComponentType<FPrimaryAssetId>(false);

	RegisterComponentType<FTopLevelAssetPath>(false);
	RegisterComponentType<FSoftClassPath>(false);
	RegisterComponentType<FSoftObjectPath>(false);

	RegisterComponentType<FAssetData>(false);
	RegisterComponentType<FAssetBundleData>(false);

	RegisterComponentType<FGuid>(false);
	RegisterComponentType<FTimespan>(false);
	RegisterComponentType<FDateTime>(false);

	RegisterComponentType<FFloatRangeBound>(false);
	RegisterComponentType<FInt8RangeBound>(false);
	RegisterComponentType<FInt16RangeBound>(false);
	RegisterComponentType<FInt32RangeBound>(false);
	RegisterComponentType<FInt64RangeBound>(false);
	
	RegisterComponentType<FFloatRange>(false);
	RegisterComponentType<FInt32Range>(false);
	RegisterComponentType<FInt64Range>(false);
	
	RegisterComponentType<FFrameNumber>(false);
	RegisterComponentType<FFrameRate>(false);

	// @TODO: make this opaque?
	RegisterComponentType<FInstancedStruct>(false);
	RegisterComponentType<FInstancedStructContainer>(false);
	RegisterComponentType<FSharedStruct>(false);
}

void UFlecsWorld::InitializeComponentPropertyObserver()
{
	ComponentRegisteredDelegateHandle = UE::FlecsLibrary::GetTypeRegisteredDelegate().AddWeakLambda(this,
		[this](flecs::world_t* InWorld, const flecs::id_t InEntityId)
	{
		if UNLIKELY_IF(!IsValid(this))
		{
			return;
		}
			
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
			
		// because in PIE and automation tests we may have multiple worlds, and this is a global delegate
		if (InWorld != GetNativeFlecsWorld())
		{
			return;
		}
			
#endif // WITH_AUTOMATION_TESTS || WITH_EDITOR
			
		static const FString USTRUCTAliasPrefix = TEXT("UScriptStruct_");
		static const FString UENUMAliasPrefix = TEXT("UEnum_");
			
		solid_checkf(!IsDeferred(), TEXT("Cannot register component properties while world is deferred."));
		
		const FFlecsEntityHandle EntityHandle = FFlecsEntityHandle(GetNativeFlecsWorld(), InEntityId);
			
		const bool bIsScriptStructComponent = EntityHandle.Has<FFlecsScriptStructComponent>();
		const bool bIsScriptEnumComponent = EntityHandle.Has<FFlecsScriptEnumComponent>();
			
		const FString StructSymbol = EntityHandle.GetSymbol();
		solid_checkf(!StructSymbol.IsEmpty(),TEXT("Registered component has no symbol"));
			
		const TSolidNotNull<const UFlecsTypeRegistryEngineSubsystem*> FlecsTypeRegistry
				= GEngine->GetEngineSubsystem<UFlecsTypeRegistryEngineSubsystem>();
						
		if (FlecsTypeRegistry->IsComponentRegistered(StructSymbol))
		{
			FFlecsComponentHandle InUntypedComponent = EntityHandle.GetUntypedComponent_Unsafe();
							
			const FFlecsComponentPropertiesDefinition* Properties = FlecsTypeRegistry->GetRegisteredComponentProperties(StructSymbol);

			if UNLIKELY_IF(!Properties->PropertiesFunction)
			{
				UE_LOGFMT(LogFlecsComponent, Log,
					"Component properties {StructName} registration function is null",
					StructSymbol);
				return;
			}

			std::invoke(Properties->PropertiesFunction, this, InUntypedComponent, *Properties);

			UE_LOGFMT(LogFlecsComponent, Log,
				"Component properties {StructName} registered", StructSymbol);
		}
		#if !NO_LOGGING
		else
		{
			UE_LOGFMT(LogFlecsComponent, Log,
				"Component properties {StructName} not found", StructSymbol);
		}
		#endif // UNLOG_ENABLED
			
		if (bIsScriptStructComponent)
		{
			const TSolidNotNull<const UScriptStruct*> ScriptStruct
				= EntityHandle.Get<FFlecsScriptStructComponent>().ScriptStruct.Get();
			
			const FString ScriptStructAlias = USTRUCTAliasPrefix + ScriptStruct->GetStructCPPName();
			solid_checkf(ScriptStructAlias.IsEmpty() == false, TEXT("Script struct alias is empty"));
			
			EntityHandle.SetAlias(StringCast<char>(*ScriptStructAlias).Get());
		}
			
		if (bIsScriptEnumComponent)
		{
			const TSolidNotNull<const UEnum*> ScriptEnum
				= EntityHandle.Get<FFlecsScriptEnumComponent>().ScriptEnum.Get();
			
			const FString ScriptEnumAlias = UENUMAliasPrefix + ScriptEnum->GetName();
			solid_checkf(ScriptEnumAlias.IsEmpty() == false, TEXT("Script enum alias is empty"));
			
			EntityHandle.SetAlias(StringCast<char>(*ScriptEnumAlias).Get());
		}
	});

}

void UFlecsWorld::InitializeSystems()
{
		ObjectComponentQuery = CreateQueryBuilder<FFlecsUObjectComponent>("ObjectComponentQuery")
			.TermAt(0).Second(flecs::Wildcard) // FFlecsUObjectComponent
			.Build();
	
		AddReferencedObjectsQuery = CreateQueryBuilder<const FFlecsScriptStructComponent>("AddReferencedObjectsQuery") // 0 (FFlecsScriptStructComponent)
			.TermAt(0).Src("$Component") // 0
			.With<FFlecsAddReferencedObjectsTrait>().Src("$Component") //  1
			.With("$Component").Or() // 2
			.WithPair("$Component", flecs::Wildcard).Or() // 2
			.WithPair(flecs::Wildcard, "$Component") // 2
			.Build();

		FCoreUObjectDelegates::GarbageCollectComplete.AddWeakLambda(this, [this]
		{
			Defer([this]()
			{
				ObjectComponentQuery.each([](flecs::iter& Iter, size_t Index, const FFlecsUObjectComponent& InUObjectComponent)
				{
					const FFlecsEntityHandle EntityHandle = Iter.entity(Index);
						
					if (!InUObjectComponent.IsValid())
					{
						UE_CLOGFMT(EntityHandle.HasName(), LogFlecsWorld, Verbose,
							"Entity Garbage Collected: {EntityName}", EntityHandle.GetName());
						
						EntityHandle.Destroy();
					}
				});
			});
		});
}

void UFlecsWorld::Reset()
{
	for (TTuple<FName, TObjectPtr<UFlecsEntityRange>> EntityRange : EntityRanges)
	{
		if LIKELY_IF(UFlecsEntityRange* EntityRangePtr = EntityRange.Value)
		{
			EntityRangePtr->InvalidateNativeEntityRange();
			EntityRangePtr->MarkAsGarbage();
		}
	}
	
	EntityRanges.Empty();
	GetNativeFlecsWorld().reset();
}

void UFlecsWorld::ResetClock() const
{
	GetNativeFlecsWorld().reset_clock();
}

bool UFlecsWorld::BeginReadOnly() const
{
	return GetNativeFlecsWorld().readonly_begin();
}

void UFlecsWorld::EndReadOnly() const
{
	GetNativeFlecsWorld().readonly_end();
}

void UFlecsWorld::SetContext(void* InContext) const
{
	GetNativeFlecsWorld().set_ctx(InContext);
}

void UFlecsWorld::AddDeferredRegisteredObject(const TSubclassOf<UObject>& InClass,
	const TArray<TSubclassOf<UObject>>& InDependencies)
{
	for (const TSubclassOf<UObject>& Dependency : InDependencies)
	{
		TArray<TSubclassOf<UObject>>& Dependents = DeferredRegisteredObjectsDependants.FindOrAdd(Dependency);
		Dependents.Add(InClass);
	}
}

void UFlecsWorld::HandleWorldPause()
{
	const bool bIsPaused = GetWorld()->IsPaused();
	const bool bHasSavedTimeScale = PrePauseTimeScale.IsSet();
	const bool bTimeScaleZero = FMath::IsNearlyZero(GetTimeScale());

	// World is paused and we don't have a saved time scale -> save time scale and set to 0
	if (bIsPaused && !bHasSavedTimeScale)
	{
		const double CurrentScale = bTimeScaleZero ? 1.0 : GetTimeScale();
		PrePauseTimeScale = CurrentScale;
		SetTimeScale(0.0);
		return;
	}

	// World is unpaused and we have a saved time scale -> restore time scale
	if (!bIsPaused && bHasSavedTimeScale)
	{
		SetTimeScale(PrePauseTimeScale.GetValue());
		PrePauseTimeScale.Reset();
		return;
	}

	// Time scale became zero while world is unpaused -> pause world
	if (!bIsPaused && bTimeScaleZero)
	{
		UGameplayStatics::SetGamePaused(this, true);
		return;
	}

	// Time scale became non-zero while world is paused -> unpause world
	if (bIsPaused && !bTimeScaleZero)
	{
		UGameplayStatics::SetGamePaused(this, false);
		return;
	}
}

bool UFlecsWorld::Progress(const double DeltaTime)
{
	return GetNativeFlecsWorld().progress(DeltaTime);
}

double UFlecsWorld::SetTimeScale(const double InTimeScale) const
{
	GetNativeFlecsWorld().set_time_scale(InTimeScale);
	return GetTimeScale();
}

void UFlecsWorld::DestroyWorld()
{
	if UNLIKELY_IF(ShouldQuit())
	{
		UE_LOGFMT(LogFlecsWorld, Warning, "World is already being destroyed: {WorldObjectName}", *GetName());
		return;
	}

	FCoreUObjectDelegates::GarbageCollectComplete.RemoveAll(this);
	
	ObjectComponentQuery.Destroy();
	AddReferencedObjectsQuery.Destroy();

	CallUnregisterOnRegisteredObjects();

	for (TTuple<FName, TObjectPtr<UFlecsEntityRange>> EntityRange : EntityRanges)
	{
		if (UFlecsEntityRange* EntityRangePtr = EntityRange.Value)
		{
			EntityRangePtr->InvalidateNativeEntityRange();
			EntityRangePtr->MarkAsGarbage();
		}
	}
	
	EntityRanges.Empty();
		
	World.release();
	World.world_ = nullptr;
	MarkAsGarbage();
}

void UFlecsWorld::SetPipeline(const FFlecsPipelineHandle& InPipeline) const
{
	GetNativeFlecsWorld().set_pipeline(InPipeline);
}

void UFlecsWorld::SetStageCount(const int32 InStageCount)
{
	GetNativeFlecsWorld().set_stage_count(InStageCount);
	
	RegisterStages(InStageCount);
}

void UFlecsWorld::PreallocateEntities(const int32 InEntityCount) const
{
	if UNLIKELY_IF(ensureAlwaysMsgf(InEntityCount > 0,
		TEXT("Entity count must be greater than 0")))
	{
		return;
	}
		
	GetNativeFlecsWorld().dim(InEntityCount);
}

void UFlecsWorld::SetThreads(const int32 InThreadCount)
{
	GetNativeFlecsWorld().set_threads(InThreadCount);
	
	RegisterStages(InThreadCount);
}

void UFlecsWorld::SetTaskThreads(const int32 InThreadCount)
{
	GetNativeFlecsWorld().set_task_threads(InThreadCount);
	
	RegisterStages(InThreadCount);
}

UFlecsEntityRange* UFlecsWorld::CreateEntityRange(const FName& InRangeName, const int32 InMinimum, const int32 InMaximum)
{
	solid_cassumef(!InRangeName.IsNone(), TEXT("Entity range name must not be None"));
	solid_cassumef(InMinimum > 0, TEXT("Entity range minimum must be greater than 0"));
	solid_cassumef(InMaximum == 0 || InMaximum >= InMinimum,
		TEXT("Entity range maximum must be greater than or equal to minimum, or 0 for unbounded"));
	
	const ecs_entity_range_t* NativeEntityRange = ecs_entity_range_new(GetNativeFlecsWorld(), InMinimum, InMaximum);
	
	if UNLIKELY_IF(!NativeEntityRange)
	{
		return nullptr;
	}
	
	UFlecsEntityRange* EntityRange = TrackEntityRange(NativeEntityRange, InRangeName);

	return EntityRange;
}

void UFlecsWorld::SetActiveEntityRange(UFlecsEntityRange* InEntityRange) const
{
	solid_check(IsValid(InEntityRange));
	solid_checkf(InEntityRange->GetTypedOuter<UFlecsWorld>() == this,
		TEXT("Entity range belongs to a different Flecs world"));
	solid_check(InEntityRange->GetNativeEntityRange());
	
	ecs_entity_range_set(GetNativeFlecsWorld(), InEntityRange->GetNativeEntityRange());
}

UFlecsEntityRange* UFlecsWorld::GetActiveEntityRange() const
{
	return FindTrackedEntityRange(ecs_entity_range_get(GetNativeFlecsWorld()));
}

TArray<UFlecsEntityRange*> UFlecsWorld::GetEntityRanges() const
{
	TArray<UFlecsEntityRange*> Ranges;
	Ranges.Reserve(EntityRanges.Num());
	
	for (const TTuple<FName, TObjectPtr<UFlecsEntityRange>>& EntityRangePair : EntityRanges)
	{
		if (UFlecsEntityRange* EntityRange = EntityRangePair.Value)
		{
			Ranges.Add(EntityRange);
		}
	}
	
	return Ranges;
}

std::generator<UFlecsEntityRange*> UFlecsWorld::GetEntityRangesGenerator() const
{
	for (const TTuple<FName, TObjectPtr<UFlecsEntityRange>>& EntityRangePair : EntityRanges)
	{
		if (UFlecsEntityRange* EntityRange = EntityRangePair.Value)
		{
			co_yield EntityRange;
		}
	}
}

void UFlecsWorld::RunPipeline(const FFlecsId InPipeline, const double DeltaTime) const
{
	solid_checkf(IsAlive(InPipeline), TEXT("Pipeline entity is not alive"));
	
	GetNativeFlecsWorld().run_pipeline(InPipeline, DeltaTime);
}

/*
FFlecsQuery UFlecsWorld::GetQueryFromEntity(const FFlecsEntityHandle& InEntity) const
{
	solid_checkf(InEntity.IsValid(), TEXT("Entity is not valid"));
	solid_checkf(poly(InEntity, ecs_query_t), TEXT("Entity is not a query"));

	return InEntity.Get<
}
*/

void UFlecsWorld::ShrinkWorld() const
{
	GetNativeFlecsWorld().shrink();
}

int32 UFlecsWorld::DeleteEmptyTables(const double TimeBudgetSeconds,
                                     const uint16 ClearGeneration,
                                     const uint16 DeleteGeneration) const
{
	ecs_delete_empty_tables_desc_t Desc;
	Desc.clear_generation = ClearGeneration;
	Desc.delete_generation = DeleteGeneration;
	Desc.time_budget_seconds = TimeBudgetSeconds;
	
	return ecs_delete_empty_tables(GetNativeFlecsWorld(), &Desc);
}

UObject* UFlecsWorld::RegisterFlecsObject(const TSubclassOf<UObject> InClass)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(InClass), TEXT("Invalid class to register")))
	{
		return nullptr;
	}
	
	if UNLIKELY_IF(!InClass->ImplementsInterface(UFlecsObjectRegistrationInterface::StaticClass()))
	{
		UE_LOGFMT(LogFlecsWorld, Error,
			"Class {ClassName} does not implement IFlecsObjectRegistrationInterface, cannot register",
			*InClass->GetName());
		return nullptr;
	}
	
	if UNLIKELY_IF(RegisteredObjectTypes.Contains(InClass))
	{
		UE_LOGFMT(LogFlecsWorld, Warning,
			"Class {ClassName} is already registered, returning existing instance",
			*InClass->GetName());
		return RegisteredObjectTypes[InClass].GetObject();
	}
	
	const TSolidNotNull<const UObject*> CDO = InClass.GetDefaultObject();
	const TSolidNotNull<const IFlecsObjectRegistrationInterface*> CDOInterface = CastChecked<const IFlecsObjectRegistrationInterface>(CDO);
	
	TArray<TSubclassOf<UObject>> DependentRegisteredObjectTypes = CDOInterface->GetDependentRegistrationClasses();
	for (const TSubclassOf<UObject>& DependentClass : DependentRegisteredObjectTypes)
	{
		solid_check(DependentClass != InClass);
		
		if (!IsFlecsObjectRegistered(DependentClass))
		{
			AddDeferredRegisteredObject(InClass, DependentRegisteredObjectTypes);
			return nullptr;
		}
	}
	
	const TSolidNotNull<UObject*> FlecsObject = NewObject<UObject>(this, InClass);
	const TSolidNotNull<IFlecsObjectRegistrationInterface*> FlecsObjectInterface = CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject);
	
	if (!FlecsObjectInterface->ShouldAutoRegisterWithWorld(this))
	{
		FlecsObject->MarkAsGarbage();
		return nullptr;
	}
	
	if (!UE::Flecs::Net::ShouldRegisterInWorld(GetWorld(), 
		static_cast<EFlecsObjectRegistrationNetworkFlags>(FlecsObjectInterface->GetObjectRegistrationNetworkFlags())))
	{
		FlecsObject->MarkAsGarbage();
		return nullptr;
	}

	RegisteredObjects.Add(FlecsObject);
	RegisteredObjectTypes.Add(InClass, FlecsObject);
	
	ExecuteInRegistrationScope(FlecsObject, FlecsObjectInterface,
		[this, FlecsObjectInterface]()
		{
			FlecsObjectInterface->RegisterObject(this);

			if (Has<FFlecsBeginPlayComponent>())
			{
				FlecsObjectInterface->FlecsWorldBeginPlay(this);
			}
		});

	if (TArray<TSubclassOf<UObject>>* Dependents = DeferredRegisteredObjectsDependants.Find(InClass))
	{
		for (const TSubclassOf<UObject>& Dependent : *Dependents)
		{
			solid_check(Dependent != InClass);
			
			if UNLIKELY_IF(IsFlecsObjectRegistered(Dependent))
			{
				UE_LOGFMT(LogFlecsWorld, Warning,
					"Dependent class {DependentClassName} is already registered, skipping",
					*Dependent->GetName());
				continue;
			}
			
			RegisterFlecsObject(Dependent);
		}
	}
	
	DeferredRegisteredObjectsDependants.Remove(InClass);
	
	return FlecsObject;
}

bool UFlecsWorld::UnregisterFlecsObject(const TSubclassOf<UObject>& InClass)
{
	if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(InClass), TEXT("Invalid class to unregister")))
	{
		return false;
	}
	
	for (int32 Index = RegisteredObjects.Num() - 1; Index >= 0; --Index)
	{
		const TScriptInterface<IFlecsObjectRegistrationInterface> RegisteredObject = RegisteredObjects[Index];
		
		if UNLIKELY_IF(!RegisteredObject)
		{
			continue;
		}
		
		if (RegisteredObject.GetObject()->GetClass() == InClass)
		{
			RegisteredObject->UnregisterObject(this);
			
			RegisteredObjects.RemoveAt(Index);
			RegisteredObjectTypes.Remove(InClass);
			return true;
		}
	}
	
	return false;
}

UFlecsStage* UFlecsWorld::GetStage(const int32 InStageId) const
{
	solid_cassumef(InStageId >= 0, TEXT("Stage ID must be non-negative and can't be the same as the main world (0)"));
	solid_checkf(Stages.IsValidIndex(InStageId), TEXT("Stage ID %d is out of bounds"), InStageId);
	return Stages[InStageId];
}

UFlecsStage* UFlecsWorld::GetStage(const flecs::world& InStageWorld) const
{
	if (!InStageWorld.is_stage())
	{
		return nullptr;
	}
	
	const int32 StageId = InStageWorld.get_stage_id();
	return GetStage(StageId);
}

UFlecsStage* UFlecsWorld::GetStage(const flecs::iter& InIter) const
{
	if (!InIter.world().is_stage())
	{
		return nullptr;
	}
	
	const int32 StageId = InIter.world().get_stage_id();
	return GetStage(StageId);
}

TTuple<int32, FFlecsId> UFlecsWorld::Search(const FFlecsTableHandle& InTableHandle, const FFlecsId& InId) const
{
	FFlecsId OutIdTemp;
	const int32 Result = ecs_search(GetNativeFlecsWorld(), InTableHandle.GetTable(), 
		InId, reinterpret_cast<ecs_id_t*>(&OutIdTemp));
	
	return MakeTuple(Result, OutIdTemp);
}

void UFlecsWorld::RegisterStages(const int32 InStageCount)
{
	for (UFlecsStage* Stage : Stages)
	{
		if (Stage)
		{
			Stage->DestroyStage();
		}
	}
	
	Stages.Empty();
	
	const TSolidNotNull<UFlecsStage*> MainStage = NewObject<UFlecsStage>(this);
	MainStage->SetStageWorld(GetNativeFlecsWorld().get_stage(0));
	
	if (InStageCount <= 1)
	{
		Stages.Add(MainStage);
		return;
	}
	
	for (int32 StageIndex = 0; StageIndex < InStageCount; ++StageIndex)
	{
		if (StageIndex == 0)
		{
			Stages.Add(MainStage);
			continue;
		}
		
		const TSolidNotNull<UFlecsStage*> NewStage = NewObject<UFlecsStage>(this);
		NewStage->SetStageWorld(GetNativeFlecsWorld().get_stage(StageIndex));
		
		Stages.Add(NewStage);
		solid_checkf(Stages.Num() - 1 == StageIndex, TEXT("Stage index does not match stage array index"));
		solid_checkf(Stages.Num() - 1 == NewStage->GetStageId(), TEXT("Stage ID does not match stage array index"));
	}
}

TSolidNotNull<UFlecsStage*> UFlecsWorld::CreateAsyncStage()
{
	flecs::world AsyncStage = GetNativeFlecsWorld().async_stage();
	
	const TSolidNotNull<UFlecsStage*> NewStage = NewObject<UFlecsStage>(this);
	NewStage->SetStageWorld(AsyncStage);
	
	return NewStage;
}

UFlecsEntityRange* UFlecsWorld::TrackEntityRange(const TSolidNotNull<const ecs_entity_range_t*> InNativeEntityRange, const FName& InRangeName)
{
	if (UFlecsEntityRange* ExistingEntityRange = FindTrackedEntityRange(InRangeName))
	{
		return ExistingEntityRange;
	}
	
	const TSolidNotNull<UFlecsEntityRange*> NewEntityRange = NewObject<UFlecsEntityRange>(this);
	NewEntityRange->SetNativeEntityRange(InNativeEntityRange, InRangeName);
	EntityRanges.Add(InRangeName, NewEntityRange);
	
	return NewEntityRange;
}

void UFlecsWorld::ImportRestModule()
{
#ifdef FLECS_REST
	
	EndScope([this]()
	{
		uint16 ClientPIEInstanceOffset = 0;
		const TSolidNotNull<const UWorld*> UnrealWorld = GetWorld();
		
#if WITH_EDITOR
		
		if (UnrealWorld->GetNetMode() == NM_Client)
		{
			ClientPIEInstanceOffset = static_cast<uint16>(UE::GetPlayInEditorID());
		}
		
#endif // WITH_EDITOR
		
		const uint16 RestPort = ECS_REST_DEFAULT_PORT + ClientPIEInstanceOffset;
		
		Set<flecs::Rest>(flecs::Rest{ .port = RestPort });
	});
	
#endif // FLECS_REST
}

void UFlecsWorld::ImportStatsModule()
{
#ifdef FLECS_STATS

	EndScope([this]()
	{
		ImportFlecsModule<flecs::stats>();
	});
	
#endif // FLECS_STATS
}

void UFlecsWorld::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);
	
	const TSolidNotNull<UFlecsWorld*> This = CastChecked<UFlecsWorld>(InThis);
	solid_check(IsValid(This));
	
	if UNLIKELY_IF(!This->GetTypeMapComponent() || !This->bIsInitialized)
	{
		return;
	}

	ecs_exclusive_access_begin(This->GetNativeFlecsWorld(), "Garbage Collection ARO");
		
	This->AddReferencedObjectsQuery.each([InThis, &Collector](flecs::iter& Iter, size_t Index,
	                               const FFlecsScriptStructComponent& InScriptStructComponent)
	    {
		    const FFlecsEntityHandle Component = Iter.get_var("Component");
		    solid_check(Component.IsValid());
		
			void* ComponentPtr;

			if (FFlecsId(FFlecsId(Iter.id(2)).GetTypeInfo(Iter.world())->component) == Component)
			{
				ComponentPtr = Iter.field_at(2, Index);
			}
			else
			{
				return;
			}
		    
		    solid_cassume(ComponentPtr);

		    Collector.AddPropertyReferencesWithStructARO(InScriptStructComponent.ScriptStruct.Get(),
		                                    ComponentPtr, InThis);
	    	
	    });

	ecs_exclusive_access_end(This->GetNativeFlecsWorld(), false);
}

void UFlecsWorld::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);
}

void UFlecsWorld::CallUnregisterOnRegisteredObjects()
{
	for (const TScriptInterface<IFlecsObjectRegistrationInterface>& RegisteredObject : RegisteredObjects)
	{
		if LIKELY_IF(RegisteredObject)
		{
			RegisteredObject->UnregisterObject(this);
		}
	}
}

UFlecsEntityRange* UFlecsWorld::FindTrackedEntityRange(const TSolidNotNull<const ecs_entity_range_t*> InNativeEntityRange) const
{
	for (const TTuple<FName, TObjectPtr<UFlecsEntityRange>>& EntityRangePair : EntityRanges)
	{
		if (UFlecsEntityRange* EntityRange = EntityRangePair.Value)
		{
			if (EntityRange->GetNativeEntityRange() == InNativeEntityRange)
			{
				return EntityRange;
			}
		}
	}
	
	return nullptr;
}

UFlecsEntityRange* UFlecsWorld::FindTrackedEntityRange(const FName& InRangeName) const
{
	if UNLIKELY_IF(InRangeName.IsNone())
	{
		return nullptr;
	}
	
	if (const TObjectPtr<UFlecsEntityRange>* FoundEntityRange = EntityRanges.Find(InRangeName))
	{
		return *FoundEntityRange;
	}
	
	return nullptr;
}
