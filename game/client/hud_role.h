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

class CHudRole : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudRole, vgui::Panel);

public:
	CHudRole(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "154 174 144 250");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Flags");
	CPanelAnimationVar(vgui::HFont, m_hTextFont2, "TextFont2", "Default");
	CPanelAnimationVarAliasType(float, flag_xpos, "flag_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, flag_ypos, "flag_ypos", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "8", "proportional_float");

	float m_flHull;
};

#endif