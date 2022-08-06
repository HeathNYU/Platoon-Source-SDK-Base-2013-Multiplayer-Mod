//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_HL2MPBASE_H
#define WEAPON_HL2MPBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "platoon_player_shared.h"
#include "basecombatweapon_shared.h"
#include "hl2mp_weapon_parse.h"
#ifndef CLIENT_DLL
	#include "AI_BaseNPC.h"
#endif

#if defined( CLIENT_DLL )
	#define CWeaponHL2MPBase C_WeaponHL2MPBase
	void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip );
#endif

class CHL2MP_Player;

// These are the names of the ammo types that go in the CAmmoDefs and that the 
// weapon script files reference.

// Given an ammo type (like from a weapon's GetPrimaryAmmoType()), this compares it
// against the ammo name you specify.
// MIKETODO: this should use indexing instead of searching and strcmp()'ing all the time.
bool IsAmmoType( int iAmmoType, const char *pAmmoName );

class CWeaponHL2MPBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponHL2MPBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponHL2MPBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	
		void SendReloadSoundEvent( void );

		void Materialize( void );
		virtual	int	ObjectCaps( void );
		virtual void FallThink( void );	
	#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const;

	CBasePlayer* GetPlayerOwner() const;
	CHL2MP_Player* GetHL2MPPlayerOwner() const;

	void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );
	
	CHL2MPSWeaponInfo const	&GetHL2MPWpnData() const;

	virtual bool	Ready(void);
	virtual bool	Lower(void);
	virtual bool	Deploy(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool	WeaponShouldBeLowered(void);
	virtual void	WeaponIdle(void);

	virtual void	ItemHolsterFrame(void);

	virtual void	FireBullets( const FireBulletsInfo_t &info );
	virtual void	FallInit( void );
	
public:
	#if defined( CLIENT_DLL )
		
		virtual bool	ShouldPredict();
		virtual void	OnDataChanged( DataUpdateType_t type );

		virtual bool	OnFireEvent( C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options );

	#else

		virtual void	Spawn();

	#endif

	float		m_flPrevAnimTime;
	float  m_flNextResetCheckTime;

	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}

protected:

	bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered

private:

	CWeaponHL2MPBase( const CWeaponHL2MPBase & );

	Vector m_vOriginalSpawnOrigin;
	QAngle m_vOriginalSpawnAngles;
};


#endif // WEAPON_HL2MPBASE_H