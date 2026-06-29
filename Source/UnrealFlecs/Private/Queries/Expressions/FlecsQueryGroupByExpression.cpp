// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryGroupByExpression.h"

#include "Entities/FlecsId.h"
#include "Queries/FlecsQueryBuilderView.h"
#include "Queries/Callbacks/FlecsGroupByCallbackDefinition.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryGroupByExpression)

FFlecsQueryGroupByExpression::FFlecsQueryGroupByExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryGroupByExpression::Apply(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	FFlecsQueryBuilderView& InQueryBuilder) const
{
	FFlecsId GroupByComponentId;
	
	if LIKELY_IF(GroupByInput.Get().ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		GroupByComponentId = GroupByInput.Get<FFlecsQueryGeneratorInputType>().GetFlecsIdOutput(InWorld);
	}
	else UNLIKELY_ATTRIBUTE
	{
		solid_checkf(false, TEXT("Unsupported GroupBy input type return type"));
	}
	
	if (GroupByCallbackDefinition.IsSet())
	{
		if UNLIKELY_IF(!ensureAlwaysMsgf(GroupByCallbackDefinition.GetValue()->GetGroupByFunction() != nullptr,
			TEXT("Invalid GroupBy callback provided!")))
		{
			return;
		}

		UE::Flecs::Queries::FFlecsGroupByContextData* TrampolineContextData = new UE::Flecs::Queries::FFlecsGroupByContextData();
		TrampolineContextData->Definition = GroupByCallbackDefinition.GetValue();
		
		TrampolineContextData->UserContext = nullptr; // @TODO: Add user context support
		
		InQueryBuilder.group_by_ctx(TrampolineContextData, [](void* InContext) -> void
			{
				UE::Flecs::Queries::FFlecsGroupByContextData* ContextData = static_cast<UE::Flecs::Queries::FFlecsGroupByContextData*>(InContext);
				solid_cassumef(ContextData != nullptr, TEXT("ContextData is nullptr"));
			
				delete ContextData;
			});
		
		InQueryBuilder.group_by(GroupByComponentId, 
			[](flecs::world_t* InWorld, flecs::table_t* InTable, flecs::id_t InId, void* InContext) -> uint64
		{
			UE::Flecs::Queries::FFlecsGroupByContextData* ContextData = static_cast<UE::Flecs::Queries::FFlecsGroupByContextData*>(InContext);
			solid_cassumef(ContextData != nullptr, TEXT("ContextData is nullptr"));
			
			const TSolidNotNull<UFlecsWorldInterfaceObject*> World = UFlecsWorldInterfaceObject::GetWorldInterfaceFromFlecsWorld(InWorld);
			return ContextData->Definition->GetGroupByFunction()(World, FFlecsTableHandle(InWorld, InTable), 
				FFlecsId(InId), ContextData->UserContext);
		});
		
		if (GroupByCallbackDefinition.GetValue()->GetCreateGroupFunction() != nullptr)
		{
			InQueryBuilder.on_group_create([](ecs_world_t *world, uint64_t group_id, void* group_by_ctx) -> void*
			{
				UE::Flecs::Queries::FFlecsGroupByContextData* ContextData = static_cast<UE::Flecs::Queries::FFlecsGroupByContextData*>(group_by_ctx);
				solid_cassumef(ContextData != nullptr, TEXT("ContextData is nullptr"));
				
				const TSolidNotNull<UFlecsWorldInterfaceObject*> World = UFlecsWorldInterfaceObject::GetWorldInterfaceFromFlecsWorld(world);
				return ContextData->Definition->GetCreateGroupFunction()(World, group_id, ContextData->UserContext);
			});
		}
		
		if (GroupByCallbackDefinition.GetValue()->GetDeleteGroupFunction() != nullptr)
		{
			InQueryBuilder.on_group_delete([](ecs_world_t *world, uint64_t group_id, void* group_ctx, void* group_by_ctx) -> void
			{
				UE::Flecs::Queries::FFlecsGroupByContextData* ContextData = static_cast<UE::Flecs::Queries::FFlecsGroupByContextData*>(group_by_ctx);
				solid_cassumef(ContextData != nullptr, TEXT("ContextData is nullptr"));
				
				const TSolidNotNull<UFlecsWorldInterfaceObject*> World = UFlecsWorldInterfaceObject::GetWorldInterfaceFromFlecsWorld(world);
				ContextData->Definition->GetDeleteGroupFunction()(World, group_id, group_ctx, ContextData->UserContext);
			});
		}
	}
	else
	{
		InQueryBuilder.group_by(GroupByComponentId);
	}
	
	Super::Apply(InWorld, InQueryBuilder);
}
