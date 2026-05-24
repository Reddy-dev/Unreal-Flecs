// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Widgets/EntityHandle/FlecsIdCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"

#include "Entities/FlecsDefaultEntityEngine.h"

#include "Widgets/EntityHandle/FlecsIdSelector.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "FlecsIdCustomization"

TSharedRef<IPropertyTypeCustomization> FFlecsIdCustomization::MakeInstance()
{
	return MakeShareable(new FFlecsIdCustomization());
}

void FFlecsIdCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	
	HeaderRow.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FFlecsIdCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;

	FString CurrentRawVal;
	FText InitialIdText = FText::GetEmpty();
	if (StructPropertyHandle->GetValueAsFormattedString(CurrentRawVal) == FPropertyAccess::Success)
	{
		CurrentRawVal.RemoveFromStart(TEXT("FlecsId="));
		InitialIdText = FText::FromString(CurrentRawVal);
	}

	StructBuilder.AddCustomRow(LOCTEXT("FlecsEntityHandle", "Flecs Entity Handle"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SelectedEntity", "Selected Entity"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	.MinDesiredWidth(300.f)
	.MaxDesiredWidth(500.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(SFlecsIdSelector)
			.SelectedItem(TAttribute<TSharedPtr<FName>>::CreateLambda([this]() -> TSharedPtr<FName>
			{
				return GetSelectedItem();
			}))
			.NoneEntityText(FName(TEXT("")))
			.PropertyHandle(StructPropertyHandle)
			.OnEntitySelected(FOnFlecsEntitySelected::CreateLambda([this](const FFlecsId NewEntity)
			{
				if (IdTextInput.IsValid())
				{
					IdTextInput->SetText(FText::FromString(FString::Printf(TEXT("%llu"), NewEntity.Id)));
				}

				if (PropertyHandle.IsValid() && PropertyHandle->IsValidHandle())
				{
					const FScopedTransaction Transaction(LOCTEXT("ChangeEntity", "Change Entity"));
					PropertyHandle->NotifyPreChange();

					PropertyHandle->EnumerateRawData([&NewEntity](void* RawData, int32, int32)
					{
						if (RawData == nullptr)
						{
							return false;
						}

						*static_cast<FFlecsId*>(RawData) = NewEntity;
						return true;
					});

					PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
					PropertyHandle->NotifyFinishedChangingProperties();
				}
			}))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.f, 0.f, 0.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(120.f)
			[
				SAssignNew(IdTextInput, SEditableTextBox)
				.Text(InitialIdText)
				.HintText(LOCTEXT("CustomIdHint", "ID"))
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

					if (PropertyHandle.IsValid() && PropertyHandle->IsValidHandle())
					{
						const FScopedTransaction Transaction(LOCTEXT("ChangeEntityId", "Change Entity ID"));
						PropertyHandle->NotifyPreChange();

						PropertyHandle->EnumerateRawData([Id](void* RawData, int32, int32)
						{
							if (RawData)
							{
								static_cast<FFlecsId*>(RawData)->Id = Id;
							}

							return true;
						});

						PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
						PropertyHandle->NotifyFinishedChangingProperties();
					}
				}))
			]
		]
	];
}

TSharedPtr<FName> FFlecsIdCustomization::GetSelectedItem() const
{
	TSharedPtr<FName> SelectedItem;
	
	if (PropertyHandle.IsValid() && PropertyHandle->IsValidHandle())
	{
		PropertyHandle->EnumerateRawData([this, &SelectedItem](void* RawData, int32 DataIndex, int32 NumDatas)
		{
			if (RawData == nullptr)
			{
				SelectedItem.Reset();
				return false;
			}

			const FFlecsId* EntityId = static_cast<FFlecsId*>(RawData);
			const flecs::entity Entity = FFlecsDefaultEntityEngine::Get().DefaultEntityWorld->get_alive(*EntityId);

			if (*EntityId == FFlecsId::Null() || !Entity.is_valid())
			{
				SelectedItem.Reset();
				return false;
			}

			const FName CurrentValue(Entity.name().c_str());

			if (DataIndex > 0 && SelectedItem.IsValid() && (SelectedItem->GetNumber() != CurrentValue.GetNumber()))
			{
				SelectedItem.Reset();
				return false;
			}

			SelectedItem = MakeShared<FName>(CurrentValue);
			return true;
		});
	}

	return SelectedItem.IsValid() ? SelectedItem : MakeShared<FName>(FName(TEXT("None")));
}


#undef LOCTEXT_NAMESPACE
