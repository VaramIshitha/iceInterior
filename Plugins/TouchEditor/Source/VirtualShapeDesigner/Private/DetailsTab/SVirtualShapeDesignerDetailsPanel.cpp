// Copyright Lost in Game Studio. All Rights Reserved.

#include "SVirtualShapeDesignerDetailsPanel.h"

#include "DetailLayoutBuilder.h"
#include "VirtualShapeDesigner.h"
#include "VirtualShapeDesignerEditor.h"
#include "Classes/VirtualShape.h"

void SVirtualShapeDesignerDetailsPanel::Construct(const FArguments& InArgs, TSharedPtr<FVirtualShapeDesignerEditor> InVirtualShapeDesigner)
{
	VirtualShapeDesignerPtr = InVirtualShapeDesigner;
	SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(InVirtualShapeDesigner->GetToolkitCommands()), true, true);

	if (FSlateApplication::IsInitialized())
	{
		TArray<UObject*> SelectedObjects;
		SelectedObjects.Add(GetObjectToObserve());
		PropertyView->SetObjects(SelectedObjects, true);
	}
}

UObject* SVirtualShapeDesignerDetailsPanel::GetObjectToObserve() const
{
	return VirtualShapeDesignerPtr.Pin()->GetVirtualShape();
}

TSharedRef<SWidget> SVirtualShapeDesignerDetailsPanel::PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget)
{
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.FillHeight(1.0f)
	[
		PropertyEditorWidget
	];
}

void SVirtualShapeDesignerDetailsPanel::Refresh()
{
	if (FSlateApplication::IsInitialized())
	{
		PropertyView->ForceRefresh();
	}
}

FVirtualShapeDesignerDetailsPanelCustomization::FVirtualShapeDesignerDetailsPanelCustomization()
{
	DetailLayoutBuilder = nullptr;
}

TSharedRef<IDetailCustomization> FVirtualShapeDesignerDetailsPanelCustomization::MakeInstance()
{
	return MakeShareable(new FVirtualShapeDesignerDetailsPanelCustomization());
}

void FVirtualShapeDesignerDetailsPanelCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailLayoutBuilder = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayoutBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);
	TArray<UObject*> StrongObjects;
	CopyFromWeakArray(StrongObjects, ObjectsBeingCustomized);

	if (StrongObjects.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No object to modify"));
		return;
	}

	VirtualShapeEdited = Cast<UVirtualShape>(StrongObjects[0]);
	if (!VirtualShapeEdited.Get(false))
	{
		UE_LOG(LogTemp, Error, TEXT("Virtual Shape is null"));
		return;
	}

	FVirtualShapeDesignerModule* EditorModule = &FModuleManager::LoadModuleChecked<FVirtualShapeDesignerModule>("VirtualShapeDesigner");
	VirtualShapeDesignerEditor = EditorModule->GetShapeDesignerEditor();
}

void FVirtualShapeDesignerDetailsPanelCustomization::ForceRefresh() const
{
	DetailLayoutBuilder->ForceRefreshDetails();
}
