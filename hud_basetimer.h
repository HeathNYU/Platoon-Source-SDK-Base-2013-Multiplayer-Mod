//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HUD_BASETIMER_H
#define HUD_BASETIMER_H
#ifdef _WIN32
#pragma once
#endif

#include "hud_numericdisplay.h"

//-----------------------------------------------------------------------------
// Purpose: Base class for all the hud elements that are just a numeric display
//			with some options for text and icons
//-----------------------------------------------------------------------------
class CHudBaseTimer : public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudBaseTimer, CHudNumericDisplay );

public:
	CHudBaseTimer(vgui::Panel *parent, const char *name);

	void SetMinutes( int minutes );
	void SetSeconds( int seconds );

protected:
	// vgui overrides
	virtual void Paint();

	void SetToPrimaryColor();
	void SetToSecondaryColor();

private:
	void PaintTime(vgui::HFont font, int xpos, int ypos, int mins, int secs);

	int m_iMinutes;
	int m_iSeconds;
	wchar_t m_LabelText[32];

	CPanelAnimationVar( float, m_flAlphaOverride, "Alpha", "255" );
	CPanelAnimationVar( Color, m_FlashColor, "SecondaryColor", "FgColor" );
};


#endif // HUD_BASETIMER_H
