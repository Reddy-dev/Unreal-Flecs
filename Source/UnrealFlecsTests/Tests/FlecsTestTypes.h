// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UnrealFlecsConfigMacros.h"

#include "UObject/Object.h"
#include "GameplayTagsManager.h"
#include "Developer/CQTest/Public/CQTest.h"

#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "Entities/FlecsDefaultEntityEngine.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsTestTypes.generated.h"

DECLARE_DEFAULT_ENTITY(TestEntityOption);
DECLARE_DEFAULT_ENTITY(TestEntityOption2WithTrait);

namespace UE::Flecs::test::internal
{
	DECLARE_DEFAULT_ENTITY(TestEntityOption3InNamespace);
} // namespace UE::Flecs::test::internal

DECLARE_DEFAULT_ENTITY(TestEntityOption4WithComponentValue);

struct FFlecsTest_CPPStruct
{
}; // struct FFlecsTest_CPPStruct

struct FFlecsTest_CPPStructValue
{
	int32 Value = 1;
}; // struct FFlecsTest_CPPStructWithNameAndValue

struct FFlecsTest_CPPStruct_Traits
{
}; // struct FFlecsTest_CppStruct_Traits

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits> : public TFlecsComponentTraitsBase<FFlecsTest_CPPStruct_Traits>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool Trait = true;
	static constexpr bool PairIsTag = true;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

struct FFlecsTest_CPPStructValue_Traits
{
	uint32 Value = 0;
}; // struct FFlecsTest_CPPStruct_Value

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStructValue_Traits> : public TFlecsComponentTraitsBase<FFlecsTest_CPPStructValue_Traits>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool Trait = true;
	static constexpr bool PairIsTag = true;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStructValue_Traits>

struct FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda
{
	uint32 Value = 0;
}; // struct FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda> : public TFlecsComponentTraitsBase<FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool Trait = true;
	static constexpr bool PairIsTag = true;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_Tag
{
	GENERATED_BODY()
}; // struct FFlecsTestStruct

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_Tag> : public TFlecsComponentTraitsBase<FFlecsTestStruct_Tag>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_Tag_Inherited
{
	GENERATED_BODY()
}; // struct FFlecsTestStruct

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_Tag_Inherited> : public TFlecsComponentTraitsBase<FFlecsTestStruct_Tag_Inherited>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT(BlueprintType, BlueprintInternalUseOnly)
struct FFlecsTestStruct_Value
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs|Test")
	int32 Value = 1;
	
}; // struct FFlecsTestStructWithNameAndValue

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_Value> : public TFlecsComponentTraitsBase<FFlecsTestStruct_Value>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUSTRUCTPairTestComponent
{
	GENERATED_BODY()

public:
}; // struct FUSTRUCTPairTestComponent

template <>
struct TFlecsComponentTraits<FUSTRUCTPairTestComponent> : public TFlecsComponentTraitsBase<FUSTRUCTPairTestComponent>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUSTRUCTPairTestComponent_Second
{
	GENERATED_BODY()
}; // struct FUSTRUCTPairTestComponent_Second

template <>
struct TFlecsComponentTraits<FUSTRUCTPairTestComponent_Second> : public TFlecsComponentTraitsBase<FUSTRUCTPairTestComponent_Second>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUSTRUCTPairTestComponent_Data
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;

	UPROPERTY()
	float FloatValue = 0.0f;

	UPROPERTY()
	FString StringValue;
	
}; // struct FUSTRUCTPairTestComponent_Data

template <>
struct TFlecsComponentTraits<FUSTRUCTPairTestComponent_Data> : public TFlecsComponentTraitsBase<FUSTRUCTPairTestComponent_Data>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestComponent_Inherited : public FFlecsTestStruct_Value
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value2;
}; // struct FFlecsTestComponent_Inherited

template <>
struct TFlecsComponentTraits<FFlecsTestComponent_Inherited> : public TFlecsComponentTraitsBase<FFlecsTestComponent_Inherited>
{
	static constexpr bool AutoRegister = false;
	
