//========= Created by Heath ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Timer.cpp
//
// implementation of CHudSpectatorTimer class
//
#include "cbase.h"
#include "platoon_gamerules.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_basespectatortimer.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Timer panel
//-----------------------------------------------------------------------------
class CHudSpectatorTimer : public CHudElement, public CHudBaseSpectatorTimer
{
	DECLARE_CLASS_SIMPLE( CHudSpectatorTimer, CHudBaseSpectatorTimer );

public:
	CHudSpectatorTimer(const char *pElementName);
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

private:
	float m_fStart; // The last transmitted start-time, to check, if the timer has restarted
	int m_iRemain; // The remaining time at the last visual update, to optimize updates
	bool redText;
	int iRemain;
};

DECLARE_HUDELEMENT( CHudSpectatorTimer );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSpectatorTimer::CHudSpectatorTimer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudSpectatorTimer")
{
	SetPaintEnabled(false);
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudSpectatorTimer::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudSpectatorTimer::Reset()
{
	redText = false;

	// start the init-event to reset the changing properties
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerInit");

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_TIMER");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"TIME REMAINING");
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudSpectatorTimer::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudSpectatorTimer::OnThink()
{
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	if (local->IsPlayerDead())
	{
		SetPaintBackgroundEnabled(true);
		SetPaintEnabled(true);
	}
	else
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
	}
	
	CHL2MPRules *pRules = HL2MPRules();
	if (!pRules)
	{
		return;
	}

	// There was no timer before or the timer has been restarted.
	if (m_fStart != pRules->m_fStart)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerInit");
		// save starttime to detect a timer-restart
		m_fStart = pRules->m_fStart;
	}

	iRemain = pRules->GetRoundtimerRemain();

	// There was no timer and still is no timer or there's no new time to display.
	if (iRemain < 0 && m_iRemain < 0)
	{
		SetMinutes(0);
		SetSeconds(0);
		return;
	}

	if (iRemain == m_iRemain)
	{
		return;
	}

	// If we're here, there's was a timer before, but if's it's not there anymore, we need to hide it.
	if (iRemain < 0) {
		//SetPaintEnabled(false);
		//SetPaintBackgroundEnabled(false);
		m_iRemain = -1;
		return;
	}

	m_iRemain = iRemain;
	SetMinutes(m_iRemain / 60);
	SetSeconds(m_iRemain % 60);

	// When the timer reaches 30, change the color to red.
	if (m_iRemain == 31) {
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerBelow31");
		redText = true;
	}
	else {
		if (m_iRemain < 31 && !redText)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerBelow31");
			redText = true;

			// For every time below 15 make it pulse.
			if (iRemain <= 15)
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerPulse");
		}
		else if (m_iRemain > 31 && redText)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SpectatorTimerInit");
	}
}