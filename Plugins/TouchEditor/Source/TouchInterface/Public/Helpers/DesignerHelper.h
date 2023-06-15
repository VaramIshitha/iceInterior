// Copyright Lost in Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "DesignerHelper.generated.h"

class SDesignHelper;
class UVirtualControlSetup;

/**
 * 
 */
UCLASS()
class TOUCHINTERFACE_API UDesignerHelper : public UWidget
{
	GENERATED_BODY()

public:
	UDesignerHelper();
	
	//Begin UWidget
	virtual void SynchronizeProperties() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual void OnBeginEditByDesigner() override;
	virtual void OnEndEditByDesigner() override;
	virtual void OnDesignerChanged(const FDesignerChangedEventArgs& EventArgs) override;
#endif
	//End UWidget

	//Begin UVisual
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//End UVisual

public:
	UPROPERTY(Category="Designer Helper", EditAnywhere)
	UVirtualControlSetup* PreviewSetup;
	
	UPROPERTY(Category="Designer Helper", EditAnywhere, BlueprintReadWrite, meta=(UIMin=0.0f, ClampMin=0.0f, UIMax=1.0f, ClampMax=1.0f))
	float Opacity;

	UPROPERTY(Category="Designer Helper", EditAnywhere, BlueprintReadWrite)
	bool bShowInteractionZone;
	
protected:
	TSharedPtr<SDesignHelper> MyDesignHelper;

private:
	FIntPoint ScreenSize;
	float CurrentDPIScale;
};