	using InheritsFrom = FFlecsTestStruct_Value;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestComponent_NonTagUSTRUCT_Inherited : public FFlecsTestComponent_Inherited
{
	GENERATED_BODY()
}; // struct FUSTRUCTTestComponent_NonTagUSTRUCT_Inherited

template <>
struct TFlecsComponentTraits<FFlecsTestComponent_NonTagUSTRUCT_Inherited> : public TFlecsComponentTraitsBase<FFlecsTestComponent_NonTagUSTRUCT_Inherited>
{
	static constexpr bool AutoRegister = false;
	
	using InheritsFrom = FFlecsTestComponent_Inherited;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_WithPropertyTraits
{
	GENERATED_BODY()

public:
	
}; // struct FFlecsTestStruct_WithProperties

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_WithPropertyTraits> : public TFlecsComponentTraitsBase<FFlecsTestStruct_WithPropertyTraits>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool Trait = true;
	static constexpr bool PairIsTag = true;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStructValue_WithPropertyTraits
{
	GENERATED_BODY()

public:
	UPROPERTY()
	uint32 Value = 0;
	
}; // struct FFlecsTestStruct_WithProperties

template <>
struct TFlecsComponentTraits<FFlecsTestStructValue_WithPropertyTraits> : public TFlecsComponentTraitsBase<FFlecsTestStructValue_WithPropertyTraits>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool Trait = true;
	static constexpr bool PairIsTag = true;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_Toggleable
{
	GENERATED_BODY()

public:
	UPROPERTY()
	uint32 Value = 0;
}; // struct FFlecsTestStruct_Toggleable

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_Toggleable> : public TFlecsComponentTraitsBase<FFlecsTestStruct_Toggleable>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool CanToggle = true;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_PairIsTag
{
	GENERATED_BODY()
}; // struct FFlecsTestStruct_PairIsTag

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_PairIsTag> : public TFlecsComponentTraitsBase<FFlecsTestStruct_PairIsTag>
{
	static constexpr bool AutoRegister = false;

	static constexpr bool PairIsTag = true;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_EmptyRegistrationFunction
{
	GENERATED_BODY()
}; // struct FFlecsTestStruct_EmptyRegistrationFunction

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_EmptyRegistrationFunction> : public TFlecsComponentTraitsBase<FFlecsTestStruct_EmptyRegistrationFunction>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestStruct_NoRegistrationLambda
{
	GENERATED_BODY()
}; // struct FFlecsTestStruct_NoRegistrationLambda

enum class ETestEnum : uint8
{
	None,
	One,
	Two,
	Three
}; // enum class ETestEnum

UENUM()
enum class EFlecsTestEnum_UENUM : uint8
{
	None,
	One,
	Two,
	Three
}; // enum class EFlecsTestEnum_UENUM

UENUM()
enum class EFlecsTestEnum_SparseUENUM : uint8
{
	None,
	One,
	Two,
	Three,
	Five = 5,
	Ten = 10,
}; // enum class EFlecsTestEnum_SparseUENUM

USTRUCT()
struct FUStructTestComponent_NonTagUSTRUCT
{
	GENERATED_BODY()

	UPROPERTY()
	bool bTest = false;
	
}; // struct FUStructTestComponent_NonTagUSTRUCT

template <>
struct TFlecsComponentTraits<FUStructTestComponent_NonTagUSTRUCT> : public TFlecsComponentTraitsBase<FUStructTestComponent_NonTagUSTRUCT>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUSTructTestComponent_AccidentalTag
{
	GENERATED_BODY()

	/*
	* If Registered as a static struct,
	* this will be registered as a tag as it is one byte and has no UPROPERTY saying otherwise.
	**/
	uint8 Test = 0;
	
}; // struct FUSTructTestComponent_AccidentalTag

template <>
struct TFlecsComponentTraits<FUSTructTestComponent_AccidentalTag> : public TFlecsComponentTraitsBase<FUSTructTestComponent_AccidentalTag>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FFlecsTestComponent_Vector
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Value;
	
}; // struct FFlecsTestComponent_Position

template <>
struct TFlecsComponentTraits<FFlecsTestComponent_Vector> : public TFlecsComponentTraitsBase<FFlecsTestComponent_Vector>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct alignas(16) FUStructTestComponent_CustomAlignedUSTRUCT_SixteenBytes
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value1;

