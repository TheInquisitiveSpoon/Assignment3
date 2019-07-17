#pragma once

#include <Windows.h>
#include <XInput.h>

#include "inputlist.h"

/* ------------------------------------------------------------------

Usage Comments:
-- Before using this class, you'll need to add the XInput library
   to your linker input settings
   -- Right click your project in Solution Explorer
   -- Select Properties at the bottom of the list
   -- Expand the Linker tab
   -- Select Input
   -- Click the drop-down icon for Additional Dependencies
   -- Click <Edit...>
   -- Paste "Xinput9_1_0.lib" (without quotes) into the box
   -- Click OK and close the properties window

-- If the program still won't compile:
   -- Download the DirectX SDK (DXSDK_Jun10) here: 
	  https://www.microsoft.com/en-gb/download/details.aspx?id=6812
   -- Follow the instructions here: 
	  http://gamesnorthwest.net/resources/modules/co2409/materials/Week%205/D3D%20Folders%20in%20Visual%20Studio.txt

-- To use:
	-- Create an XController
	-- Call UpdateInput() at the beginning of each frame
	-- You can then call ButtonHit, ButtonHeld, and GetAnalogueInput
       for any input polling at any point in the frame as you would
       with TL-Engine

------------------------------------------------------------------ */

class XController
{

// -------------------------------------------------------------------------------------------------
// Data
// ----

private:
	// XInput State in its raw Windows form
	// -- Contains button inputs from last poll and 
	//    analogue values in range of -32768 to 32767
	XINPUT_STATE m_RawState;

	// Buttons pressed last frame
	// -- Used to check if a button was hit this frame
	WORD m_LastFrameButtons;

	// Analogue values converted to range -1.0f to 1.0f
	float m_LStickX;	float m_LStickY;
	float m_RStickX;	float m_RStickY;
	float m_LTrigger;	float m_RTrigger; 

	// Shows whether the state could successfully be read
	bool m_IsConnected;

	// Which virtual port the controller is bound to
	// -- Range of 0 to XUSER_MAX_COUNT (4)
	int m_Port;

// -------------------------------------------------------------------------------------------------
// Construction
// ------------

public:
	XController(int playerNum = 1)
	{
		// Initialise member variables to default values
		// -- Simple boundary checking on playerNum to account
		//    for users inputting 0 or 1 for player one
		// -- No upper boundary checks
		//    -- It won't crash, it just won't work with anything
		//       above playerNum = 4
		m_Port = playerNum - 1;
		if (m_Port < 0) m_Port = 0;

		m_LastFrameButtons = 0;
		m_LStickX = 0.0f;
		m_LStickY = 0.0f;
		m_RStickX = 0.0f;
		m_RStickY = 0.0f;
		m_LTrigger = 0.0f;
		m_RTrigger = 0.0f;

		ZeroMemory(&m_RawState, sizeof(XINPUT_STATE));

		// Check if a controller is connected at current virtual port
		// -- If controller is connected, mark it as such
		if (XInputGetState(playerNum, &m_RawState) == ERROR_SUCCESS)
		{
			m_IsConnected = true;
		}
		
		else m_IsConnected = false;
	}

	~XController() {}

	// Prevent copy/move/assignment construction
	XController(const XController&)				= delete;
	XController(XController&&)					= delete;
	XController& operator=(const XController&)	= delete;
	XController& operator=(XController&&)		= delete;

// -------------------------------------------------------------------------------------------------
// Usage
// -----

public:
	bool IsConnected()
	{
		return m_IsConnected;
	}

	int GetPlayerNum()
	{
		return m_Port + 1;
	}

	bool UpdateInput()
	{
		// Store button values before overwriting the raw state
		m_LastFrameButtons = m_RawState.Gamepad.wButtons;

		// If getting state succeeded, process inputs
		ZeroMemory(&m_RawState, sizeof(XINPUT_STATE));
		if (XInputGetState(m_Port, &m_RawState) == ERROR_SUCCESS)
		{
			// Convert analogue values to range -1.0f to 1.0f
			// -- Check negativity for correct division value
			// -- Negative boundary is slightly higher, causing slight differences if using same value to divide
			m_LStickX = static_cast<float>(m_RawState.Gamepad.sThumbLX) / ((m_RawState.Gamepad.sThumbLX > 0) ? 32767.0f : 32768.0f);
			m_LStickY = static_cast<float>(m_RawState.Gamepad.sThumbLY) / ((m_RawState.Gamepad.sThumbLY > 0) ? 32767.0f : 32768.0f);

			m_RStickX = static_cast<float>(m_RawState.Gamepad.sThumbRX) / ((m_RawState.Gamepad.sThumbRX > 0) ? 32767.0f : 32768.0f);
			m_RStickY = static_cast<float>(m_RawState.Gamepad.sThumbRY) / ((m_RawState.Gamepad.sThumbRY > 0) ? 32767.0f : 32768.0f);

			m_LTrigger = static_cast<float>(m_RawState.Gamepad.bLeftTrigger)  / 255.0f;
			m_RTrigger = static_cast<float>(m_RawState.Gamepad.bRightTrigger) / 255.0f;

			return m_IsConnected = true;
		}

		return m_IsConnected = false;
	}

	// Returns true if button was hit this frame
	bool ButtonHit(EButtonCode button)
	{
		// If controller isn't connected, don't try to get new information
		if (!m_IsConnected) return false;

		// If button is held this frame and was not held last frame
		if ( (m_RawState.Gamepad.wButtons & button) != 0 && 
			!(m_LastFrameButtons & button) != 0 ) return true;

		return false;
	}

	// Returns true if button is currently being held
	bool ButtonHeld(EButtonCode button)
	{
		// If controller isn't connected, don't try to get new information
		if (!m_IsConnected) return false;

		// Last frame check not required to see if button is held
		return (m_RawState.Gamepad.wButtons & button) != 0;
	}

	// Returns analogue input from desired source in the range of -1.0f to 1.0f
	float GetAnalogueInput(EAnalogueCode analogueType)
	{
		// Return correct value based on requested input type
		// -- Default case to account for anybody trying to
		//    use ANALOGUE_CODE_AMOUNT for whatever reason
		switch (analogueType)
		{
			case Analogue_LStickX:	return m_LStickX;
			case Analogue_LStickY:	return m_LStickY;
			case Analogue_RStickX:	return m_RStickX;
			case Analogue_RStickY:	return m_RStickY;
			case Analogue_LTrigger:	return m_LTrigger;
			case Analogue_RTrigger:	return m_RTrigger;
			default: return 0.0f;
		}
	}

// -------------------------------------------------------------------------------------------------

};