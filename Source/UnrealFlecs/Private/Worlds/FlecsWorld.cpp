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

#include "Types/SolidCppStructOps.h"

#include "Worlds/FlecsWorldSubsystem.h"

#include "Logs/FlecsCategories.h"

#include "EntityRecords/FlecsEntityRecord.h"

#include "Components/FlecsAddReferencedObjectsTrait.h"
#include "Components/FlecsBeginPlayComponent.h"
#include "Components/FlecsUObjectComponent.h"
#include "EntityRecords/FlecsEntityRecordComponent.h"

#include "GameFramework/WorldSettings.h"

#include "General/FlecsDeveloperSettings.h"
#include "General/FlecsGameplayTagManagerEntity.h"
#include "General/FlecsObjectRegistrationInterface.h"
#include "General/FlecsObjectRegistrationProviderBase.h"

#include "Worlds/FlecsStage.h"

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include "Queries/FlecsQueryBuilder.h"
#include "Queries/FlecsQueryBuilderView.h"

#include "Pipelines/FlecsGameLoopInterface.h"
#include "Pipelines/FlecsGameLoopTag.h"
#include "Pipelines/TickFunctions/FlecsTickFunction.h"
#include "Pipelines/TickFunctions/FlecsTickFunctionComponent.h"
#include "Pipelines/TickFunctions/FlecsTickTypeRelationship.h"

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
	if LIKELY_IF(World)
	{
		World.release();
		World.world_ = nullptr;
	}

	FCoreUObjectDelegates::GarbageCollectComplete.RemoveAll(this);
}

UFlecsWorld* UFlecsWorld::GetDefaultWorld(const UObject* WorldContextObject)
{
	solid_cassume(WorldContextObject);

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
		
	RegisterComponentType<FString>()
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

	RegisterComponentType<FName>()
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

	RegisterComponentType<FText>()
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

	RegisterComponentType<std::string>()
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

	RegisterComponentType<FGameplayTag>(false)
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
		
	RegisterComponentType<FObjectPtr>()
	     .Opaque(flecs::Uptr)
	     .serialize([](const flecs::serializer* Serializer, const FObjectPtr* Data)
	     {
		     const UObject* Object = Data->Get();
		     return Serializer->value(flecs::Uptr, std::addressof(Object));
	     });
		
	RegisterComponentType<FWeakObjectPtr>()
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

	RegisterComponentType<FSoftObjectPtr>()
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

	RegisterComponentType<TSubclassOf<UObject>>()
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
		RegisterComponentType<FFlecsAddReferencedObjectsTrait>()
			.Add(flecs::Trait);
	});
	
	const TSolidNotNull<UFlecsTypeRegistryEngineSubsystem*> FlecsTypeRegistry
		= GEngine->GetEngineSubsystem<UFlecsTypeRegistryEngineSubsystem>();
	
	FlecsTypeRegistry->RegisterAllTypes(this);
}

void UFlecsWorld::InitializeFlecsRegistrationObjects()
{
	TSet<TSubclassOf<UObject>> RegisteredObjectClasses;
	
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		if UNLIKELY_IF(!IsValid(Class))
		{
			continue;
		}

		if (!Class->IsChildOf(UFlecsObjectRegistrationProviderBase::StaticClass()))
		{
			continue;
		}

		if (Class == UFlecsObjectRegistrationProviderBase::StaticClass())
		{
			continue;
		}

		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			continue;
		}

		const UFlecsObjectRegistrationProviderBase* ProviderCDO = Class->GetDefaultObject<UFlecsObjectRegistrationProviderBase>();

		if UNLIKELY_IF(!IsValid(ProviderCDO))
		{
			continue;
		}

		const TArray<TSubclassOf<UObject>> ProviderRegisteredObjectClasses = ProviderCDO->GetClassesToRegister();

		RegisteredObjectClasses.Append(ProviderRegisteredObjectClasses);
	}
	
	for (const TSubclassOf<UObject>& RegisteredClass : RegisteredObjectClasses)
	{
		if UNLIKELY_IF(!ensureAlwaysMsgf(IsValid(RegisteredClass), 
			TEXT("Invalid class in Flecs registration provider: %s"), *GetNameSafe(RegisteredClass)))
		{
			continue;
		}
		
		if UNLIKELY_IF(IsFlecsObjectRegistered(RegisteredClass))
		{
			UE_LOGFMT(LogFlecsWorld, Warning,
			          "Flecs World {WorldName} Object class {ClassName} is already registered, skipping",
			          *GetName(),
			          *RegisteredClass->GetName());
			continue;
		}
		
		RegisterFlecsObject(RegisteredClass);
	}
}

