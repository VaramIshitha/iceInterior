// Copyright Lost in Game Studio. All Right Reserved

#include "SHierarchyCategory.h"

#include "DetailLayoutBuilder.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "HierarchyItem.h"

#define LOCTEXT_NAMESPACE "SHierarchyCategory"

void SHierarchyCategory::Construct(const FArguments& InArgs)
{
	//Todo: Reformat category name for better visibility
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.Padding(FMargin(5))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderImage(this, &SHierarchyCategory::GetBorderImage)
			.BorderBackgroundColor(FLinearColor(0.6,0.6,0.6,1))
			.OnMouseDoubleClick(this, &SHierarchyCategory::HandleOnMouseDoubleClick)
			[
				SNew(SHorizontalBox)

				//Expand button
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.OnClicked(this, &SHierarchyCategory::HandleOnArrowClicked)
					[
						SNew(SImage)
						.Image(this, &SHierarchyCategory::GetExpandedArrow)
					]
				]

				//Category text
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFontBold())
					.Text(InArgs._CategoryName)
				]			
			]
		]

		//Hierarchy Item container
		+SVerticalBox::Slot()
		[
			//Todo: Make reorder function
			SAssignNew(Container, SVerticalBox)
			.Visibility(this, &SHierarchyCategory::IsExpandedVisibility)
		]
	];
	
	OnExpandStateChanged = InArgs._OnExpandStateChanged;
}

FReply SHierarchyCategory::HandleOnMouseDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	bIsExpanded = !bIsExpanded;
	
	if (OnExpandStateChanged.IsBound())
	{
		OnExpandStateChanged.Execute(bIsExpanded);
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply SHierarchyCategory::HandleOnArrowClicked()
{
	bIsExpanded = !bIsExpanded;
	return FReply::Handled();
}

void SHierarchyCategory::AddItem(TSharedRef<SHierarchyItem> Item)
{
	Container->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	[
		Item
	];
}

int32 SHierarchyCategory::RemoveItem(TSharedRef<SWidget> ItemToRemove)
{
	Container->RemoveSlot(ItemToRemove);
	return Container->GetChildren()->Num();
}

EVisibility SHierarchyCategory::IsExpandedVisibility() const
{
	return bIsExpanded ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

EVisibility SHierarchyCategory::GetTextVisibility() const
{
	return bShowText ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

const FSlateBrush* SHierarchyCategory::GetBorderImage() const
{
#if ENGINE_MAJOR_VERSION > 4
	const ISlateStyle& SlateStyle = FAppStyle::Get();
#else
	const ISlateStyle& SlateStyle = FEditorStyle::Get();
#endif
	
	if (IsHovered())
	{
		return IsExpanded() ? SlateStyle.GetBrush("DetailsView.CategoryTop_Hovered") : SlateStyle.GetBrush("DetailsView.CollapsedCategory_Hovered");
	}

	return IsExpanded() ? SlateStyle.GetBrush("DetailsView.CategoryTop") : SlateStyle.GetBrush("DetailsView.CollapsedCategory");
}

const FSlateBrush* SHierarchyCategory::GetExpandedArrow() const
{
#if ENGINE_MAJOR_VERSION > 4
	const ISlateStyle& SlateStyle = FAppStyle::Get();
#else
	const ISlateStyle& SlateStyle = FEditorStyle::Get();
#endif
	
	if (bIsExpanded)
	{
		return SlateStyle.GetBrush("TreeArrow_Expanded");
	}

	return SlateStyle.GetBrush("TreeArrow_Collapsed");
}

#undef LOCTEXT_NAMESPACE
