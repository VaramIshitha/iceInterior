// Copyright Lost in Game Studio. All Right Reserved

#include "HierarchyItem.h"

#include "Editor/TouchInterfaceDesignerStyle.h"
#include "Slate/Public/Widgets/Input/SEditableText.h"
#include "Slate/Public/Widgets/Text/SInlineEditableTextBlock.h"
#include "Editor/VirtualControlDesignerEditor.h"
#include "VirtualControlSetup.h"
#include "Styling/SlateColor.h"

#define LOCTEXT_NAMESPACE "TouchDesigner"

void SHierarchyItem::Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InEditor)
{
	TouchDesignerEditorPtr = InEditor;
	
	ControlName = InArgs._ControlName;
	Type = InArgs._Type;
	OnClicked = InArgs._OnClicked;
	OnTextChanged = InArgs._OnTextChanged;
	bStartInEditMode = InArgs._bStartInEditMode;
	bIsSelected = false;

	FName StyleName;

	switch (InArgs._Type)
	{
	case EControlType::Button:
		StyleName = "Hierarchy.Button.Icon";
		break;
	case EControlType::Joystick:
		StyleName = "Hierarchy.Joystick.Icon";
		break;
	case EControlType::TouchRegion:
		StyleName = "Hierarchy.TouchRegion.Icon";
		break;
	default:
		break;
	}
	
	ChildSlot
	[
		//TODO: Remove SButton and use border instead
		SNew(SButton)
		//.ContentPadding(FMargin(2))
		.HAlign(HAlign_Fill)
		.OnClicked(FOnClicked::CreateSP(this, &SHierarchyItem::ExecuteOnClicked))
		.ButtonStyle(GetSlateStyle(), "SimpleSharpButton")
		//.ButtonStyle(FEditorStyle::Get(), "SimpleSharpButton")
		.ButtonColorAndOpacity(this, &SHierarchyItem::GetBorderColor)
		.ForegroundColor(FSlateColor(FLinearColor::White))
		[
			SNew(SHorizontalBox)

			//Item Icon
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FTouchInterfaceDesignerStyle::Get().GetBrush(StyleName))
			]

			//Item Name
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SAssignNew(InlineEditableText, SInlineEditableTextBlock)
				.Text(this, &SHierarchyItem::GetDisplayText)
				.OnTextCommitted(FOnTextCommitted::CreateSP(this, &SHierarchyItem::ExecuteOnTextChanged))
				.OnVerifyTextChanged(FOnVerifyTextChanged::CreateSP(InEditor.Get(), &FVirtualControlDesignerEditor::VerifyTextCommitted))
				.MultiLine(false)
				.IsReadOnly(true)
				//.Style()
			]

			//Lock Button
			+SHorizontalBox::Slot()
			.FillWidth(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.OnClicked(this, &SHierarchyItem::HandleOnLockItem)
			]
		]
	];
}

void SHierarchyItem::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsHovered = true;
	TouchDesignerEditorPtr.Pin()->SetControlHoverState(ControlName, true, true, false);

	if (bIsSelected)
	{
		InlineEditableText->SetReadOnly(false);
		bIsReadOnly = false;
	}
	
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SHierarchyItem::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	bIsHovered = false;
	TouchDesignerEditorPtr.Pin()->SetControlHoverState(ControlName, false, true, false);

	InlineEditableText->SetReadOnly(true);
	bIsReadOnly = true;
	
	SCompoundWidget::OnMouseLeave(MouseEvent);
}

FReply SHierarchyItem::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (!bIsSelected)
		{
			if (OnClicked.IsBound())
			{
				OnClicked.Execute(ControlName);
				bIsSelected = true;
			}
		}
		
		//Todo: Get CommandList in TouchDesignerEditor ?
		FMenuBuilder MenuBuilder(true,NULL);
		
		FUIAction DeleteAction(FExecuteAction::CreateSP(TouchDesignerEditorPtr.Pin().ToSharedRef(), &FVirtualControlDesignerEditor::RemoveSelected));
		FUIAction EditTextAction(FExecuteAction::CreateSP(this, &SHierarchyItem::EditText));

		MenuBuilder.BeginSection(NAME_None, NSLOCTEXT("PropertyView", "ExpansionHeading", "Control"));
		MenuBuilder.AddMenuEntry(LOCTEXT("EditName", "Edit Name"), LOCTEXT("ChangeControlName", "Change Control Name"), FSlateIcon(), EditTextAction);
		MenuBuilder.AddMenuEntry(LOCTEXT("Delete", "Delete"),LOCTEXT("Delete control", "Delete Control"), FSlateIcon(), DeleteAction);
		MenuBuilder.EndSection();

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SHierarchyItem::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	EditText();
	return SCompoundWidget::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FText SHierarchyItem::GetDisplayText() const
{
	return FText::FromName(ControlName);
}

FSlateColor SHierarchyItem::GetBorderColor() const
{
	if (bIsHovered && !bIsSelected)
	{
		return FTouchInterfaceDesignerStyle::Get().GetColor("HoverColor");
	}
	
	if (bIsSelected)
	{
		//return FCoreStyle::Get().GetSlateColor("SelectionColor");
		return GetSlateStyle().GetSlateColor("SelectionColor");
	}

	return FLinearColor(1,1,1,0);
}

FReply SHierarchyItem::ExecuteOnClicked()
{
	if (!bIsSelected)
	{
		InlineEditableText->SetReadOnly(false);
		bIsReadOnly = false;

		if (OnClicked.IsBound())
		{
			OnClicked.Execute(ControlName);
			bIsSelected = true;
			return FReply::Handled().SetUserFocus(InlineEditableText.ToSharedRef());
		}
	}
	
	return FReply::Unhandled();
}

void SHierarchyItem::ExecuteOnTextChanged(const FText& NewText, ETextCommit::Type CommitType)
{
	/*switch (CommitType)
	{
	case ETextCommit::Default:
		GLog->Log("Default Commit");
		break;
	case ETextCommit::OnCleared:
		GLog->Log("OnCleared Commit");
		break;
	case ETextCommit::OnEnter:
		GLog->Log("OnEnter Commit");
		break;
	case ETextCommit::OnUserMovedFocus:
		GLog->Log("OnUserMovedFocus Commit");
		break;
	}*/
	
	const FName NewName = *NewText.ToString();
	
	if (NewName != ControlName)
	{
		OnTextChanged.ExecuteIfBound(ControlName, NewName);
		ControlName = NewName;
	}

	InlineEditableText->SetReadOnly(true);
	bIsReadOnly = true;
	//bIsSelected = false;
}

FReply SHierarchyItem::HandleOnLockItem()
{
	//Todo: Add lock function
	return FReply::Unhandled();
}

void SHierarchyItem::EditText()
{
	FSlateApplication::Get().ForEachUser([&](FSlateUser& User)
	{
		FSlateApplication::Get().SetUserFocus(User.GetUserIndex(),InlineEditableText);
	});
	
	InlineEditableText->SetReadOnly(false);
	InlineEditableText->EnterEditingMode();
}

const ISlateStyle& SHierarchyItem::GetSlateStyle() const
{
#if ENGINE_MAJOR_VERSION > 4
	return FAppStyle::Get();
#else
	return FEditorStyle::Get();
#endif
}

#undef LOCTEXT_NAMESPACE
