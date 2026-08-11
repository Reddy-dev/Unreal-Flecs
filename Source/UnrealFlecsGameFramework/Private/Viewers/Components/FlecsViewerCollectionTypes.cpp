// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Components/FlecsViewerCollectionTypes.h"

#include "Collections/FlecsCollectionBuilder.h"
#include "Viewers/Components/FlecsViewerPerspectiveComponent.h"
#include "Viewers/Components/FlecsViewerTransformComponent.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsViewerCollectionTypes)

void UFlecsViewerCollectionBase::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Builder
		.Add<FFlecsViewerTransformComponent>();
}

void UFlecsPlayerViewerCollection::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Super::BuildCollection(Builder);
	//Builder.Add<FFlecsViewerPerspectiveComponent>();
	
	Builder
		.Add(EFlecsViewerType::Player)
		.AddPair<FFlecsViewerRelationship, FFlecsViewerPlayerComponent>()
		.Parameters<FFlecsViewerPlayerComponent>({},
			[&](FFlecsEntityHandle TargetEntity, const FFlecsViewerPlayerComponent& Parameters)
		{
			TargetEntity.SetPair<FFlecsViewerRelationship, FFlecsViewerPlayerComponent>(Parameters);
		});
}

FInstancedStruct UFlecsPlayerViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerPlayerComponent>();
}

void UFlecsActorViewerCollection::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Super::BuildCollection(Builder);
	
	Builder
		.Add(EFlecsViewerType::Actor)
		.AddPair<FFlecsViewerRelationship, FFlecsViewerActorComponent>()
		.Parameters<FFlecsViewerActorComponent>({},
			[&](const FFlecsEntityHandle TargetEntity, const FFlecsViewerActorComponent& Parameters)
		{
			TargetEntity.SetPair<FFlecsViewerRelationship, FFlecsViewerActorComponent>(Parameters);
		});
}

FInstancedStruct UFlecsActorViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerActorComponent>();
}

void UFlecsStreamSourceViewerCollection::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Super::BuildCollection(Builder);
	
	Builder
		.Add(EFlecsViewerType::StreamingSource)
		.AddPair<FFlecsViewerRelationship, FFlecsViewerStreamingSourceComponent>()
		.Parameters<FFlecsViewerStreamingSourceComponent>({},
			[&](const FFlecsEntityHandle TargetEntity, const FFlecsViewerStreamingSourceComponent& Parameters)
		{
			TargetEntity.SetPair<FFlecsViewerRelationship, FFlecsViewerStreamingSourceComponent>(Parameters);
		});
}

FInstancedStruct UFlecsStreamSourceViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerStreamingSourceComponent>();
}
