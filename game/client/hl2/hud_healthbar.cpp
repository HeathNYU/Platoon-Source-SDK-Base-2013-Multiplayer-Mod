//========= Created by Heath ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Healthbar.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "c_baseplayer.h"
#include "c_basehlplayer.h"
#include "hud_healthbar.h"
#include "iclientmode.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

using namespace vgui;

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudHealthbar);
DECLARE_HUD_MESSAGE(CHudHealthbar, Battery);

#define HULL_INIT 80 

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudHealthbar::CHudHealthbar(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HudHealthbar")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthbar::Init()
{
	HOOK_HUD_MESSAGE(CHudHealthbar, Battery);
	m_iBat = 0;
	m_iNewBat = 0;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudHealthbar::Reset(void)
{
	m_iBat = HULL_INIT;
	m_flHull = HULL_INIT;
	m_nHullLow = -1;
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudHealthbar::OnThink(void)
{
	newHull = 0;

	C_BaseHLPlayer *local = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	// Never below zero
	newHull = max(local->GetHealth(), 0);

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (m_iNewBat == m_iBat && newHull == m_flHull)
		return;

	m_iBat = m_iNewBat;
	m_flHull = newHull;
}


//------------------------------------------------------------------------
// Purpose: draws the health bar
//------------------------------------------------------------------------

void CHudHealthbar::Paint()
{
	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_iBat / 100.0f) + 0.5f);
	
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX + m_flBarWidth, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos -= (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos -= (m_flBarChunkWidth + m_flBarChunkGap);
	}
	
	// Get bar chunks

	int chunkCount2 = m_flBarWidth2 / (m_flBarChunkWidth2 + m_flBarChunkGap2);
	int enabledChunks2 = (int)((float)chunkCount2 * (m_flHull / 100.0f) + 0.5f);

	surface()->DrawSetColor(m_HullColor2);

	int xpos2 = m_flBarInsetX2 + m_flBarWidth2, ypos2 = m_flBarInsetY2;

	for (int i = 0; i < enabledChunks2; i++)
	{
		surface()->DrawFilledRect(xpos2, ypos2, xpos2 + m_flBarChunkWidth2, ypos2 + m_flBarHeight2);
		xpos2 -= (m_flBarChunkWidth2 + m_flBarChunkGap2);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor2[0], m_HullColor2[1], m_HullColor2[2], m_iHullDisabledAlpha2));

	for (int i = enabledChunks2; i < chunkCount2; i++)
	{
		surface()->DrawFilledRect(xpos2, ypos2, xpos2 + m_flBarChunkWidth2, ypos2 + m_flBarHeight2);
		xpos2 -= (m_flBarChunkWidth2 + m_flBarChunkGap2);
	}

	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_TextColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	//wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_AUX_POWER");

	surface()->DrawPrintText(L"FLAK JACKET & HEALTH", wcslen(L"FLAK JACKET & HEALTH"));
}

void CHudHealthbar::MsgFunc_Battery(bf_read &msg)
{
	m_iNewBat = msg.ReadShort();
}