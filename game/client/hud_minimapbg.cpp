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
#include "hud_minimapbg.h"
#include "iclientmode.h"
#include "vgui/ISurface.h"
#include <vgui/ILocalize.h>

using namespace vgui;

#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CMinimapBackground);

# define HULL_INIT 0

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CMinimapBackground::CMinimapBackground(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "MinimapBackground")
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
}