/*
=================================================================================
This file is part of Cafu, the open-source game engine and graphics engine
for multiplayer, cross-platform, real-time 3D action.
Copyright (C) 2002-2012 Carsten Fuchs Software.

Cafu is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

Cafu is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Cafu. If not, see <http://www.gnu.org/licenses/>.

For support and more information about Cafu, visit us at <http://www.cafu.de>.
=================================================================================
*/

#include "cw_Gauss.hpp"
#include "HumanPlayer.hpp"
#include "Constants_AmmoSlots.hpp"
#include "Constants_WeaponSlots.hpp"
#include "Libs/LookupTables.hpp"
#include "Models/ModelManager.hpp"

using namespace GAME_NAME;


CarriedWeaponGaussT::CarriedWeaponGaussT(ModelManagerT& ModelMan)
    : CarriedWeaponT(ModelMan.GetModel("Games/DeathMatch/Models/Weapons/Gauss/Gauss_v.cmdl"),
                     ModelMan.GetModel("Games/DeathMatch/Models/Weapons/Gauss/Gauss_p.cmdl"))
{
}


bool CarriedWeaponGaussT::ServerSide_PickedUpByEntity(EntHumanPlayerT* Player) const
{
    EntityStateT& State=Player->GetState();

    // Consider if the entity already has this weapon.
    if (State.HaveWeapons & (1 << WEAPON_SLOT_GAUSS))
    {
        // If it also has the max. amount of ammo of this type, ignore the touch.
        if (State.HaveAmmo[AMMO_SLOT_CELLS]==200) return false;

        // Otherwise pick the weapon up and let it have the ammo.
        State.HaveAmmo[AMMO_SLOT_CELLS]+=40;
    }
    else
    {
        // This weapon is picked up for the first time.
        State.HaveWeapons|=1 << WEAPON_SLOT_GAUSS;
        State.ActiveWeaponSlot   =WEAPON_SLOT_GAUSS;
        State.ActiveWeaponSequNr =8;    // Draw
        State.ActiveWeaponFrameNr=0.0;

        State.HaveAmmoInWeapons[WEAPON_SLOT_GAUSS] =20;
        State.HaveAmmo         [AMMO_SLOT_CELLS  ]+=20;
    }

    // Limit the amount of carryable ammo.
    if (State.HaveAmmo[AMMO_SLOT_CELLS]>200) State.HaveAmmo[AMMO_SLOT_CELLS]=200;

    return true;
}


void CarriedWeaponGaussT::ServerSide_Think(EntHumanPlayerT* Player, const PlayerCommandT& PlayerCommand, bool /*ThinkingOnServerSide*/, unsigned long /*ServerFrameNr*/, bool AnimSequenceWrap) const
{
    EntityStateT& State=Player->GetState();

    enum SequenceNames
    {
        Idle1,
        Idle2,
        Fidget,
        SpinUp,
        Spin,
        Fire1,
        Fire2,
        Holster,
        Draw
    };

    switch (State.ActiveWeaponSequNr)
    {
        case Draw:
            if (AnimSequenceWrap)
            {
                State.ActiveWeaponSequNr =Idle1;
                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case Idle1:
        case Idle2:
        case Fidget:
            if (AnimSequenceWrap)
            {
                switch (LookupTables::RandomUShort[PlayerCommand.Nr & 0xFFF] & 3)
                {
                    case  0: State.ActiveWeaponSequNr=Idle2;
                    case  1: State.ActiveWeaponSequNr=Fidget;
                    default: State.ActiveWeaponSequNr=Idle1;
                }

                State.ActiveWeaponFrameNr=0.0;
            }
            break;
    }
}