	UPROPERTY()
	int32 Value2;

}; // struct FUStructTestComponent_CustomAlignedUSTRUCT_SixteenBytes

template <>
struct TFlecsComponentTraits<FUStructTestComponent_CustomAlignedUSTRUCT_SixteenBytes> : public TFlecsComponentTraitsBase<FUStructTestComponent_CustomAlignedUSTRUCT_SixteenBytes>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct alignas(32) FUStructTestComponent_CustomAlignedUSTRUCT_ThirtyTwoBytes
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value1;

	UPROPERTY()
	int32 Value2;

	UPROPERTY()
	int32 Value3;

	UPROPERTY()
	int32 Value4;

}; // struct FUStructTestComponent_CustomAlignedUSTRUCT_ThirtyTwoBytes

template <>
struct TFlecsComponentTraits<FUStructTestComponent_CustomAlignedUSTRUCT_ThirtyTwoBytes> : public TFlecsComponentTraitsBase<FUStructTestComponent_CustomAlignedUSTRUCT_ThirtyTwoBytes>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct alignas(64) FUStructTestComponent_CustomAlignedUSTRUCT_SixtyFourBytes
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value1;

	UPROPERTY()
	int32 Value2;

	UPROPERTY()
	int32 Value3;

}; // struct FUStructTestComponent_CustomAlignedUSTRUCT_SixtyFourBytes

template <>
struct TFlecsComponentTraits<FUStructTestComponent_CustomAlignedUSTRUCT_SixtyFourBytes> : public TFlecsComponentTraitsBase<FUStructTestComponent_CustomAlignedUSTRUCT_SixtyFourBytes>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUStructTestComponent_MovableUSTRUCT
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TArray<int32> Values;

	FUStructTestComponent_MovableUSTRUCT() = default;
	
}; // struct FUStructTestComponent_MovableUSTRUCT

template <>
struct TFlecsComponentTraits<FUStructTestComponent_MovableUSTRUCT> : public TFlecsComponentTraitsBase<FUStructTestComponent_MovableUSTRUCT>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUStructTestComponent_MovableNotRegisteredUSTRUCT
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TArray<int32> Values;
	
	FUStructTestComponent_MovableNotRegisteredUSTRUCT() = default;
	
}; // struct FUStructTestComponent_MovableNotRegisteredUSTRUCT

template <>
struct TFlecsComponentTraits<FUStructTestComponent_MovableNotRegisteredUSTRUCT> : public TFlecsComponentTraitsBase<FUStructTestComponent_MovableNotRegisteredUSTRUCT>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_Traits>

USTRUCT()
struct FUStructTestComponent_LifecycleTracker
{
	GENERATED_BODY()
	
	enum class EConstructor : uint8
	{
		Default,
		Copy,
		Move
	}; // enum class EConstructor
	
	EConstructor ConstructedVia = EConstructor::Default;

	UPROPERTY()
	int32 TimesDestructed = 0;

	UPROPERTY()
	int32 TimesCopyAssignedInto = 0;

	mutable int32 TimesCopyAssignedFrom = 0;
	mutable int32 TimesCopyConstructedFrom = 0;

	UPROPERTY()
	int32 TimesMoveAssignedInto = 0;

	UPROPERTY()
	int32 TimesMoveAssignedFrom = 0;

	UPROPERTY()
	int32 TimesMoveConstructedFrom = 0;

	FUStructTestComponent_LifecycleTracker() = default;

	FUStructTestComponent_LifecycleTracker(const FUStructTestComponent_LifecycleTracker& That)
		: ConstructedVia(EConstructor::Copy)
	{
		++That.TimesCopyConstructedFrom;
	}

