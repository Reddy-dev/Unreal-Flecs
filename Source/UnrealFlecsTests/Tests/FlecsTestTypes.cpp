// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTestTypes.h"

#include "Types/SolidCppStructOps.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTestTypes)

REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStruct_Traits);
REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStructValue_Traits_WithTypedComponentHandleInLambda);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_Tag);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_Tag_Inherited);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_Value);
REGISTER_FLECS_COMPONENT(FUSTRUCTPairTestComponent);
REGISTER_FLECS_COMPONENT(FUSTRUCTPairTestComponent_Second);
REGISTER_FLECS_COMPONENT(FUSTRUCTPairTestComponent_Data);
REGISTER_FLECS_COMPONENT(FFlecsTestComponent_Inherited);
REGISTER_FLECS_COMPONENT(FFlecsTestComponent_NonTagUSTRUCT_Inherited);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_WithPropertyTraits);
REGISTER_FLECS_COMPONENT(FFlecsTestStructValue_WithPropertyTraits);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_Toggleable);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_PairIsTag);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_EmptyRegistrationFunction);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_NoRegistrationLambda);
REGISTER_FLECS_COMPONENT(ETestEnum);
REGISTER_FLECS_COMPONENT(EFlecsTestEnum_UENUM);
REGISTER_FLECS_COMPONENT(EFlecsTestEnum_SparseUENUM);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_NonTagUSTRUCT);
REGISTER_FLECS_COMPONENT(FUSTructTestComponent_AccidentalTag);
REGISTER_FLECS_COMPONENT(FFlecsTestComponent_Vector);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_CustomAlignedUSTRUCT_SixteenBytes);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_CustomAlignedUSTRUCT_ThirtyTwoBytes);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_CustomAlignedUSTRUCT_SixtyFourBytes);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_MovableUSTRUCT);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_MovableNotRegisteredUSTRUCT);
REGISTER_FLECS_COMPONENT(FUStructTestComponent_LifecycleTracker);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_LifecycleTracker_NoMoveReg);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_FlecsHookTracker);
REGISTER_FLECS_COMPONENT(FFlecsTestStruct_WithUObjectProperty);

DEFINE_SOLID_MOVEABLE_CPP_STRUCT(FUStructTestComponent_MovableUSTRUCT);
DEFINE_SOLID_MOVEABLE_CPP_STRUCT(FUStructTestComponent_LifecycleTracker);

DEFINE_DEFAULT_ENTITY(TestEntityOption, 6000 + FLECS_HI_COMPONENT_ID, [](flecs::entity InEntity)
{
});

namespace UE::Flecs::test::internal
{
	DEFINE_DEFAULT_ENTITY(TestEntityOption3InNamespace,
		6002 + FLECS_HI_COMPONENT_ID, [](flecs::entity InEntity)
	{
		
	});
	
} // namespace UE::Flecs::test::internal

DEFINE_DEFAULT_ENTITY(TestEntityOption2WithTrait,
	6001 + FLECS_HI_COMPONENT_ID, [](flecs::entity InEntity)
	{
		InEntity
			.add(flecs::Trait);
	});

DEFINE_DEFAULT_ENTITY(TestEntityOption4WithComponentValue,
	6003 + FLECS_HI_COMPONENT_ID, [](flecs::entity InEntity)
	{
		
	});

FFlecsTestNativeGameplayTags FFlecsTestNativeGameplayTags::StaticInstance;