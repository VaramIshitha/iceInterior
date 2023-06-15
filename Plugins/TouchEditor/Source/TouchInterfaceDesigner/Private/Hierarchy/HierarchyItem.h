// Copyright Lost in Game Studio. All Right Reserved

#pragma once

//#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FVirtualControlDesignerEditor;

enum class EControlType : uint8;

DECLARE_DELEGATE_OneParam(FOnControl, FName);
DECLARE_DELEGATE_TwoParams(FOnControlNameChanged, FName, FName);

class SHierarchyItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHierarchyItem) :
	_ControlName(),
	_bStartInEditMode(false)
	{}
		SLATE_ARGUMENT(FName, ControlName)
		SLATE_ARGUMENT(EControlType, Type)
		SLATE_ARGUMENT(bool, bStartInEditMode)
	
		SLATE_EVENT(FOnControl, OnClicked)
		SLATE_EVENT(FOnControlNameChanged, OnTextChanged)
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InEditor);

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	void SetIsSelected(bool InIsSelected) {bIsSelected = InIsSelected;}
	void SetIsHovered(bool InIsHovered) {bIsHovered = InIsHovered;}

	EControlType GetItemType() const { return Type; }

	// Set user focus to this item and edit text
	void EditText();

private:
	const ISlateStyle& GetSlateStyle() const;
	
	FName ControlName;
	
	TSharedPtr<SInlineEditableTextBlock> InlineEditableText;
	
	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;
	
	bool bIsSelected;
	bool bIsHovered;
	bool bIsReadOnly;
	bool bStartInEditMode;

	EControlType Type;
	
	FText GetDisplayText() const;

	FSlateColor GetBorderColor() const;
	
	FOnControl OnClicked;
	FOnControlNameChanged OnTextChanged;

	FReply ExecuteOnClicked();
	void ExecuteOnTextChanged(const FText& NewText, ETextCommit::Type CommitType);

	FReply HandleOnLockItem();
};