	FUStructTestComponent_LifecycleTracker& operator=(const FUStructTestComponent_LifecycleTracker& That)
	{
		++TimesCopyAssignedInto;
		++That.TimesCopyAssignedFrom;
		return *this;
	}

	FUStructTestComponent_LifecycleTracker(FUStructTestComponent_LifecycleTracker&& That) noexcept
		: ConstructedVia(EConstructor::Move)
	{
		++That.TimesMoveConstructedFrom;
	}

	FUStructTestComponent_LifecycleTracker& operator=(FUStructTestComponent_LifecycleTracker&& That) noexcept
	{
		++TimesMoveAssignedInto;
		++That.TimesMoveAssignedFrom;
		return *this;
	}

	~FUStructTestComponent_LifecycleTracker()
	{
		++TimesDestructed;
	}

	// --- Convenience queries (match original semantics) ---
	bool MovedFrom() const
	{
		return TimesMoveAssignedFrom > 0 || TimesMoveConstructedFrom > 0;
	}

	bool MovedInto() const
	{
		return TimesMoveAssignedInto > 0 || ConstructedVia == EConstructor::Move;
	}

	bool CopiedFrom() const
	{
		return TimesCopyAssignedFrom > 0 || TimesCopyConstructedFrom > 0;
	}

	bool CopiedInto() const
	{
		return TimesCopyAssignedInto > 0 || ConstructedVia == EConstructor::Copy;
	}
	
}; // struct FUStructTestComponent_LifecycleTracker

template <>
struct TFlecsComponentTraits<FUStructTestComponent_LifecycleTracker> : public TFlecsComponentTraitsBase<FUStructTestComponent_LifecycleTracker>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FUStructTestComponent_LifecycleTracker>

USTRUCT()
struct FFlecsTestStruct_LifecycleTracker_NoMoveReg
{
	GENERATED_BODY()
	
	enum class EConstructor : uint8
	{
		Default,
		Copy,
		Move
	}; // enum class EConstructor
	
	EConstructor ConstructedVia = EConstructor::Default;

	UPROPERTY()
	int32 TimesDestructed = 0;

	UPROPERTY()
	int32 TimesCopyAssignedInto = 0;

	mutable int32 TimesCopyAssignedFrom = 0;
	mutable int32 TimesCopyConstructedFrom = 0;

	UPROPERTY()
	int32 TimesMoveAssignedInto = 0;

	UPROPERTY()
	int32 TimesMoveAssignedFrom = 0;

	UPROPERTY()
	int32 TimesMoveConstructedFrom = 0;

	FFlecsTestStruct_LifecycleTracker_NoMoveReg() = default;

	FFlecsTestStruct_LifecycleTracker_NoMoveReg(const FFlecsTestStruct_LifecycleTracker_NoMoveReg& That)
		: ConstructedVia(EConstructor::Copy)
	{
		++That.TimesCopyConstructedFrom;
	}

	FFlecsTestStruct_LifecycleTracker_NoMoveReg& operator=(const FFlecsTestStruct_LifecycleTracker_NoMoveReg& That)
	{
		++TimesCopyAssignedInto;
		++That.TimesCopyAssignedFrom;
		return *this;
	}

	FFlecsTestStruct_LifecycleTracker_NoMoveReg(FFlecsTestStruct_LifecycleTracker_NoMoveReg&& That) noexcept
		: ConstructedVia(EConstructor::Move)
	{
		++That.TimesMoveConstructedFrom;
	}

	FFlecsTestStruct_LifecycleTracker_NoMoveReg& operator=(FFlecsTestStruct_LifecycleTracker_NoMoveReg&& That) noexcept
	{
		++TimesMoveAssignedInto;
		++That.TimesMoveAssignedFrom;
		return *this;
	}

	~FFlecsTestStruct_LifecycleTracker_NoMoveReg()
	{
		++TimesDestructed;
	}

	bool MovedFrom() const
	{
		return TimesMoveAssignedFrom > 0 || TimesMoveConstructedFrom > 0;
	}

