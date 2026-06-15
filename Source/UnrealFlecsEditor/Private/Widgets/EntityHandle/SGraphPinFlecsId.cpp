// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Widgets/EntityHandle/SGraphPinFlecsId.h"

#include "EdGraph/EdGraphPin.h"

#include "Widgets/EntityHandle/FlecsIdSelector.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Entities/FlecsDefaultEntityEngine.h"

#define LOCTEXT_NAMESPACE "SGraphPinFlecsId"

namespace UE::Flecs::IdPin
{
	FString ExtractNumericValue(FString Value)
	{
		Value.TrimStartAndEndInline();
		Value.RemoveFromStart(TEXT("("));
		Value.RemoveFromEnd(TEXT(")"));
		Value.RemoveFromStart(TEXT("FlecsId="));
		Value.RemoveFromStart(TEXT("Id="));
		return Value;
	}
} // namespace UE::Flecs::IdPin

void SGraphPinFlecsId::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget> SGraphPinFlecsId::GetDefaultValueWidget()
{
	Options.Empty();
	EntityOptions.Empty();

	Options.Add(MakeShared<FName>(FName(TEXT(""))));
	EntityOptions.Add(FFlecsId(flecs::entity::null().id()));

	FFlecsDefaultEntityEngine::Get().DefaultEntityQuery.each([this](flecs::entity Entity)
	{
		EntityOptions.Add(FFlecsId(Entity.id()));
		Options.Add(MakeShared<FName>(FName(Entity.name().c_str())));
	});

	const FString InitialRawVal =
		UE::Flecs::IdPin::ExtractNumericValue(GraphPinObj->GetDefaultAsString());
	const FText InitialIdText = FText::FromString(InitialRawVal);

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.FillWidth(1.f)
	[
		SNew(SFlecsIdSelector)
		.SelectedItem(TAttribute<TSharedPtr<FName>>::CreateLambda([this]() -> TSharedPtr<FName>
		{
			return GetSelectedName();
		}))
		.NoneEntityText(FName(TEXT("")))
		.PropertyHandle(nullptr)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		.OnEntitySelected(FOnFlecsEntitySelected::CreateLambda([this](const FFlecsId NewEntity)
		{
			if (IdTextInput.IsValid())
			{
				IdTextInput->SetText(FText::FromString(FString::Printf(TEXT("%llu"), NewEntity.GetId())));
			}

			if (GraphPinObj)
			{
				const FString NewDefaultValue = FString::Printf(TEXT("FlecsId=%llu"), NewEntity.GetId());
				GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, NewDefaultValue);
			}
		}))
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4.f, 0.f, 0.f, 0.f)
	[
		SNew(SBox)
		.WidthOverride(80.f)
		[
			SAssignNew(IdTextInput, SEditableTextBox)
			.Text(InitialIdText)
			.HintText(LOCTEXT("IdHint", "ID"))
			.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
			.OnVerifyTextChanged(FOnVerifyTextChanged::CreateLambda([](const FText& InText, FText& OutError) -> bool
			{
				for (const TCHAR Ch : InText.ToString())
				{
					if (!FChar::IsDigit(Ch))
					{
						OutError = LOCTEXT("NumericOnly", "Numeric input only");
						return false;
					}
				}
				
				return true;
			}))
			.OnTextCommitted(FOnTextCommitted::CreateLambda([this](const FText& NewText, ETextCommit::Type CommitType)
			{
				if (CommitType == ETextCommit::OnCleared)
				{
					return;
				}

				const FString TextStr = NewText.ToString();
				const uint64 Id = TextStr.IsNumeric()
					? FCString::Strtoui64(*TextStr, nullptr, 10)
					: 0;

				if (GraphPinObj)
				{
					const FString NewDefaultValue = FString::Printf(TEXT("FlecsId=%llu"), Id);
					GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, NewDefaultValue);
				}
				
			}))
		]
	];
}

TSharedPtr<FName> SGraphPinFlecsId::GetSelectedName() const
{
	const FString CurrentValue =
		UE::Flecs::IdPin::ExtractNumericValue(GraphPinObj->GetDefaultAsString());
	const uint64 CurrentId = FCString::Strtoui64(*CurrentValue, nullptr, 10);

	for (int32 Index = 1; Index < EntityOptions.Num(); ++Index)
	{
		if (EntityOptions[Index].GetId() == CurrentId)
		{
			return Options[Index];
		}
	}

	return MakeShared<FName>(FName(TEXT("")));
}

#undef LOCTEXT_NAMESPACE
