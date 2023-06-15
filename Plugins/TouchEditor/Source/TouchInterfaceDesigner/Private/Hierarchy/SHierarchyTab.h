// Copyright Lost in Game Studio. All Right Reserved

#pragma once

class FVirtualControlDesignerEditor;
class UVirtualControlSetup;
class SHierarchyCategory;
class SHierarchyItem;

enum class EControlType : uint8;

class SHierarchyTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHierarchyTab){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InTouchDesignerEditor);

private:
	/** Generate buttons from VirtualControlSetup */
	void GenerateControlButtons();

public:
	/** Add new button to button stack */
	void AddNewItem(EControlType Type, const FName ControlName);
	
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
	void SelectButton(FName ControlName);
	void UnselectButton() const;

	void EditItemText(const FName ControlName) const;

	/** Enable/Disable hover effect on desired button */
	void SetButtonHoverEffect(const FName ControlName, const bool Enable);

	/** Clear all effect on current selected/hovered button */
	void ClearButtonEffect() const;

	/** Remove existing button from button stack */
	void RemoveItem(const FName ControlName) const;

	/** Check control available in Touch Designer Interface and refresh/add buttons if necessary */
	void Refresh();
	
private:
	TWeakPtr<FVirtualControlDesignerEditor> TouchDesignerEditorPtr;
	TWeakPtr<SHierarchyCategory> JoystickCategory;
	TWeakPtr<SHierarchyCategory> ButtonCategory;

	TSharedPtr<SVerticalBox> CategoryContainer;

	TSharedPtr<SHierarchyItem> ButtonSelected;
	TSharedPtr<SHierarchyItem> ButtonHovered;
	
	TMap<FName, TSharedPtr<SHierarchyItem>> HierarchyItems;
	
	TMap<FName, TSharedPtr<SHierarchyCategory>> Categories;
	
	void OnControlSelected(FName ControlName);
	void OnControlNameChanged(FName ControlName, FName NewControlName);
	
	TSharedPtr<SHierarchyItem> GetButtonByName(FName ControlName) const;
};
