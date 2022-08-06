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
#include "hud_role.h"
#include "iclientmode.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

using namespace vgui;

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudRole);

# define HULL_INIT 0

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudRole::CHudRole(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HudRole")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudRole::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudRole::Reset(void)
{
	m_flHull = HULL_INIT;
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudRole::OnThink(void)
{
	float newHull = 0;

	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	// Never below zero 
	newHull = max(local->GetHealth(), 0);

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHull == m_flHull)
		return;

	m_flHull = newHull;
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudRole::Paint()
{
	// Draw our name

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_TextColor);
	surface()->DrawSetTextPos(flag_xpos, flag_ypos);

	surface()->DrawPrintText(L"u", wcslen(L"u"));
	
	surface()->DrawSetTextFont(m_hTextFont2);
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	//wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_AUX_POWER");

	surface()->DrawPrintText(L"USMC - FIRST LIEUTENANT", wcslen(L"USMC - FIRST LIEUTENANT"));
}