	bool MovedInto() const
	{
		return TimesMoveAssignedInto > 0 || ConstructedVia == EConstructor::Move;
	}

	bool CopiedFrom() const
	{
		return TimesCopyAssignedFrom > 0 || TimesCopyConstructedFrom > 0;
	}

	bool CopiedInto() const
	{
		return TimesCopyAssignedInto > 0 || ConstructedVia == EConstructor::Copy;
	}
	
}; // struct FFlecsTestStruct_LifecycleTracker_NoMoveReg

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_LifecycleTracker_NoMoveReg> : public TFlecsComponentTraitsBase<FFlecsTestStruct_LifecycleTracker_NoMoveReg>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FUStructTestComponent_LifecycleTracker>

USTRUCT()
struct FFlecsTestStruct_FlecsHookTracker
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int32 OnAddCalled = 0;
	
	UPROPERTY()
	int32 OnRemoveCalled = 0;
	
	UPROPERTY()
	int32 OnSetCalled = 0;
	
	UPROPERTY()
	int32 OnReplaceCalled = 0;
	
}; // struct FFlecsTestStruct_FlecsHookTracker

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_FlecsHookTracker> : public TFlecsComponentTraitsBase<FFlecsTestStruct_FlecsHookTracker>
{
	static constexpr bool AutoRegister = false;
	
}; // struct TFlecsComponentTraits<FUStructTestComponent_LifecycleTracker>

/*
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_FlecsHookTracker,
	[](flecs::world InWorld, const TFlecsComponentHandle<FFlecsTestStruct_FlecsHookTracker>& InComponentHandle)
	{
		InComponentHandle
			.OnAdd([](FFlecsTestStruct_FlecsHookTracker& HookTracker)
			{
				++HookTracker.OnAddCalled;
			})
			.OnReplace([](FFlecsTestStruct_FlecsHookTracker& Previous, FFlecsTestStruct_FlecsHookTracker& Next)
			{
				++Previous.OnReplaceCalled;
				++Next.OnReplaceCalled;
			});
	});
	*/

struct FFlecsTestNativeGameplayTags : public FGameplayTagNativeAdder
{
	static FFlecsTestNativeGameplayTags StaticInstance;
	
	FGameplayTag TestTag1;
	FGameplayTag TestTag2;
	FGameplayTag TestTag3;

	FGameplayTag TestSameSubTag1;
	FGameplayTag TestSameSubTag2;

	FGameplayTag TestSameSubGrandchildTag1;
	FGameplayTag TestSameSubGrandchildTag2;

	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		TestTag1 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Tag1"));
		TestTag2 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Tag2"));
		TestTag3 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Tag3"));

		TestSameSubTag1 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Sub1.Tag1"));
		TestSameSubTag2 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Sub2.Tag1"));

		TestSameSubGrandchildTag1 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Sub1.Tag1"));
		TestSameSubGrandchildTag2 = Manager.AddNativeGameplayTag(TEXT("Test.UnrealFlecs.Sub1.Tag2"));
	}

	static NO_DISCARD FORCEINLINE const FFlecsTestNativeGameplayTags& Get()
	{
		return StaticInstance;
	}
	
}; // struct FFlecsTestNativeGameplayTags

UCLASS()
class UNREALFLECSTESTS_API UFlecsUObjectComponentTestObject : public UObject
{
	GENERATED_BODY()
}; // class UFlecsUObjectComponentTestObject

USTRUCT()
struct FFlecsTestStruct_WithUObjectProperty
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<UFlecsUObjectComponentTestObject> Object;
}; // struct FFlecsTestStruct_WithUObjectProperty

template <>
struct TFlecsComponentTraits<FFlecsTestStruct_WithUObjectProperty> : public TFlecsComponentTraitsBase<FFlecsTestStruct_WithUObjectProperty>
{
	static constexpr bool AutoRegister = false;
	
	static constexpr bool WithAddReferencedObjects = true;
	
}; // struct TFlecsComponentTraits<FFlecsTestStruct_WithUObjectProperty>





