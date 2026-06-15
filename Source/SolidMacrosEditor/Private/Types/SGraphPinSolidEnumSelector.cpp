// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Types/SGraphPinSolidEnumSelector.h"

#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "Misc/OutputDeviceNull.h"
#include "ScopedTransaction.h"
#include "SSearchableComboBox.h"
#include "UObject/Class.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SGraphPinSolidEnumSelector"

void SGraphPinSolidEnumSelector::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget> SGraphPinSolidEnumSelector::GetDefaultValueWidget()
{
	LoadSelector();
	BuildEnumClassOptions();
	RebuildEnumValueOptions();

	return SNew(SVerticalBox)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.MinDesiredWidth(180.f)
			[
				SNew(SSearchableComboBox)
				.OptionsSource(&EnumClassOptions)
				.InitiallySelectedItem(FindSelectedEnumClassOption())
				.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
				.OnGenerateWidget(this, &SGraphPinSolidEnumSelector::GenerateEnumClassWidget)
				.OnSelectionChanged(this, &SGraphPinSolidEnumSelector::OnEnumClassChanged)
				.Content()
				[
					SNew(STextBlock)
					.Text(this, &SGraphPinSolidEnumSelector::GetSelectedEnumClassText)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 2.f, 0.f, 0.f)
		[
			SAssignNew(EnumValueComboBox, SComboBox<TSharedPtr<int64>>)
				.OptionsSource(&EnumValueOptions)
				.InitiallySelectedItem(FindSelectedEnumValueOption())
				.IsEnabled(this, &SGraphPin::GetDefaultValueIsEditable)
				.OnGenerateWidget(this, &SGraphPinSolidEnumSelector::GenerateEnumValueWidget)
				.OnSelectionChanged(this, &SGraphPinSolidEnumSelector::OnEnumValueChanged)
				.Content()
				[
					SNew(STextBlock)
					.Text(this, &SGraphPinSolidEnumSelector::GetSelectedEnumValueText)
				]
		];
}

void SGraphPinSolidEnumSelector::LoadSelector()
{
	Selector = FSolidEnumSelector();

	if (!GraphPinObj)
	{
		return;
	}

	const FString DefaultValue = GraphPinObj->GetDefaultAsString();
	if (DefaultValue.IsEmpty())
	{
		return;
	}

	FOutputDeviceNull NullOutput;
	FSolidEnumSelector::StaticStruct()->ImportText(
		*DefaultValue,
		&Selector,
		nullptr,
		PPF_SerializedAsImportText,
		&NullOutput,
		GraphPinObj->PinName.ToString());
}

void SGraphPinSolidEnumSelector::SaveSelector()
{
	if (!GraphPinObj)
	{
		return;
	}

	FString ExportedValue;
	FSolidEnumSelector::StaticStruct()->ExportText(
		ExportedValue,
		&Selector,
		&Selector,
		nullptr,
		PPF_SerializedAsImportText,
		nullptr);

	if (ExportedValue == GraphPinObj->GetDefaultAsString())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("ChangeSolidEnumSelectorPinValue", "Change Enum Selector Pin Value"));
	GraphPinObj->Modify();
	GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, ExportedValue);
}

void SGraphPinSolidEnumSelector::BuildEnumClassOptions()
{
	EnumClassOptions.Reset();
	EnumClassesByOption.Reset();

	for (TObjectIterator<UEnum> It; It; ++It)
	{
		if (It->HasMetaData(TEXT("Hidden")))
		{
			continue;
		}

		const TSharedPtr<FString> Option = MakeShared<FString>(It->GetName());
		EnumClassOptions.Add(Option);
		EnumClassesByOption.Add(Option, *It);
	}

	EnumClassOptions.Sort([](const TSharedPtr<FString>& Left, const TSharedPtr<FString>& Right)
	{
		return *Left < *Right;
	});
}

