// Copyright Lost in Game Studio, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TouchInterfaceDesignerSettings.generated.h"

/**
 * Settings for Touch Interface Designer Editor
 */
UCLASS(Config=Engine, DefaultConfig, meta=(DisplayName="Touch Interface Designer"))
class TOUCHINTERFACEDESIGNER_API UTouchInterfaceDesignerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Constructor
	UTouchInterfaceDesignerSettings();

	/** Default visual art for button */
	UPROPERTY(Config, EditAnywhere, Category="Visual", meta=(AllowedClasses="/Script/Engine.Texture2D"))
	FSoftObjectPath DefaultButtonImage;

	/** Default visual art for joystick background */
	UPROPERTY(Config, EditAnywhere, Category="Visual", meta=(AllowedClasses="/Script/Engine.Texture2D"))
	FSoftObjectPath DefaultBackgroundJoystickImage;

	/** Default visual art for joystick thumb */
	UPROPERTY(Config, EditAnywhere, Category="Visual", meta=(AllowedClasses="/Script/Engine.Texture2D"))
	FSoftObjectPath DefaultThumbJoystickImage;

	/** Max layer to draw for one virtual control */
	UPROPERTY(Config, EditAnywhere, Category="Visual")
	uint32 MaxLayer;
	

	/** Keep offset between Widget and Cursor when widget is clicked for moving. If false, widget is snapped to cursor position */
	UPROPERTY(Config, EditAnywhere, Category="Editor")
	uint8 bKeepCursorOffset:1;

	/** Translate canvas to keep same percent from cursor position when zoom in/out */
	UPROPERTY(Config, EditAnywhere, Category="Editor")
	uint8 bZoomToPointerPosition:1;

	UPROPERTY(Config, EditAnywhere, Category="Editor")
	uint8 bShowDashedOutlineByDefault:1;

	UPROPERTY(Config, EditAnywhere, Category="Color")
	FLinearColor InteractionVisualizationColor;

	/** Enable Virtual Shape Designer
	 * Warning! This is a first implementation and it's very experimental
	 * The editor is not finished and you may encounter bugs and slowdowns
	 * BACKUP YOUR PROJECT FIRST BEFORE ACTIVATING THIS!!!
	 */
	UPROPERTY(Category="Virtual Shape Editor", Config, EditAnywhere, meta=(ConfigRestartRequired="true"))
	uint8 bEnableVirtualShapeEditor:1;
	
	//Line color used in virtual shape editor 
	UPROPERTY(Config, EditAnywhere, Category="Shape Editor")
	FLinearColor ShapeLineColor;

	//Dot color used in virtual shape editor
	UPROPERTY(Config, EditAnywhere, Category="Shape Editor")
	FLinearColor ShapeDotColor;

	//Line color used in virtual shape editor
	UPROPERTY(Config, EditAnywhere, Category="Shape Editor", meta=(ClampMin=1.0f, ClampMax=10.0f))
	float ShapeLineSize;

	//Dot color used in virtual shape editor
	UPROPERTY(Config, EditAnywhere, Category="Shape Editor", meta=(ClampMin=4.0f, ClampMax=10.0f))
	float ShapeDotSize;
};