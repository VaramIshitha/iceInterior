// Copyright Lost in Game Studio. All Right Reserved

#pragma once

DECLARE_DELEGATE_OneParam(FOnStateChanged, bool);

class SHierarchyCategory : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHierarchyCategory){}
		SLATE_ARGUMENT(FText, CategoryName)
		SLATE_EVENT(FOnStateChanged, OnExpandStateChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	void AddItem(TSharedRef<class SHierarchyItem> Item);

	/** Return item amount after delete */
	int32 RemoveItem(TSharedRef<SWidget> ItemToRemove);
	
	bool IsExpanded() const { return bIsExpanded; }

	void SetIsExpanded(const bool bExpand) { bIsExpanded = bExpand; }

	EVisibility IsExpandedVisibility() const;

protected:
	FOnStateChanged OnExpandStateChanged;
	FOnClicked OnAddControl;
	
private:
	bool bIsExpanded = true;
	bool bShowText = false;

	FReply HandleOnMouseDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent);
	FReply HandleOnArrowClicked();
	
	EVisibility GetTextVisibility() const;

	const FSlateBrush* GetBorderImage() const;

	const FSlateBrush* GetExpandedArrow() const;

	TSharedPtr<SVerticalBox> Container;
};
