// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "UnrealFlecsTests/Fixtures/FlecsRegisteredWorldFixture.h"

#if WITH_AUTOMATION_TESTS

template<typename TDerived, typename TAsserter>
struct TFlecsReplicationTest : TFlecsRegisteredWorldTest<TDerived, TAsserter>
{
	using Super = TFlecsRegisteredWorldTest<TDerived, TAsserter>;

protected:
	virtual void OnRegisteredWorldSetUp() override
	{
		NetworkSubsystemInstance = this->UnrealWorld()->GetSubsystem<UFlecsNetworkWorldSubsystem>();
		check(NetworkSubsystemInstance);

		CaptureTransportInstance = NewObject<UFlecsReplicationCaptureTransport>(NetworkSubsystemInstance);
		NetworkSubsystemInstance->SetReplicationTransportForTesting(CaptureTransportInstance);

		RegisterReplicationComponent<FFlecsReplicationTestRequiredTag>();
		RegisterReplicationComponent<FFlecsReplicationTestValue>();
		RegisterReplicationComponent<FFlecsReplicationTestDontFragmentValue>();
		RegisterReplicationComponent<FFlecsReplicationTestNativeValue>();
		RegisterReplicationComponent<FFlecsReplicationTestTag>();
		RegisterReplicationComponent<FFlecsReplicationTestRelationship>();
		RegisterReplicationComponent<FFlecsReplicationTestValueRelationship>();
		RegisterReplicationComponent<FFlecsReplicationTestWithValue>();
		RegisterReplicationComponent<FFlecsReplicationTestLocalOnly>();
	}

	virtual void OnWorldTearDown() override
	{
		if (NetworkSubsystemInstance)
		{
			NetworkSubsystemInstance->SetReplicationTransportForTesting(nullptr);
		}

		CaptureTransportInstance = nullptr;
		NetworkSubsystemInstance = nullptr;
		Super::OnWorldTearDown();
	}

	NO_DISCARD UFlecsNetworkWorldSubsystem* NetworkSubsystem() const
	{
		return NetworkSubsystemInstance;
	}

	NO_DISCARD UFlecsReplicationCaptureTransport* CaptureTransport() const
	{
		return CaptureTransportInstance;
	}

private:
	template<typename T>
	void RegisterReplicationComponent()
	{
		const FFlecsComponentHandle Component = this->World()->template RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(this->World(), Component, Properties);
	}

	TObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystemInstance = nullptr;
	TObjectPtr<UFlecsReplicationCaptureTransport> CaptureTransportInstance = nullptr;
}; // struct TFlecsReplicationTest

#define FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(_ClassName, _TestDir, _Flags, _TestTags) \
	TEST_CLASS_WITH_BASE_AND_FLAGS_AND_TAGS(_ClassName, _TestDir, TFlecsReplicationTest, _Flags, _TestTags)

#endif // WITH_AUTOMATION_TESTS