void SGraphPinSolidEnumSelector::RebuildEnumValueOptions()
{
	EnumValueOptions.Reset();

	if (!Selector.Class)
	{
		return;
	}

	const int32 EnumCount = FMath::Max(Selector.Class->NumEnums() - 1, 0);
	for (int32 Index = 0; Index < EnumCount; ++Index)
	{
		if (Selector.Class->HasMetaData(TEXT("Hidden"), Index) ||
			Selector.Class->HasMetaData(TEXT("Spacer"), Index))
		{
			continue;
		}

		EnumValueOptions.Add(MakeShared<int64>(Selector.Class->GetValueByIndex(Index)));
	}
}

void SGraphPinSolidEnumSelector::OnEnumClassChanged(
	const TSharedPtr<FString> SelectedItem,
	ESelectInfo::Type SelectInfo)
{
	if (!SelectedItem.IsValid())
	{
		return;
	}

	UEnum* const* EnumClass = EnumClassesByOption.Find(SelectedItem);
	if (!EnumClass)
	{
		return;
	}

	Selector.Class = *EnumClass;
	RebuildEnumValueOptions();
	Selector.Value = EnumValueOptions.IsEmpty() ? 0 : *EnumValueOptions[0];
	SaveSelector();

	if (EnumValueComboBox.IsValid())
	{
		EnumValueComboBox->RefreshOptions();
		EnumValueComboBox->SetSelectedItem(FindSelectedEnumValueOption());
	}
}

void SGraphPinSolidEnumSelector::OnEnumValueChanged(
	const TSharedPtr<int64> SelectedItem,
	ESelectInfo::Type SelectInfo)
{
	if (!SelectedItem.IsValid())
	{
		return;
	}

	Selector.Value = *SelectedItem;
	SaveSelector();
}

TSharedRef<SWidget> SGraphPinSolidEnumSelector::GenerateEnumClassWidget(const TSharedPtr<FString> Item) const
{
	return SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty());
}

TSharedRef<SWidget> SGraphPinSolidEnumSelector::GenerateEnumValueWidget(const TSharedPtr<int64> Item) const
{
	if (!Selector.Class || !Item.IsValid())
	{
		return SNew(STextBlock).Text(FText::GetEmpty());
	}

	const FText DisplayName = Selector.Class->GetDisplayNameTextByValue(*Item);
	return SNew(STextBlock)
		.Text(DisplayName.IsEmpty() ? FText::AsNumber(*Item) : DisplayName);
}

FText SGraphPinSolidEnumSelector::GetSelectedEnumClassText() const
{
	return Selector.Class
		? Selector.Class->GetDisplayNameText()
		: LOCTEXT("SelectEnumClass", "Select Enum");
}

FText SGraphPinSolidEnumSelector::GetSelectedEnumValueText() const
{
	if (!Selector.Class)
	{
		return LOCTEXT("SelectEnumValue", "Select Value");
	}

	const FText DisplayName = Selector.Class->GetDisplayNameTextByValue(Selector.Value);
	return DisplayName.IsEmpty() ? FText::AsNumber(Selector.Value) : DisplayName;
}

TSharedPtr<FString> SGraphPinSolidEnumSelector::FindSelectedEnumClassOption() const
{
	if (!Selector.Class)
	{
		return nullptr;
	}

	const TSharedPtr<FString>* Option = EnumClassOptions.FindByPredicate([this](const TSharedPtr<FString>& Item)
	{
		return Item.IsValid() && *Item == Selector.Class->GetName();
	});
	return Option ? *Option : nullptr;
}

TSharedPtr<int64> SGraphPinSolidEnumSelector::FindSelectedEnumValueOption() const
{
	const TSharedPtr<int64>* Option = EnumValueOptions.FindByPredicate([this](const TSharedPtr<int64>& Item)
	{
		return Item.IsValid() && *Item == Selector.Value;
	});
	return Option ? *Option : nullptr;
}

#undef LOCTEXT_NAMESPACE
