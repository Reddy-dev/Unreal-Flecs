// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "Widgets/Input/SComboBox.h"

#include "Types/SolidEnumSelector.h"

class SGraphPinSolidEnumSelector : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SGraphPinSolidEnumSelector) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

protected:
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

private:
	void LoadSelector();
	void SaveSelector();
	void BuildEnumClassOptions();
	void RebuildEnumValueOptions();

	void OnEnumClassChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	void OnEnumValueChanged(TSharedPtr<int64> SelectedItem, ESelectInfo::Type SelectInfo);

	TSharedRef<SWidget> GenerateEnumClassWidget(TSharedPtr<FString> Item) const;
	TSharedRef<SWidget> GenerateEnumValueWidget(TSharedPtr<int64> Item) const;

	FText GetSelectedEnumClassText() const;
	FText GetSelectedEnumValueText() const;

	TSharedPtr<FString> FindSelectedEnumClassOption() const;
	TSharedPtr<int64> FindSelectedEnumValueOption() const;

	FSolidEnumSelector Selector;
	TArray<TSharedPtr<FString>> EnumClassOptions;
	TArray<TSharedPtr<int64>> EnumValueOptions;
	TMap<TSharedPtr<FString>, UEnum*> EnumClassesByOption;
	TSharedPtr<SComboBox<TSharedPtr<int64>>> EnumValueComboBox;

}; // class SGraphPinSolidEnumSelector
