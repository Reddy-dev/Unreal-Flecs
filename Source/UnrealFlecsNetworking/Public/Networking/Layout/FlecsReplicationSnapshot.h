// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Iris/Serialization/NetSerializerConfig.h"

#include "FlecsReplicationLayoutId.h"
#include "Networking/FlecsReplicationKey.h"
#include "Iris/Serialization/NetSerializer.h"

#include "FlecsReplicationSnapshot.generated.h"

class UFlecsNetworkWorldSubsystem;
class FFlecsReplicationLayoutRegistry;

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicatedPackedValue
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	uint32 Revision = 0;
	
	UPROPERTY()
	uint32 Offset = 0;
	
	UPROPERTY()
	uint32 Size = 0;
}; // struct FFlecsReplicatedPackedValue

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsEntityReplicationSnapshot
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;
	
	UPROPERTY()
	TArray<FFlecsReplicatedPackedValue> PackedValues;
	
	UPROPERTY()
	TArray<uint8> PayloadData;
	
	UPROPERTY()
	uint32 StateRevision = 0;
	
	// Increments StateRevision
	void FillFromEntity(const FFlecsEntityHandle& InEntityHandle, const FFlecsReplicationLayoutRegistry& InLayoutRegistry);
	
}; // struct FFlecsEntityReplicationSnapshot

USTRUCT()
struct FFlecsEntityReplicationSnapshotSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
}; // struct FFlecsEntityReplicationSnapshotSerializerConfig

/*
namespace UE::Net
{
	UE_NET_DECLARE_SERIALIZER(FFlecsEntityReplicationSnapshotSerializerConfig, UNREALFLECSNETWORKING_API);
} // namespace UE::Net
*/

