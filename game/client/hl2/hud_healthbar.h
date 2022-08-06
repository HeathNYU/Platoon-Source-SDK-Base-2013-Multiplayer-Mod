//========= Created by Heath ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// hud_healthbar.h
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

class CHudHealthbar : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthbar, vgui::Panel);

public:
	CHudHealthbar(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

	void MsgFunc_Battery(bf_read &msg);

protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "148 151 165 250");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "40");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "5", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "142", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");

	CPanelAnimationVar(Color, m_HullColor2, "HullColor2", "154 174 144 250");
	CPanelAnimationVar(int, m_iHullDisabledAlpha2, "HullDisabledAlpha2", "90");
	CPanelAnimationVarAliasType(float, m_flBarInsetX2, "BarInsetX2", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY2, "BarInsetY2", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth2, "BarWidth2", "142", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight2, "BarHeight2", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth2, "BarChunkWidth2", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap2, "BarChunkGap2", "0", "proportional_float");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "154 174 144 250");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "55", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "19", "proportional_float");

	int		m_iBat;
	int		m_iNewBat;

	float m_flHull;

	float newHull;

	int m_nHullLow;
};

#endif