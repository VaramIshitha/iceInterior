// Copyright Lost in Game Studio. All Rights Reserved.


#include "Helpers/DesignerHelper.h"
#include "Helpers/SDesignHelper.h"

#define LOCTEXT_NAMESPACE "UMG"

UDesignerHelper::UDesignerHelper()
{
	Opacity = 1.0f;
	bShowInteractionZone = false;
}

void UDesignerHelper::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	MyDesignHelper->SetSetupAsset(PreviewSetup);
	MyDesignHelper->SetOpacity(Opacity);
	MyDesignHelper->SetDrawInteractionZone(bShowInteractionZone);
}

TSharedRef<SWidget> UDesignerHelper::RebuildWidget()
{
	MyDesignHelper = SNew(SDesignHelper)
	.Setup(PreviewSetup)
	.StartOpacity(Opacity)
	.ScreenSize(ScreenSize)
	.DPIScale(CurrentDPIScale)
	.DrawInteractionZone(bShowInteractionZone);
	
	return MyDesignHelper.ToSharedRef();
}

#if WITH_EDITOR
const FText UDesignerHelper::GetPaletteCategory()
{
	return LOCTEXT("VirtualControlDesigner", "Virtual Control Designer");
}

void UDesignerHelper::OnBeginEditByDesigner()
{
	Super::OnBeginEditByDesigner();
	UE_LOG(LogTemp, Display, TEXT("On Begin Edit by Designer"));
	Opacity = 1.0f;
	ScreenSize = FIntPoint(1280, 720);
	CurrentDPIScale = 1.0f;
}

void UDesignerHelper::OnEndEditByDesigner()
{
	Super::OnEndEditByDesigner();
	UE_LOG(LogTemp, Display, TEXT("On End Edit by Designer"));
}

void UDesignerHelper::OnDesignerChanged(const FDesignerChangedEventArgs& EventArgs)
{
	Super::OnDesignerChanged(EventArgs);
	UE_LOG(LogTemp, Display, TEXT("On Designer Changed"));
	
	ScreenSize = FIntPoint(EventArgs.Size.X, EventArgs.Size.Y);
	CurrentDPIScale = EventArgs.DpiScale;

	//MyDesignHelper->SetSetupAsset(PreviewSetup);
	MyDesignHelper->SetOpacity(Opacity);
	MyDesignHelper->SetScreenSize(ScreenSize);
	MyDesignHelper->SetDpiScale(CurrentDPIScale);
	MyDesignHelper->SetDrawInteractionZone(bShowInteractionZone);
}
#endif

void UDesignerHelper::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyDesignHelper.Reset();
}

#undef LOCTEXT_NAMESPACE