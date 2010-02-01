/*
=================================================================================
This file is part of Cafu, the open-source game and graphics engine for
multiplayer, cross-platform, real-time 3D action.
$Id$

Copyright (C) 2002-2010 Carsten Fuchs Software.

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

/*************************/
/*** Weapon RPG (Code) ***/
/*************************/

#include "WeaponRPG.hpp"
#include "cw.hpp"
#include "Constants_AmmoSlots.hpp"
#include "Constants_WeaponSlots.hpp"
#include "EntityCreateParams.hpp"
#include "HumanPlayer.hpp"
#include "TypeSys.hpp"


// Implement the type info related code.
const cf::TypeSys::TypeInfoT* EntWeaponRPGT::GetType() const
{
    return &TypeInfo;
 // return &EntWeaponRPGT::TypeInfo;
}

void* EntWeaponRPGT::CreateInstance(const cf::TypeSys::CreateParamsT& Params)
{
    return new EntWeaponRPGT(*static_cast<const EntityCreateParamsT*>(&Params));
}

const cf::TypeSys::TypeInfoT EntWeaponRPGT::TypeInfo(GetBaseEntTIM(), "EntWeaponRPGT", "EntWeaponT", EntWeaponRPGT::CreateInstance, NULL /*MethodsList*/);


EntWeaponRPGT::EntWeaponRPGT(const EntityCreateParamsT& Params)
    : EntWeaponT(Params, "Games/DeathMatch/Models/Weapons/Bazooka_w.mdl")
{
}


void EntWeaponRPGT::NotifyTouchedBy(BaseEntityT* Entity)
{
    // If we are touched by anything else than a human player, ignore the touch.
    // Would be interesting to also allow touchs by bots, though.
    if (Entity->GetType()!=&EntHumanPlayerT::TypeInfo) return;

    // If we are touched when not being "active", ignore the touch.
    if (State.StateOfExistance!=StateOfExistance_Active) return;

    // Give this weapon to the entity.
    if (!CarriedWeaponT::GetCarriedWeapon(WEAPON_SLOT_RPG)->ServerSide_PickedUpByEntity(Entity)) return;

    // And finally retire for a while.
    State.StateOfExistance=StateOfExistance_NotActive;
    State.Events^=(1 << EventID_PickedUp);
    TimeLeftNotActive=5.0;
}