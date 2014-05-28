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

#include "extension.h"
#include <sourcehook.h>
#include <IBinTools.h>
#include <iplayerinfo.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

CSGONamedItemEcon g_CSGO_GNI_ECON;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_CSGO_GNI_ECON);

IGameConfig *g_pGameConf = NULL;

#define MAXPLAYERS 65
int g_iHookId[MAXPLAYERS+1];
int g_iInventoryOffset = 0;
int g_iGetItemInLoadout = 0;
ICallWrapper *g_pItemSchema = NULL;
ICallWrapper *g_pGetItemInLoadout = NULL;
ICallWrapper *g_pGetDefintionIndex = NULL;
ICallWrapper *g_pGetItemDefintionByName = NULL;
ICallWrapper *g_pGetLoadoutSlot = NULL;
IForward *g_pOnGiveNamedItem = NULL;
IBinTools *g_pBinTools = NULL;
class CEconItemView;

void RemoveGiveNamedItemHook(int client);
void LevelShutdown();
CBaseEntity *GiveNamedItem(const char *szItem, int iSubType, CEconItemView *pView, bool removeIfNotCarried);
CEconItemView *GetEconItemView(void *pItemDef, CBaseEntity *pEnt, int iLoadoutSlot, int iTeam);

SH_DECL_MANUALHOOK4(GiveNamedItemHook, 0, 0, 0, CBaseEntity *, const char *, int, CEconItemView *, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, false);

bool CSGONamedItemEcon::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255];
	
	if(!gameconfs->LoadGameConfigFile("csgo_econ_item.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		snprintf(error, maxlength, "Could not read csgo_econ_item.games: %s", conf_error);
		return false;
	}

	int offset = -1;

	if(!g_pGameConf->GetOffset("GiveNamedItem", &offset))
	{
		snprintf(error, maxlength, "Failed to get GiveNamedItem offset");
		return false;
	}
	if(!g_pGameConf->GetOffset("InventoryOffset", &g_iInventoryOffset))
	{
		snprintf(error, maxlength, "Failed to get InventoryOffset offset");
		return false;
	}
	if(!g_pGameConf->GetOffset("GetItemInLoadout", &g_iGetItemInLoadout))
	{
		snprintf(error, maxlength, "Failed to get GetItemInLoadout offset");
		return false;
	}

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	SH_MANUALHOOK_RECONFIGURE(GiveNamedItemHook, offset, 0, 0);
	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_STATIC(LevelShutdown), true);
	playerhelpers->AddClientListener(&g_CSGO_GNI_ECON);
	for(int i = 0; i <= MAXPLAYERS; i++)
	{
		g_iHookId[i] = 0;
	}
	g_pOnGiveNamedItem = forwards->CreateForward("GNIEcon_OnGiveNamedItem", ET_Hook, 5, NULL, Param_Cell, Param_Cell, Param_Cell, Param_Cell, Param_String); 
	return true;
}
void CSGONamedItemEcon::SDK_OnUnload()
{
	for(int i = 0; i <= MAXPLAYERS; i++)
	{
		if(g_iHookId[i] != 0)
		{
			RemoveGiveNamedItemHook(i);
		}
	}
	if(g_pGetItemInLoadout)
	{
		g_pGetItemInLoadout->Destroy();
		g_pGetItemInLoadout = NULL;
	}
	if(g_pItemSchema)
	{
		g_pItemSchema->Destroy();
		g_pItemSchema = NULL;
	}
	if(g_pGetItemDefintionByName)
	{
		g_pGetItemDefintionByName->Destroy();
		g_pGetItemDefintionByName = NULL;
	}
	if(g_pGetDefintionIndex)
	{
		g_pGetDefintionIndex->Destroy();
		g_pGetDefintionIndex = NULL;
	}
	if(g_pGetLoadoutSlot)
	{
		g_pGetLoadoutSlot->Destroy();
		g_pGetLoadoutSlot = NULL;
	}
	SH_REMOVE_HOOK(IServerGameDLL, LevelShutdown, gamedll, SH_STATIC(LevelShutdown), true);
}
void CSGONamedItemEcon::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
}
void CSGONamedItemEcon::OnClientPutInServer(int client)
{
	CBaseEntity *pEnt = gamehelpers->ReferenceToEntity(client);
	if(pEnt)
	{
		g_iHookId[client] = SH_ADD_MANUALHOOK(GiveNamedItemHook, pEnt, SH_STATIC(GiveNamedItem), false);
	}
}
void CSGONamedItemEcon::OnClientDisconnected(int client)
{
	RemoveGiveNamedItemHook(client);
}
void LevelShutdown()
{
	for(int i = 0; i <= MAXPLAYERS; i++)
	{
		RemoveGiveNamedItemHook(i);
	}
}

