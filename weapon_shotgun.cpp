//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
#include "c_platoon_player.h"
#else
#include "platoon_player.h"
#include "ai_basenpc.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponShotgun C_WeaponShotgun
#endif

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

class CWeaponShotgun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponShotgun, CBaseHL2MPCombatWeapon);

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary);
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
#endif

private:
	CNetworkVar(float, m_flNextPump);
	CNetworkVar(float, m_flNextRoundFill);
	CNetworkVar(bool, m_bNeedFill);
	CNetworkVar(bool, m_bDelayedFire1);		// Fire primary when finished reloading
	CNetworkVar(bool, m_bDelayedFire2);		// Fire secondary when finished reloading
	CNetworkVar(bool, m_bDelayedReload);	// Reload when finished pump
	CNetworkVar(bool, m_bNeedPump);			// When emptied completely

public:
	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 1; }

	bool StartReload(void);
	bool Reload(void);
	void FillClip(void);
	void FinishReload(void);
	void CheckHolsterReload(void);
	void Pump(void);
	void ItemHolsterFrame(void);
	void ItemPostFrame(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void DryFire(void);
	virtual float GetFireRate(void) { return 0.7; };

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

	CWeaponShotgun(void);

private:
	CWeaponShotgun(const CWeaponShotgun &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponShotgun, DT_WeaponShotgun)

BEGIN_NETWORK_TABLE(CWeaponShotgun, DT_WeaponShotgun)
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bNeedPump)),
RecvPropBool(RECVINFO(m_bDelayedFire1)),
RecvPropBool(RECVINFO(m_bDelayedFire2)),
RecvPropBool(RECVINFO(m_bDelayedReload)),
#else
SendPropBool(SENDINFO(m_bNeedPump)),
SendPropBool(SENDINFO(m_bDelayedFire1)),
SendPropBool(SENDINFO(m_bDelayedFire2)),
SendPropBool(SENDINFO(m_bDelayedReload)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponShotgun)
DEFINE_PRED_FIELD(m_bNeedPump, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bDelayedFire1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bDelayedFire2, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_shotgun, CWeaponShotgun);
PRECACHE_WEAPON_REGISTER(weapon_shotgun);

