// Copyright Lost in Game Studio. All Right Reserved

#include "SHierarchyTab.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "VirtualControlSetup.h"
#include "HierarchyItem.h"
#include "SHierarchyCategory.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "VCD_HierarchyTab"

void SHierarchyTab::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor)
{
	TouchDesignerEditorPtr = InTouchDesignerEditor;
	
	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(4,4,4,0))
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarAlwaysVisible(false)

			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SAssignNew(CategoryContainer, SVerticalBox)
			]
		]
	];

	GenerateControlButtons();
}

void SHierarchyTab::GenerateControlButtons()
{
	const UVirtualControlSetup* TouchDesignerInterface = TouchDesignerEditorPtr.Pin()->GetVirtualControlSetup();
	TArray<FVirtualControl> Controls = TouchDesignerInterface->VirtualControls;
	
	for (int32 Index = 0; Index < Controls.Num(); ++Index)
	{
		AddNewItem(Controls[Index].Type, Controls[Index].ControlName);
	}
}

void SHierarchyTab::AddNewItem(EControlType Type, const FName ControlName)
{
	//const UEnum* ControlType = FindObject<UEnum>(ANY_PACKAGE, TEXT("EControlType"));
	const UEnum* ControlType = FindObject<UEnum>(UVirtualControlSetup::StaticClass(), TEXT("EControlType"));
	const FString CategoryString = ControlType ? ControlType->GetNameStringByValue(static_cast<int32>(Type)) : "Invalid Category Name";
	const FName CategoryName = ControlType ? ControlType->GetNameByIndex(static_cast<int32>(Type)) : NAME_None;

	TSharedPtr<SHierarchyCategory> Category = nullptr;
	
	if (!Categories.Contains(CategoryName))
	{
		//Create new category
		Category = SNew(SHierarchyCategory).CategoryName(FText::FromString(CategoryString));
		CategoryContainer->AddSlot()
		.AutoHeight()
		[
			Category.ToSharedRef()
		];
		Categories.Add(CategoryName, Category);
	}
	else
	{
		//Recover category
		Category = Categories.FindRef(CategoryName);
	}
	
	TSharedRef<SHierarchyItem> Widget = SNew(SHierarchyItem, TouchDesignerEditorPtr.Pin())
	.ControlName(ControlName)
	.Type(Type)
	.OnClicked(FOnControl::CreateSP(this, &SHierarchyTab::OnControlSelected))
	.OnTextChanged(FOnControlNameChanged::CreateSP(this, &SHierarchyTab::OnControlNameChanged))
	.bStartInEditMode(false);

	Category->AddItem(Widget);
	HierarchyItems.Add(ControlName, Widget);
}

FReply SHierarchyTab::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//Todo: Test click on ControlButton here ? If So, SButton not necessary in ControlButton
		TouchDesignerEditorPtr.Pin()->UnselectControl();
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

void SHierarchyTab::SelectButton(FName ControlName)
{
	if (ButtonSelected.IsValid()) ButtonSelected->SetIsSelected(false);

	if (HierarchyItems.Contains(ControlName))
	{
		TSharedPtr<SHierarchyItem> Button = HierarchyItems.FindRef(ControlName);
		ButtonSelected = Button;
		ButtonSelected->SetIsSelected(true);
	}
}

void SHierarchyTab::UnselectButton() const
{
	ButtonSelected->SetIsSelected(false);
}

void SHierarchyTab::EditItemText(const FName ControlName) const
{
	check(HierarchyItems.Contains(ControlName))
	HierarchyItems.FindRef(ControlName)->EditText();
}

void SHierarchyTab::SetButtonHoverEffect(const FName ControlName, const bool Enable)
{
	if (ButtonHovered.IsValid()) ButtonHovered->SetIsHovered(false);

	ButtonHovered = GetButtonByName(ControlName);
	if (ButtonHovered.IsValid()) ButtonHovered->SetIsHovered(Enable);
}

void SHierarchyTab::ClearButtonEffect() const
{
	if (ButtonHovered.IsValid()) ButtonHovered->SetIsHovered(false);
	if (ButtonSelected.IsValid()) ButtonSelected->SetIsSelected(false);
}

void SHierarchyTab::RemoveItem(const FName ControlName) const
{	
	TSharedPtr<SHierarchyItem> ItemToRemove = HierarchyItems.FindRef(ControlName);

	//const UEnum* ControlType = FindObject<UEnum>(ANY_PACKAGE, TEXT("EControlType"));
	const UEnum* ControlType = FindObject<UEnum>(UVirtualControlSetup::StaticClass(), TEXT("EControlType"));
	const FString CategoryString = ControlType ? ControlType->GetNameStringByValue(static_cast<int32>(ItemToRemove->GetItemType())) : "Invalid Category Name";
	const FName CategoryName = ControlType ? ControlType->GetNameByIndex(static_cast<int32>(ItemToRemove->GetItemType())) : NAME_None;

	//Remove category dynamically on item remove
	if (Categories.FindRef(CategoryName)->RemoveItem(ItemToRemove.ToSharedRef()) <= 0)
	{
		UE_LOG(LogTemp, Display, TEXT("Remove category"))
	}

	//Todo: remove from Buttons TMap
}

void SHierarchyTab::Refresh()
{	
	HierarchyItems.Empty();
	GenerateControlButtons();
}

void SHierarchyTab::OnControlSelected(FName ControlName)
{
	SelectButton(ControlName);
	TouchDesignerEditorPtr.Pin()->SelectControl(ControlName);
}

void SHierarchyTab::OnControlNameChanged(FName ControlName, FName NewControlName)
{
	TouchDesignerEditorPtr.Pin()->ChangeControlName(ControlName, NewControlName);

	TSharedPtr<SHierarchyItem> RemovedButton = HierarchyItems.FindAndRemoveChecked(ControlName);
	HierarchyItems.Add(NewControlName, RemovedButton);
}

TSharedPtr<SHierarchyItem> SHierarchyTab::GetButtonByName(FName ControlName) const
{
	if (HierarchyItems.Contains(ControlName))
	{
		return HierarchyItems.FindRef(ControlName);
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
