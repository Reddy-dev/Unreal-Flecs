#include "SolidMacrosEditor.h"

#include "EdGraphUtilities.h"
#include "PropertyEditorModule.h"

#include "Types/SolidEnumSelectorDetails.h"
#include "Types/SolidEnumSelectorPinFactory.h"

#define LOCTEXT_NAMESPACE "FSolidMacrosEditorModule"

void FSolidMacrosEditorModule::StartupModule()
{
	RegisterDetailsCustomizations();

	SolidEnumSelectorPinFactory = MakeShared<FSolidEnumSelectorPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(SolidEnumSelectorPinFactory);
}

void FSolidMacrosEditorModule::ShutdownModule()
{
	if (SolidEnumSelectorPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(SolidEnumSelectorPinFactory);
		SolidEnumSelectorPinFactory.Reset();
	}

	UnregisterDetailsCustomizations();
}

void FSolidMacrosEditorModule::RegisterDetailsCustomizations()
{
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.RegisterCustomPropertyTypeLayout("SolidEnumSelector",
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSolidEnumSelectorDetails::MakeInstance));
}

void FSolidMacrosEditorModule::UnregisterDetailsCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		return;
	}

	FPropertyEditorModule& PropertyEditor = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditor.UnregisterCustomPropertyTypeLayout("SolidEnumSelector");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSolidMacrosEditorModule, SolidMacrosEditor)
