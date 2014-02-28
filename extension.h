/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"

enum CSGOWeapon
{
	CSGOWeapon_NONE,
	CSGOWeapon_DEAGLE,
	CSGOWeapon_ELITE,
	CSGOWeapon_FIVESEVEN,
	CSGOWeapon_GLOCK,
	CSGOWeapon_P228,
	CSGOWeapon_USP,
	CSGOWeapon_AK47,
	CSGOWeapon_AUG,
	CSGOWeapon_AWP,
	CSGOWeapon_FAMAS,
	CSGOWeapon_G3SG1,
	CSGOWeapon_GALIL,
	CSGOWeapon_GALILAR,
	CSGOWeapon_M249,
	CSGOWeapon_M3,
	CSGOWeapon_M4A1,
	CSGOWeapon_MAC10,
	CSGOWeapon_MP5NAVY,
	CSGOWeapon_P90,
	CSGOWeapon_SCOUT,
	CSGOWeapon_SG550,
	CSGOWeapon_SG552,
	CSGOWeapon_TMP,
	CSGOWeapon_UMP45,
	CSGOWeapon_XM1014,
	CSGOWeapon_BIZON,
	CSGOWeapon_MAG7,
	CSGOWeapon_NEGEV,
	CSGOWeapon_SAWEDOFF,
	CSGOWeapon_TEC9,
	CSGOWeapon_TASER,
	CSGOWeapon_HKP2000,
	CSGOWeapon_MP7,
	CSGOWeapon_MP9,
	CSGOWeapon_NOVA,
	CSGOWeapon_P250,
	CSGOWeapon_SCAR17,
	CSGOWeapon_SCAR20,
	CSGOWeapon_SG556,
	CSGOWeapon_SSG08,
	CSGOWeapon_KNIFE_GG,
	CSGOWeapon_KNIFE,
	CSGOWeapon_FLASHBANG,
	CSGOWeapon_HEGRENADE,
	CSGOWeapon_SMOKEGRENADE,
	CSGOWeapon_MOLOTOV,
	CSGOWeapon_DECOY,
	CSGOWeapon_INCGRENADE,
	CSGOWeapon_C4, //49
	CSGOWeapon_KEVLAR = 50,
	CSGOWeapon_ASSAULTSUIT,
	CSGOWeapon_NVG,
	CSGOWeapon_DEFUSER
};

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class CSGONamedItemEcon : public SDKExtension, public IClientListener
{
public:
	virtual void OnClientPutInServer(int client);
	virtual void OnClientDisconnected(int client);
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	//virtual bool QueryRunning(char *error, size_t maxlength);
public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength);
#endif
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
