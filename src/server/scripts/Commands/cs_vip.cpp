/*
 * Copyright (C) 2013-2015 DeathCore <http://www.noffearrdeathproject.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Chat.h"
#include "Common.h"
#include "Language.h"
#include "CellImpl.h"
#include "GridNotifiers.h"

class vipcommands : public CommandScript
{
public:
    vipcommands() : CommandScript("vipcommands") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand vipCommandTable[] =

        {
		{ "Buff",           SEC_VIP,  false, &HandleBuffCommand,		"", NULL },
		{ "Sala",	    SEC_VIP,     true, &HandleVipMallCommand,         "", NULL },
        { "Raça",    SEC_VIP,  false, &HandleChangeRaceCommand,             "", NULL },
	    { "Facção",	SEC_VIP,  false, &HandleChangeFactionCommand,		"", NULL },
	    { "Skills",	SEC_VIP,  false, &HandleMaxSkillsCommand,		"", NULL },
	    { "Visual",	SEC_VIP,  false, &HandleCustomizeCommand,		"", NULL },
	    { "Tele",           SEC_VIP,  false, &HandleTeleCommand,		"", NULL },
	    { "Forma",           SEC_VIP,  false, &HandleMorphCommand,		"", NULL },
        { NULL,             0,                     false, NULL,                                          "", NULL }
        };
        static ChatCommand commandTable[] =
        {
            { "Vip",	    SEC_VIP,   true, NULL,      "",  vipCommandTable},
	       { NULL,             0,                  false, NULL,                               "", NULL }
        };
        return commandTable;
    }

static bool HandleActivateCommand(ChatHandler * handler, const char * args)
{
	Player* player = handler->GetSession()->GetPlayer();

	if(player->GetSession()->GetSecurity() >= 1)
	{
		handler->PSendSysMessage("Você já é um Jogador VIP");
		handler->SetSentErrorMessage(true);
		return false;
	}

	if(player->HasItemCount(313370, 1, false)) // Token ID, Count.
	{
                  PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_ACCESS);
                  stmt->setUInt32(0, player->GetSession()->GetAccountId());
                  stmt->setUInt8(1, 1);
                  stmt->setInt32(2, 1);
	         LoginDatabase.Execute(stmt);
		player->DestroyItemCount(313370, 1, true, false); // Token ID, Count.
		handler->PSendSysMessage("Você agora é um Jogador VIP, para ativa-lo relogue e entre no jogo novamente");
		return true;
	}
	return true;
}

static bool HandleTeleCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        Player* me = handler->GetSession()->GetPlayer();

        // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
        GameTele const* tele = handler->extractGameTeleFromLink((char*)args);

        if (!tele)
        {
            handler->SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (me->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        MapEntry const* map = sMapStore.LookupEntry(tele->mapId);
        if (!map || map->IsBattlegroundOrArena())
        {
            handler->SendSysMessage(LANG_CANNOT_TELE_TO_BG);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (me->IsInFlight())
        {
            me->GetMotionMaster()->MovementExpired();
            me->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            me->SaveRecallPosition();

        me->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
        return true;
		}


static bool HandleBuffCommand(ChatHandler * handler, const char * args)
    {
        Player * pl = handler->GetSession()->GetPlayer();
		if(pl->IsInCombat())
		{
			pl->GetSession()->SendNotification("Você não pode usar isso em combate!!");
			return false;
		}
		if(pl->InArena())
		{
			pl->GetSession()->SendNotification("Você não pode usar isso em uma arena!");
			return false;
		}
		else
		pl->AddAura(15366, pl);
		pl->AddAura(16609, pl);
		pl->AddAura(48162, pl);
		pl->AddAura(48074, pl);
		pl->AddAura(48170, pl);
		pl->AddAura(43223, pl);
		pl->AddAura(36880, pl);
		pl->AddAura(467, pl);
		pl->AddAura(69994, pl);
		pl->AddAura(33081, pl);
		pl->AddAura(24705, pl);
		pl->AddAura(26035, pl);
		pl->AddAura(48469, pl);
		handler->PSendSysMessage("|cffFFFFFF[|cffFFA500Buff Vip|cffFFFFFF] |cffFF0000Você está quase imortal!");
		return true;

    }


static bool HandlevipCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();

            me->Say("Comandos V.I.P?", LANG_UNIVERSAL);
            return true;
    }

static bool HandleMorphCommand(ChatHandler* handler, const char* args)
    {
        handler->GetSession()->GetPlayer()->SetDisplayId((uint32)atol((char*)args));
        return true;
    }

static bool HandleChangeRaceCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
		handler->PSendSysMessage("Relogue para mudar sua raça!");
        return true;
    }

static bool HandleChangeFactionCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
		handler->PSendSysMessage("Relogue para mudar sua facção!");
        return true;
    }

static bool HandleMaxSkillsCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->UpdateSkillsForLevel();
		handler->PSendSysMessage("Suas habilidades de armas agora estão no nível máximo!");
        return true;
    }

static bool HandleCustomizeCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();
		me->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
		handler->PSendSysMessage("Relogue para mudar o seu visual!");
        return true;
    }

		static bool HandleVipMallCommand(ChatHandler* handler, const char* args)
    {

        Player* me = handler->GetSession()->GetPlayer();

        if (me->IsInCombat())
        {
            handler->SendSysMessage(LANG_YOU_IN_COMBAT);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // stop flight if need
        if (me->IsInFlight())
        {
            me->GetMotionMaster()->MovementExpired();
            me->CleanupAfterTaxiFlight();
        }
        // save only in non-flight case
        else
            me->SaveRecallPosition();

		me->TeleportTo(1, 4847.365234f, -2811.368408f, 1444.530029f, 6.261538f); // MapId, X, Y, Z, O
                return true;
    }

};

void AddSC_vipcommands()
{
    new vipcommands();
}