void UFlecsWorld::CallBeginPlayForRegisteredObjects()
{
	for (const TScriptInterface<IFlecsObjectRegistrationInterface>& RegisteredObject : RegisteredObjects)
	{
		const bool bRegisterWithModule = RegisteredObject->ShouldRegisterWithModule();
		FName ModuleName = NAME_None;
		
		if (bRegisterWithModule)
		{
			ModuleName = RegisteredObject->GetModuleName();
			
			if (ModuleName.IsNone())
			{
				const FString PackageName = RegisteredObject.GetObject()->GetClass()->GetOuterUPackage()->GetName();
				ModuleName = FName(*FPackageName::GetShortName(PackageName));			
			}
		}
		
		FFlecsEntityHandle ModuleEntity;
		
		if (bRegisterWithModule)
		{
			ModuleEntity = GetFlecsModule(ModuleName);
			
			if UNLIKELY_IF(!ModuleEntity.IsValid() || !ModuleEntity.Has(flecs::Module))
			{
				UE_LOGFMT(LogFlecsWorld, Warning,
					"Module {ModuleName} does not exist or is not a valid flecs module, registered object {ObjectName} will not be registered with the module",
					*ModuleName.ToString(), *RegisteredObject.GetObject()->GetName());
			
				ModuleEntity = FFlecsEntityHandle::GetNullHandle();
			}
		}
		
		FFlecsId OldScope = FFlecsId::Null();
		
		if (bRegisterWithModule && ModuleEntity.IsValid())
		{
			OldScope = SetScope(ModuleEntity);
		}
		
		RegisteredObject->FlecsWorldBeginPlay(this);
		
		SetScope(OldScope);
	}
}

void UFlecsWorld::RegisterUnrealTypes() const
{
	RegisterComponentType<FGameplayTagContainer>();
	
	RegisterComponentType<FVector>();
	RegisterComponentType<FQuat>();
	RegisterComponentType<FRotator>();
	RegisterComponentType<FTransform>();
	
	RegisterComponentType<FBox>();
	RegisterComponentType<FBoxSphereBounds>();
	RegisterComponentType<FCapsuleShape>();
	RegisterComponentType<FRay>();
	RegisterComponentType<FPlane>();
	RegisterComponentType<FMatrix>();

	RegisterComponentType<FVector4>();
		
	RegisterComponentType<FVector2D>();
	RegisterComponentType<FQuat2D>();
	RegisterComponentType<FTransform2D>();
	RegisterComponentType<FBox2D>();
	
	RegisterComponentType<FIntVector>();
	RegisterComponentType<FIntVector4>();
	RegisterComponentType<FIntPoint>();
	RegisterComponentType<FIntRect>();
	
	RegisterComponentType<FRandomStream>();

	RegisterComponentType<FColor>();
	RegisterComponentType<FLinearColor>();

	RegisterComponentType<FPrimaryAssetType>();
	RegisterComponentType<FPrimaryAssetId>();

	RegisterComponentType<FTopLevelAssetPath>();
	RegisterComponentType<FSoftClassPath>();
	RegisterComponentType<FSoftObjectPath>();

	RegisterComponentType<FAssetData>();
	RegisterComponentType<FAssetBundleData>();

	RegisterComponentType<FGuid>();
	RegisterComponentType<FTimespan>();
	RegisterComponentType<FDateTime>();

	RegisterComponentType<FFloatRangeBound>();
	RegisterComponentType<FInt8RangeBound>();
	RegisterComponentType<FInt16RangeBound>();
	RegisterComponentType<FInt32RangeBound>();
	RegisterComponentType<FInt64RangeBound>();
	
	RegisterComponentType<FFloatRange>();
	RegisterComponentType<FInt32Range>();
	RegisterComponentType<FInt64Range>();
	
	RegisterComponentType<FFrameNumber>();
	RegisterComponentType<FFrameRate>();

	// @TODO: make this opaque?
	RegisterComponentType<FInstancedStruct>();
	RegisterComponentType<FInstancedStructContainer>();
	RegisterComponentType<FSharedStruct>();
}

