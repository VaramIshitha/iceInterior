// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FVirtualControlDesignerEditor;
class SWrapBox;

/**
 * 
 */
class SPaletteTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaletteTab)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<FVirtualControlDesignerEditor> InVCDEditor);

private:
	void CreateItem();
	
	TSharedPtr<SWrapBox> WrapBox;

	TWeakPtr<FVirtualControlDesignerEditor> VCDEditor;
};