#ifndef CLIENT_DLL
acttable_t	CWeaponShotgun::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeaponShotgun);

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponShotgun::CWeaponShotgun(void)
{
	m_bReloadsSingly = true;

	m_bNeedPump = false;
	m_bDelayedFire1 = false;
	m_bDelayedFire2 = false;

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 2048;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 2048;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponShotgun::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;

	Vector vecShootOrigin2;
	QAngle	angShootDir2;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);

	if (bUseWeaponAngles)
	{
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin2, angShootDir2);

	DispatchParticleEffect("muzzle_smgs", vecShootOrigin2, angShootDir2);
	DispatchParticleEffect("weapon_muzzle_smoke_long", PATTACH_POINT_FOLLOW, pOperator->GetActiveWeapon(), "muzzle", true);

	pOperator->FireBullets(sk_plr_num_shotgun_pellets.GetInt(), vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif


//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponShotgun::StartReload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

#ifndef CLIENT_DLL
	CHL2MP_Player *pPlayer = ToHL2MPPlayer(GetOwner());
	if (pPlayer)
	{
		if (pPlayer->IsSprinting() && (pPlayer->m_nButtons & IN_FORWARD || pPlayer->m_nButtons & IN_BACK || pPlayer->m_nButtons & IN_MOVELEFT || pPlayer->m_nButtons & IN_MOVERIGHT))
		{
			m_bWeaponIsLowered = false;
			pPlayer->StopSprinting();
		}
	}
#endif

	if (m_bNeedPump)
		return false;

	m_bInReload = true;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1, 0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponShotgun::Reload(void)
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		return false;
	}

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
	{
		return false;
	}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;
	
	m_bNeedFill = true;

	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim(ACT_VM_RELOAD);

	// Play the player's reload animation
	if (pOwner->IsPlayer())
	{
		((CBasePlayer *)pOwner)->SetAnimation(PLAYER_RELOAD);
	}

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextRoundFill = gpGlobals->curtime + SequenceDuration() - 0.07;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::FinishReload(void)
{
	// Make shotgun shell invisible
	SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::FillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1() && (m_flNextRoundFill <= gpGlobals->curtime))
		{
			m_iClip1++;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
			m_bNeedFill = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponShotgun::Pump(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bNeedPump = false;

	if (m_bDelayedReload)
	{
		m_bDelayedReload = false;
		StartReload();
	}

	WeaponSound(SPECIAL1);

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_PUMP);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::DryFire(void)
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		StartReload();
		return;
	}

	// MUST call sound before removing a round
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	DispatchParticleEffect("weapon_muzzle_smoke_long", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true);

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = m_flNextPump = gpGlobals->curtime + SequenceDuration();
	m_iClip1 -= 1;

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	FireBulletsInfo_t info(sk_plr_num_shotgun_pellets.GetInt(), vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accurate
	pPlayer->FireBullets(info);

#ifndef CLIENT_DLL
	// DM: Hellow? NPCs... look here! I'm shooting!
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2);
#endif

	QAngle punch;
	punch.Init(SharedRandomFloat("shotgunpax", -2, -1), SharedRandomFloat("shotgunpay", -2, 2), 0);
	pPlayer->ViewPunch(punch);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_bNeedPump = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponShotgun::SecondaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	pPlayer->m_nButtons &= ~IN_ATTACK2;
	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(WPN_DOUBLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(ACT_VM_SECONDARYATTACK);

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = m_flNextPump = gpGlobals->curtime + SequenceDuration();
	m_iClip1 -= 2;	// Shotgun uses same clip for primary and secondary attacks

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	FireBulletsInfo_t info(sk_plr_num_shotgun_pellets.GetInt() * 2, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuract
	pPlayer->FireBullets(info);
	pPlayer->ViewPunch(QAngle(SharedRandomFloat("shotgunsax", -5, 5), 0, 0));

#ifndef CLIENT_DLL
	// DM: Hellow? NPCs... look here! I'm shooting!
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2);
#endif

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_bNeedPump = true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponShotgun::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

#ifndef CLIENT_DLL
	CHL2MP_Player *pPlayer = ToHL2MPPlayer(GetOwner());
	if (!pPlayer)
		return;

	if (!m_bWeaponIsLowered && pPlayer->IsSprinting() && !(pPlayer->m_nButtons & IN_ATTACK) && (pPlayer->m_nButtons & IN_FORWARD || pPlayer->m_nButtons & IN_BACK || pPlayer->m_nButtons & IN_MOVELEFT || pPlayer->m_nButtons & IN_MOVERIGHT))
	{
		m_bWeaponIsLowered = true;
	}

	if (m_bWeaponIsLowered && !pPlayer->IsSprinting())
	{
		m_bWeaponIsLowered = false;
	}

	if (pPlayer->IsSprinting() && pPlayer->m_nButtons & IN_ATTACK)
	{
		m_bWeaponIsLowered = false;
		pPlayer->StopSprinting();
	}
#endif

	if (pOwner->GetPlayerMaxSpeed() > 160 && (pOwner->m_nButtons & IN_FORWARD || pOwner->m_nButtons & IN_BACK || pOwner->m_nButtons & IN_MOVELEFT || pOwner->m_nButtons & IN_MOVERIGHT))
	{
		if (m_bInReload)
		{
			m_bInReload = false;
			m_bDelayedReload = false;
			m_flNextPrimaryAttack = gpGlobals->curtime;
			return;
		}
		
		if ((m_bNeedPump) && (m_flNextPump <= gpGlobals->curtime))
		{
			m_flNextPump += 0.4;
			return;
		}
	}

	if (m_bNeedFill)
	{
		FillClip();
	}

	if (m_bNeedPump && (pOwner->m_nButtons & IN_RELOAD))
	{
		m_bDelayedReload = true;
	}

	if (m_bInReload)
	{
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_iClip1 >= 1) && !m_bNeedPump)
		{
			m_bInReload = false;
			m_bNeedPump = false;
			m_bDelayedFire1 = true;
		}
		// If I'm secondary firing and have two rounds stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_iClip1 >= 2) && !m_bNeedPump)
		{
			m_bInReload = false;
			m_bNeedPump = false;
			m_bDelayedFire2 = true;
		}
		else if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			else if (m_iClip1 < GetMaxClip1())
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}
	else
	{
		// Make shotgun shell invisible
		SetBodygroup(1, 1);
	}

	if ((m_bNeedPump) && (m_flNextPump <= gpGlobals->curtime))
	{
		Pump();
		return;
	}

	// Shotgun uses same timing and ammo for secondary attack
	if ((m_bDelayedFire2 || pOwner->m_nButtons & IN_ATTACK2) && (m_flNextPrimaryAttack <= gpGlobals->curtime) && !m_bNeedPump)
	{
		m_bDelayedFire2 = false;
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			DryFire();
			StartReload();
		}
		if ((m_iClip1 <= 1 && UsesClipsForAmmo1()))
		{
			// If only one shell is left, do a single shot instead	
			if (m_iClip1 == 1)
			{
				PrimaryAttack();
			}
		}

		// Fire underwater?
		else if (GetOwner()->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			if (pOwner->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			SecondaryAttack();
		}
	}
	else if ((m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime && !m_bNeedPump)
	{
		m_bDelayedFire1 = false;
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			DryFire();
			StartReload();
		}
		// Fire underwater?
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	
	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if (!m_bInReload)
		{
			WeaponIdle();
		}
	}

	if (!IsWeaponVisible() && GetOwner() && (GetOwner()->IsPlayer() == true) && (GetOwner()->GetActiveWeapon() == this))
	{
		SetWeaponVisible(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponShotgun::ItemHolsterFrame(void)
{
	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;
}