void UFlecsWorld::InitializeComponentPropertyObserver()
{
	ComponentRegisteredDelegateHandle = UE::FlecsLibrary::GetTypeRegisteredDelegate().AddWeakLambda(this,
		[this](const flecs::id_t InEntityId)
	{
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

		TickFunctionQuery = CreateQueryBuilder("TickFunctionQuery")
			.With<FFlecsTickFunctionComponent>().InOutNone() // 0
			.WithPair<FFlecsTickTypeRelationship>("$TickTypeTag") // 1
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

bool UFlecsWorld::ProgressGameLoops(const FGameplayTag& TickTypeTag, const double DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_FlecsWorldProgress);

	if UNLIKELY_IF(ShouldQuit())
	{
		UE_LOGFMT(LogFlecsWorld, Warning, "World is quitting, cannot progress: {WorldObjectName}", *GetName());
		return false;
	}

	if (!GameLoopTickTypes.Contains(TickTypeTag))
	{
		return false;
	}

	HandleWorldPause();

	const TConstArrayView<TScriptInterface<IFlecsGameLoopInterface>> GameLoopsToTick = GameLoopTickTypes[TickTypeTag];

	for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : GameLoopsToTick)
	{
		solid_checkf(GameLoopInterface,
			TEXT("Game loop interface is nullptr for tick type %s in world %s"),
			*TickTypeTag.ToString(),
			*GetName());

		const bool bGameLoopResult = GameLoopInterface->Progress(DeltaTime, TickTypeTag, this);

		if UNLIKELY_IF(!bGameLoopResult)
		{
			UE_LOGFMT(LogFlecsWorld, Error,
				"Game loop {GameLoopName} failed to progress for tick type {TickType} in world {WorldName}",
				GameLoopInterface.GetObject()->GetName(),
				TickTypeTag.ToString(),
				GetName());
			
			return false;
		}
	}

	return true;
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

	TickFunctionQuery.Destroy();
	
	for (const TScriptInterface<IFlecsGameLoopInterface>& GameLoopInterface : GameLoopInterfaces)
	{
		if (GameLoopInterface)
		{
			GameLoopInterface->GetEntityHandle().Remove<FFlecsGameLoopTag>();
		}
	}
	
	CallUnregisterOnRegisteredObjects();
		
	World.release();
	World.world_ = nullptr;
	MarkAsGarbage();
}

void UFlecsWorld::SetPipeline(const FFlecsEntityHandle& InPipeline) const
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

void UFlecsWorld::RunPipeline(const FFlecsId InPipeline, const double DeltaTime) const
{
	solid_checkf(InPipeline.IsValid(), TEXT("Pipeline is not valid"));
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

bool UFlecsWorld::IsSupportedForNetworking() const
{
	return false;
}

void UFlecsWorld::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

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

FFlecsEntityHandle UFlecsWorld::GetFlecsTickFunctionByType(const FGameplayTag& InTickType) const
{
	TickFunctionQuery.set_var("TickTypeTag", GetTagEntity(InTickType));
	return TickFunctionQuery.first();
}

UObject* UFlecsWorld::RegisterFlecsObject(const TSubclassOf<UObject> InClass)
{
	solid_check(InClass);
	
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
		
	const TSolidNotNull<UObject*> FlecsObject = NewObject<UObject>(this, InClass);
	
	if (!CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject)->ShouldAutoRegisterWithWorld(this))
	{
		FlecsObject->MarkAsGarbage();
		return nullptr;
	}

	RegisteredObjects.Add(FlecsObject);
	RegisteredObjectTypes.Add(InClass, FlecsObject);
	
	const bool bRegisterWithFlecsModule = CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject)->ShouldRegisterWithModule();
	FName ModuleName = NAME_None;
	if (bRegisterWithFlecsModule)
	{
		ModuleName = CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject)->GetModuleName();
		
		if (ModuleName.IsNone())
		{
			const FString PackageName = InClass->GetOuterUPackage()->GetName();
			ModuleName = FName(*FPackageName::GetShortName(PackageName));
		}
	}
	
	FFlecsEntityHandle ModuleEntity;
	
	if (bRegisterWithFlecsModule)
	{
		ModuleEntity = LookupEntity(ModuleName.ToString());
		
		if UNLIKELY_IF(!ModuleEntity.IsValid() || !ModuleEntity.Has(flecs::Module))
		{
			UE_LOGFMT(LogFlecsWorld, Warning,
				"Module {ModuleName} does not exist or is not a valid flecs module, registered object {ObjectName} will not be registered with the module",
				*ModuleName.ToString(), *FlecsObject->GetName());
			
			ModuleEntity = FFlecsEntityHandle::GetNullHandle();
		}
	}
	
	FFlecsId OldScope = FFlecsId::Null();
	
	if (bRegisterWithFlecsModule && ModuleEntity.IsValid())
	{
		OldScope = SetScope(ModuleEntity);
	}
	
	CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject)->RegisterObject(this);
	
	if (Has<FFlecsBeginPlayComponent>())
	{
		CastChecked<IFlecsObjectRegistrationInterface>(FlecsObject)->FlecsWorldBeginPlay(this);
	}
	else
	{
		// do nothing
	}
	
	SetScope(OldScope);
	
	return FlecsObject;
}

