//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "ammodef.h"
#include "platoon_gamerules.h"


#ifdef CLIENT_DLL
extern IVModelInfoClient* modelinfo;
#else
extern IVModelInfo* modelinfo;
#endif


#if defined( CLIENT_DLL )

	#include "vgui/ISurface.h"
	#include "vgui_controls/Controls.h"
	#include "c_platoon_player.h"
	#include "hud_crosshair.h"

#else

	#include "platoon_player.h"
	#include "vphysics/constraints.h"
	#include "globalstate.h"

#endif

#include "weapon_hl2mpbase.h"


// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //

bool IsAmmoType( int iAmmoType, const char *pAmmoName )
{
	return GetAmmoDef()->Index( pAmmoName ) == iAmmoType;
}

static const char * s_WeaponAliasInfo[] = 
{
	"none",	//	WEAPON_NONE = 0,

	//Melee
	"shotgun",	//WEAPON_AMERKNIFE,
	
	NULL,		// end of list marker
};


// ----------------------------------------------------------------------------- //
// CWeaponHL2MPBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHL2MPBase, DT_WeaponHL2MPBase )

BEGIN_NETWORK_TABLE( CWeaponHL2MPBase, DT_WeaponHL2MPBase )

#ifdef CLIENT_DLL

#else
	// world weapon models have no aminations
	//	SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
	//	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	//	SendPropExclude( "DT_LocalActiveWeaponData", "m_flTimeWeaponIdle" ),
#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponHL2MPBase )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_hl2mp_base, CWeaponHL2MPBase )


#ifdef GAME_DLL

	BEGIN_DATADESC( CWeaponHL2MPBase )

	END_DATADESC();

#endif

// ----------------------------------------------------------------------------- //
// CWeaponHL2MPBase implementation.
// ----------------------------------------------------------------------------- //
CWeaponHL2MPBase::CWeaponHL2MPBase()
{
	SetPredictionEligible( true );
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_flNextResetCheckTime = 0.0f;
}


bool CWeaponHL2MPBase::IsPredicted() const
{
	return true;
}

void CWeaponHL2MPBase::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
#ifdef CLIENT_DLL

		// If we have some sounds from the weapon classname.txt file, play a random one of them
		const char *shootsound = GetWpnData().aShootSounds[ sound_type ]; 
		if ( !shootsound || !shootsound[0] )
			return;

		CBroadcastRecipientFilter filter; // this is client side only
		if ( !te->CanPredict() )
			return;
				
		CBaseEntity::EmitSound( filter, GetPlayerOwner()->entindex(), shootsound, &GetPlayerOwner()->GetAbsOrigin() ); 
#else
		BaseClass::WeaponSound( sound_type, soundtime );
#endif
}


CBasePlayer* CWeaponHL2MPBase::GetPlayerOwner() const
{
	return dynamic_cast< CBasePlayer* >( GetOwner() );
}

CHL2MP_Player* CWeaponHL2MPBase::GetHL2MPPlayerOwner() const
{
	return dynamic_cast< CHL2MP_Player* >( GetOwner() );
}

#ifdef CLIENT_DLL
	
void CWeaponHL2MPBase::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( GetPredictable() && !ShouldPredict() )
		ShutdownPredictable();
}


bool CWeaponHL2MPBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer() )
		return true;

	return BaseClass::ShouldPredict();
}


#else
	
void CWeaponHL2MPBase::Spawn()
{
	BaseClass::Spawn();

	// Set this here to allow players to shoot dropped weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
}

void CWeaponHL2MPBase::Materialize( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound( "AlyxEmp.Charge" );
		
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );
		SetMoveType( MOVETYPE_VPHYSICS );

		HL2MPRules()->AddLevelDesignerPlacedObject( this );
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		if ( GetOriginalSpawnOrigin() == vec3_origin )
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}

	SetPickupTouch();

	SetThink (NULL);
}

int CWeaponHL2MPBase::ObjectCaps()
{
	return BaseClass::ObjectCaps();
}

#endif

void CWeaponHL2MPBase::FallInit( void )
{
#ifndef CLIENT_DLL
	SetModel( GetWorldModel() );
	VPhysicsDestroyObject();

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		SetMoveType( MOVETYPE_NONE );
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_TRIGGER );

		UTIL_DropToFloor( this, MASK_SOLID );
	}
	else
	{
		if ( !VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false ) )
		{
			SetMoveType( MOVETYPE_NONE );
			SetSolid( SOLID_BBOX );
			AddSolidFlags( FSOLID_TRIGGER );
		}
		else
		{
	#if !defined( CLIENT_DLL )
			// Constrained start?
			if ( HasSpawnFlags( SF_WEAPON_START_CONSTRAINED ) )
			{
				//Constrain the weapon in place
				IPhysicsObject *pReferenceObject, *pAttachedObject;
				
				pReferenceObject = g_PhysWorldObject;
				pAttachedObject = VPhysicsGetObject();

				if ( pReferenceObject && pAttachedObject )
				{
					constraint_fixedparams_t fixed;
					fixed.Defaults();
					fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );
					
					fixed.constraint.forceLimit	= lbs2kg( 10000 );
					fixed.constraint.torqueLimit = lbs2kg( 10000 );

					IPhysicsConstraint *pConstraint = GetConstraint();

					pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

					pConstraint->SetGameData( (void *) this );
				}
			}
	#endif //CLIENT_DLL
		}
	}

	SetPickupTouch();
	
	SetThink( &CWeaponHL2MPBase::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );

