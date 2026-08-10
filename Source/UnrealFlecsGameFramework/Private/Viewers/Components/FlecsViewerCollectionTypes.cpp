// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Components/FlecsViewerCollectionTypes.h"

#include "Collections/FlecsCollectionBuilder.h"
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
	
	FFlecsRecordPair RecordPair;
	RecordPair.First = FFlecsRecordPairSlot::Make<FFlecsViewerRelationship>();
	RecordPair.Second = FFlecsRecordPairSlot::Make<FFlecsViewerPlayerComponent>();
	
	Builder
		.AddPair(RecordPair)
		.Parameters<FFlecsViewerPlayerComponent>({},
			[&](FFlecsEntityHandle TargetEntity, const FInstancedStruct& Parameters)
		{
			TargetEntity.Assign<FFlecsViewerPlayerComponent>(Parameters.Get<FFlecsViewerPlayerComponent>());
		});
}

FInstancedStruct UFlecsPlayerViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerPlayerComponent>();
}

void UFlecsActorViewerCollection::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Super::BuildCollection(Builder);
	
	FFlecsRecordPair RecordPair;
	RecordPair.First = FFlecsRecordPairSlot::Make<FFlecsViewerRelationship>();
	RecordPair.Second = FFlecsRecordPairSlot::Make<FFlecsViewerActorComponent>();
	
	Builder
		.AddPair(RecordPair)
		.Parameters<FFlecsViewerActorComponent>({},
			[&](const FFlecsEntityHandle TargetEntity, const FInstancedStruct& Parameters)
		{
			TargetEntity.Assign<FFlecsViewerActorComponent>(Parameters.Get<FFlecsViewerActorComponent>());
		});
}

FInstancedStruct UFlecsActorViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerActorComponent>();
}

void UFlecsStreamSourceViewerCollection::BuildCollection(FFlecsCollectionBuilder& Builder) const
{
	Super::BuildCollection(Builder);
	
	FFlecsRecordPair RecordPair;
	RecordPair.First = FFlecsRecordPairSlot::Make<FFlecsViewerRelationship>();
	RecordPair.Second = FFlecsRecordPairSlot::Make<FFlecsViewerStreamingSourceComponent>();
	
	Builder
		.AddPair(RecordPair)
		.Parameters<FFlecsViewerStreamingSourceComponent>({},
			[&](const FFlecsEntityHandle TargetEntity, const FInstancedStruct& Parameters)
		{
			TargetEntity.Assign<FFlecsViewerStreamingSourceComponent>(Parameters.Get<FFlecsViewerStreamingSourceComponent>());
		});
}

FInstancedStruct UFlecsStreamSourceViewerCollection::GetParametersType() const
{
	return FInstancedStruct::Make<FFlecsViewerStreamingSourceComponent>();
}