UFlecsStage* UFlecsWorld::GetStage(const int32 InStageId) const
{
	solid_cassumef(InStageId > 0, TEXT("Stage ID must be non-negative and can't be the same as the main world (0)"));
	solid_checkf(Stages.IsValidIndex(InStageId), TEXT("Stage ID %d is out of bounds"), InStageId);
	return Stages[InStageId];
}

UFlecsStage* UFlecsWorld::GetStage(const flecs::world& InStageWorld) const
{
	if UNLIKELY_IF(!InStageWorld.is_stage())
	{
		return nullptr;
	}
	
	const int32 StageId = InStageWorld.get_stage_id();
	return GetStage(StageId);
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
	
	if (InStageCount <= 1)
	{
		Stages.Add(nullptr);
		return;
	}
	
	for (int32 StageIndex = 0; StageIndex < InStageCount; ++StageIndex)
	{
		if (StageIndex == 0)
		{
			Stages.Add(nullptr);
			continue;
		}
		
		const TSolidNotNull<UFlecsStage*> NewStage = NewObject<UFlecsStage>(this);
		NewStage->SetStageWorld(GetNativeFlecsWorld().get_stage(StageIndex));
		
		Stages.Add(NewStage);
		solid_checkf(Stages.Num() - 1 == StageIndex, TEXT("Stage index does not match stage array index"));
		solid_checkf(Stages.Num() - 1 == NewStage->GetStageId(), TEXT("Stage ID does not match stage array index"));
	}
}

UFlecsStage* UFlecsWorld::CreateAsyncStage()
{
	flecs::world AsyncStage = GetNativeFlecsWorld().async_stage();
	
	const TSolidNotNull<UFlecsStage*> NewStage = NewObject<UFlecsStage>(this);
	NewStage->SetStageWorld(AsyncStage);
	
	return NewStage;
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
		
			void* ComponentPtr = nullptr;

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
