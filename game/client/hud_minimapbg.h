//========= Created by Heath ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// hud_role.h
//
//
//
#if !defined HUD_HULL_H
#define HUD_HULL_H

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"

//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------

class CMinimapBackground : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CMinimapBackground, vgui::Panel);

public:
	CMinimapBackground(const char * pElementName);
};

#endif