void *GetItemSchema()
{
	static void *addr = NULL;
	if(!addr)
	{
		if(!g_pGameConf->GetMemSig("GetItemSchema", &addr) || !addr)
		{
			smutils->LogError(myself, "Failed to GetItemSchema location");
			return NULL;
		}
		PassInfo ret;
		ret.flags = PASSFLAG_BYVAL;
		ret.type = PassType_Basic;
		ret.size = sizeof(void *);
		g_pItemSchema = g_pBinTools->CreateCall(addr, CallConv_Cdecl, &ret, NULL, 0);
	}

	void *pSchema = NULL;
	g_pItemSchema->Execute(NULL, &pSchema);
	return pSchema;
}

void *GetItemDefinitionByName(const char *szItem)
{
	void *pSchema = GetItemSchema();

	if(!pSchema)
		return 0;

	if(!g_pGetItemDefintionByName)
	{
		int offset = -1;

		if(!g_pGameConf->GetOffset("GetItemDefintionByName", &offset))
		{
			smutils->LogError(myself, "Failed to GetItemDefintionByName offset");
			return 0;
		}

		PassInfo pass[1];
		PassInfo ret;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].type  = PassType_Basic;
		pass[0].size  = sizeof(const char *);

		ret.flags = PASSFLAG_BYREF;
		ret.type = PassType_Basic;
		ret.size = sizeof(void *);

		g_pGetItemDefintionByName = g_pBinTools->CreateVCall(offset, 0, 0, &ret, pass, 1);
	}

	unsigned char vstk[sizeof(void *) + sizeof(const char *)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pSchema;
	vptr += sizeof(void *);
	*(const char **)vptr = szItem;

	void *pItemDef = 0;
	g_pGetItemDefintionByName->Execute(vstk, &pItemDef);
	return pItemDef;
}

