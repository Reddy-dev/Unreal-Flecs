// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Types/SolidNotNull.h"

#include "FlecsEntityRecord.h"

#include "FlecsNamedEntityRecordFragment.generated.h"

// @TODO: add additional settings
USTRUCT(BlueprintType, DisplayName = "Named Entity Fragment")
/**
 * @brief Applies a name to an entity while an entity record is materialized.
 *
 * When enabled, the same name is stored for sub-entity name propagation and
 * consumed by the sub-entity name observer.
 */
struct UNREALFLECS_API FFlecsNamedEntityRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	/** Constructs an empty named record fragment. */
	FORCEINLINE FFlecsNamedEntityRecordFragment() = default;
	/**
	 * @brief Constructs a named record fragment.
	 * @param InName Name applied during record application.
	 * @param bInNameInheritedSubEntities Whether sub-entities inherit the name.
	 */
	FORCEINLINE FFlecsNamedEntityRecordFragment(const FString& InName, const bool bInNameInheritedSubEntities = true)
		: Name(InName)
		, bNameInheritedSubEntities(bInNameInheritedSubEntities)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record Fragment")
	/** Name applied to the entity during record application. */
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Entity Record Fragment")
	/** Whether the name should be propagated to sub-entities. */
	bool bNameInheritedSubEntities = true;
	
	/**
	 * @brief Applies the configured name before the record is materialized.
	 * @param InFlecsWorld World used for record application.
	 * @param InEntityHandle Entity receiving the name.
	 */
	virtual void PreApplyRecordToEntity(
			const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	
	/** Custom fluent builder used by FFlecsEntityRecord::FragmentScope. */
	struct FBuilder;
	
}; // struct FFlecsNamedEntityRecordFragment

/**
 * @brief Fluent builder for FFlecsNamedEntityRecordFragment.
 */
struct FFlecsNamedEntityRecordFragment::FBuilder : public FFlecsEntityRecord::TFragmentBuilderType<FFlecsNamedEntityRecordFragment>
{
	using Super = FFlecsEntityRecord::TFragmentBuilderType<FFlecsNamedEntityRecordFragment>;
	using Super::Super;
	
public:
	/**
	 * @brief Sets the name applied by the fragment.
	 * @param InName Name to apply.
	 * @return This builder.
	 */
	FORCEINLINE FBuilder& Named(const FString& InName)
	{
		this->GetSelf().Name = InName;
		return *this;
	}
	
	/**
	 * @brief Configures name propagation to sub-entities.
	 * @param bInInheritSubEntities Whether sub-entities inherit the name.
	 * @return This builder.
	 */
	FORCEINLINE FBuilder& InheritSubEntityNames(const bool bInInheritSubEntities)
	{
		this->GetSelf().bNameInheritedSubEntities = bInInheritSubEntities;
		return *this;
	}
		
}; // struct FFlecsNamedEntityRecordFragment::FBuilder