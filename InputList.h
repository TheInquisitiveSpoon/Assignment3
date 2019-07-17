#pragma once

#include <XInput.h>

// Button codes for XInput controller action polling
enum EButtonCode
{
	Button_A		= XINPUT_GAMEPAD_A,					// A Button
	Button_B		= XINPUT_GAMEPAD_B,					// B Button
	Button_X		= XINPUT_GAMEPAD_X,					// X Button
	Button_Y		= XINPUT_GAMEPAD_Y,					// Y Button

	Button_L		= XINPUT_GAMEPAD_LEFT_SHOULDER,		// Left Shoulder Button
	Button_R		= XINPUT_GAMEPAD_RIGHT_SHOULDER,	// Right Shoulder Button

	Button_Start	= XINPUT_GAMEPAD_START,				// Start Button
	Button_Back		= XINPUT_GAMEPAD_BACK,				// Back (Select) Button

	Button_LThumb	= XINPUT_GAMEPAD_LEFT_THUMB,		// Left Stick In
	Button_RThumb	= XINPUT_GAMEPAD_RIGHT_THUMB,		// Right Stick In

	DPad_Up			= XINPUT_GAMEPAD_DPAD_UP,			// D-Pad Up
	DPad_Down		= XINPUT_GAMEPAD_DPAD_DOWN,			// D-Pad Down
	DPad_Left		= XINPUT_GAMEPAD_DPAD_LEFT,			// D-Pad Left
	DPad_Right		= XINPUT_GAMEPAD_DPAD_RIGHT,		// D-Pad Right

	BUTTON_CODE_AMOUNT	// Total amount of button codes
};

// Types of analogue input
enum EAnalogueCode
{
	Analogue_LStickX,		// Left stick horizontal analogue
	Analogue_LStickY,		// Left stick vertical analogue

	Analogue_RStickX,		// Right stick horizontal analogue
	Analogue_RStickY,		// Right stick vertical analogue

	Analogue_LTrigger,		// Left trigger analogue
	Analogue_RTrigger,		// Right trigger analogue

	ANALOGUE_CODE_AMOUNT	// Total amount of analogue codes
};