int GetDefintionIndex(void *pItemDef)
{
	if(!pItemDef)
	{
		return 0;
	}

	if(!g_pGetDefintionIndex)
	{
		int offset = -1;

		if(!g_pGameConf->GetOffset("GetDefintionIndex", &offset))
		{
			smutils->LogError(myself, "Failed to GetDefintionIndex offset");
			return 0;
		}

		PassInfo ret;
		ret.flags = PASSFLAG_BYVAL;
		ret.type  = PassType_Basic;
		ret.size  = sizeof(uint16_t);
		g_pGetDefintionIndex = g_pBinTools->CreateVCall(offset, 0, 0, &ret, NULL, 0);
	}

	unsigned char vstk[sizeof(void *)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pItemDef;

	uint16_t id = 0;
	g_pGetDefintionIndex->Execute(vstk, &id);

	return id;
}

int GetLoadoutSlot(void *pItemDef, int iTeam)
{
	static void *addr = NULL;
	if(!addr)
	{
		if(!g_pGameConf->GetMemSig("GetLoadoutSlot", &addr) || !addr)
		{
			smutils->LogError(myself, "Failed to GetLoadoutSlot location");
			return 0;
		}

		PassInfo pass[1];
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].type  = PassType_Basic;
		pass[0].size  = sizeof(int);

		PassInfo ret;
		ret.flags = PASSFLAG_BYVAL;
		ret.type = PassType_Basic;
		ret.size = sizeof(int);
		g_pGetLoadoutSlot = g_pBinTools->CreateCall(addr, CallConv_ThisCall, &ret, pass, 1);
	}

	unsigned char vstk[sizeof(void *) + sizeof(int)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pItemDef;
	vptr += sizeof(void *);
	*(int *)vptr = iTeam;

	int slot = 0;
	g_pGetLoadoutSlot->Execute(vstk, &slot);

	return slot;
}

CBaseEntity *GiveNamedItem(const char *szItem, int iSubType, CEconItemView *pView, bool removeIfNotCarried)
{
	if(pView == NULL)
	{
		CBaseEntity *pEnt = META_IFACEPTR(CBaseEntity);
		IPlayerInfo *playerinfo = playerhelpers->GetGamePlayer(gamehelpers->EntityToBCompatRef(pEnt))->GetPlayerInfo();
		if(playerinfo)
		{
			void *pItemDef = GetItemDefinitionByName(szItem);
			uint16_t id = GetDefintionIndex(pItemDef);
			if(id)
			{
				int iTeam = playerinfo->GetTeamIndex();
				if(iTeam == 2 || iTeam == 3)
				{
					int iLoadoutSlot = GetLoadoutSlot(pItemDef, iTeam);

					cell_t res = PLUGIN_CONTINUE;
					g_pOnGiveNamedItem->PushCell(gamehelpers->EntityToBCompatRef(pEnt));
					g_pOnGiveNamedItem->PushCell(id);
					g_pOnGiveNamedItem->PushCell(iTeam);
					g_pOnGiveNamedItem->PushCell(iLoadoutSlot);
					g_pOnGiveNamedItem->PushString(szItem);
					g_pOnGiveNamedItem->Execute(&res);

					if(res == PLUGIN_CONTINUE)
					{
						CEconItemView *pNewView = GetEconItemView(pItemDef, pEnt, iLoadoutSlot, iTeam);
						RETURN_META_VALUE_MNEWPARAMS(MRES_HANDLED, NULL, GiveNamedItemHook, (szItem, iSubType, pNewView, removeIfNotCarried));
					}
				}
			}
		}
	}
	RETURN_META_VALUE(MRES_IGNORED, NULL);
}

CEconItemView *GetEconItemView(void *pItemDef, CBaseEntity *pEnt, int iLoadoutSlot, int iTeam)
{
	if(!g_pGetItemInLoadout)
	{
		PassInfo pass[2];
		PassInfo ret;
		pass[0].flags = PASSFLAG_BYVAL;
		pass[0].type  = PassType_Basic;
		pass[0].size  = sizeof(int);
		pass[1].flags = PASSFLAG_BYVAL;
		pass[1].type  = PassType_Basic;
		pass[1].size  = sizeof(int);

		ret.flags = PASSFLAG_BYVAL;
		ret.type = PassType_Basic;
		ret.size = sizeof(void *);

		g_pGetItemInLoadout = g_pBinTools->CreateVCall(g_iGetItemInLoadout, 0, 0, &ret, pass, 2);
	}

	if(iLoadoutSlot == -1)
	{
		if(iTeam == 3)
		{
			iTeam = 2;
			iLoadoutSlot = GetLoadoutSlot(pItemDef, iTeam);
		}
		else if(iTeam == 2)
		{
			iTeam = 3;
			iLoadoutSlot = GetLoadoutSlot(pItemDef, iTeam);
		}
		else
		{
			return NULL;
		}
	}
	unsigned char vstk[sizeof(void *) + sizeof(int) * 2];
	unsigned char *vptr = vstk;

	*(void **)vptr = (void *)((intptr_t)pEnt + g_iInventoryOffset);
	vptr += sizeof(void *);
	*(int *)vptr = iTeam;
	vptr += sizeof(int);
	*(int *)vptr = iLoadoutSlot;

	CEconItemView *pView = NULL;
	g_pGetItemInLoadout->Execute(vstk, &pView);

	return pView;
}
void RemoveGiveNamedItemHook(int client)
{
	if(g_iHookId[client] != 0)
	{
		SH_REMOVE_HOOK_ID(g_iHookId[client]);
		g_iHookId[client] = 0;
	}
}