#endif
}

#ifdef GAME_DLL
void CWeaponHL2MPBase::FallThink(void)
{
	// Prevent the common HL2DM weapon respawn bug from happening
	// When a weapon is spawned, the following chain of events occurs:
	// - Spawn() is called (duh), which then calls FallInit()
	// - FallInit() is called, and prepares the weapon's 'Think' function (CBaseCombatWeapon::FallThink())
	// - FallThink() is called, and performs several checks before deciding whether the weapon should Materialize()
	// - Materialize() is called (the HL2DM version above), which sets the weapon's respawn location.
	// The problem occurs when a weapon isn't placed properly by a level designer.
	// If the weapon is unable to move from its location (e.g. if its bounding box is halfway inside a wall), Materialize() never gets called.
	// Since Materialize() never gets called, the weapon's respawn location is never set, so if a person picks it up, it respawns forever at
	// 0 0 0 on the map (infinite loop of fall, wait, respawn, not nice at all for performance and bandwidth!)
	if (HasSpawnFlags(SF_NORESPAWN) == false)
	{
		if (GetOriginalSpawnOrigin() == vec3_origin)
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}
	return BaseClass::FallThink();
}
#endif // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();

	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;
}

//-----------------------------------------------------------------------------
// Purpose: Drops the weapon into a lowered pose
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::Lower(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Brings the weapon up to the ready position
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::Ready(void)
{
	//Don't bother if we don't have the animation
	if (SelectWeightedSequence(ACT_VM_LOWERED_TO_IDLE) == ACTIVITY_NOT_AVAILABLE)
		return false;

	m_bLowered = false;
	m_flRaiseTime = gpGlobals->curtime + 0.5f;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::Deploy(void)
{
	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		CHL2MP_Player *pPlayer = assert_cast<CHL2MP_Player*>(GetOwner());
		if (pPlayer->IsWeaponLowered())
		{
			if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) != ACTIVITY_NOT_AVAILABLE)
			{
				if (DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix()))
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack(gpGlobals->curtime + 1.0);
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}

	m_bLowered = false;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (BaseClass::Holster(pSwitchingTo))
	{
		SetWeaponVisible(false);
		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::WeaponShouldBeLowered(void)
{
	// Can't be in the middle of another animation
	if (GetIdealActivity() != ACT_VM_IDLE_LOWERED && GetIdealActivity() != ACT_VM_IDLE &&
		GetIdealActivity() != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity() != ACT_VM_LOWERED_TO_IDLE)
		return false;

	if (m_bLowered)
		return true;

#if !defined( CLIENT_DLL )

	if (GlobalEntity_GetState("friendly_encounter") == GLOBAL_ON)
		return true;
#endif

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Allows the weapon to choose proper weapon idle animation
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::WeaponIdle(void)
{
	//See if we should idle high or low
	if (WeaponShouldBeLowered())
	{
		// Move to lowered position if we're not there yet
		if (GetActivity() != ACT_VM_IDLE_LOWERED && GetActivity() != ACT_VM_IDLE_TO_LOWERED
			&& GetActivity() != ACT_TRANSITION)
		{
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			// Keep idling low
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
	}
	else
	{
		// See if we need to raise immediately
		if (m_flRaiseTime < gpGlobals->curtime && GetActivity() == ACT_VM_IDLE_LOWERED && !m_bWeaponIsLowered)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
		else if (HasWeaponIdleTimeElapsed() && !m_bWeaponIsLowered)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
}

const CHL2MPSWeaponInfo &CWeaponHL2MPBase::GetHL2MPWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CHL2MPSWeaponInfo *pHL2MPInfo;

	#ifdef _DEBUG
		pHL2MPInfo = dynamic_cast< const CHL2MPSWeaponInfo* >( pWeaponInfo );
		Assert( pHL2MPInfo );
	#else
		pHL2MPInfo = static_cast< const CHL2MPSWeaponInfo* >( pWeaponInfo );
	#endif

	return *pHL2MPInfo;
}
void CWeaponHL2MPBase::FireBullets( const FireBulletsInfo_t &info )
{
	FireBulletsInfo_t modinfo = info;

	modinfo.m_iPlayerDamage = GetHL2MPWpnData().m_iPlayerDamage;

	BaseClass::FireBullets( modinfo );
}


#if defined( CLIENT_DLL )

#include "c_te_effect_dispatch.h"

#define NUM_MUZZLE_FLASH_TYPES 4

bool CWeaponHL2MPBase::OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options )
{
	return BaseClass::OnFireEvent( pViewModel, origin, angles, event, options );
}


void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle	final = in + punch;

	//Clip each component
	for ( int i = 0; i < 3; i++ )
	{
		if ( final[i] > clip[i] )
		{
			final[i] = clip[i];
		}
		else if ( final[i] < -clip[i] )
		{
			final[i] = -clip[i];
		}

		//Return the result
		in[i] = final[i] - punch[i];
	}
}

#endif