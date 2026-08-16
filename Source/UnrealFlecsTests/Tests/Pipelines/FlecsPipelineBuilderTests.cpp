// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Pipelines/FlecsPipelineBuilder.h"
#include "Worlds/FlecsWorld.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsPipelineBuilderTests,
	"UnrealFlecs.Pipelines.Builder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Pipelines][Builder]")
{
protected:
	virtual bool ShouldUseDefaultGameLoop() const override
	{
		return false;
	}

public:
	TEST_METHOD(BuildPipeline_WithCascadeTraversal)
	{
		const FFlecsPipelineHandle Pipeline = World()->CreatePipeline(TEXT("CascadeTraversalPipeline"))
			.With(flecs::System)
			.With(flecs::Phase)
			.Cascade(flecs::DependsOn)
			.Build();

		ASSERT_THAT(IsTrue(Pipeline.IsValid()));
	}
}; // FlecsPipelineBuilderTests

#endif // WITH_AUTOMATION_TESTS
