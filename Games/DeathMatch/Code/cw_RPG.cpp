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

#include "cw_RPG.hpp"
#include "HumanPlayer.hpp"
#include "Rocket.hpp"
#include "Constants_AmmoSlots.hpp"
#include "Constants_WeaponSlots.hpp"
#include "Libs/LookupTables.hpp"
#include "../../GameWorld.hpp"
#include "Models/ModelManager.hpp"

using namespace GAME_NAME;


CarriedWeaponRPGT::CarriedWeaponRPGT(ModelManagerT& ModelMan)
    : CarriedWeaponT(ModelMan.GetModel("Games/DeathMatch/Models/Weapons/Bazooka/Bazooka_v.cmdl"),
                     ModelMan.GetModel("Games/DeathMatch/Models/Weapons/Bazooka/Bazooka_p.cmdl"))
{
}


bool CarriedWeaponRPGT::ServerSide_PickedUpByEntity(EntHumanPlayerT* Player) const
{
    EntityStateT& State=Player->GetState();

    // Consider if the entity already has this weapon.
    if (State.HaveWeapons & (1 << WEAPON_SLOT_RPG))
    {
        // If it also has the max. amount of ammo of this type, ignore the touch.
        if (State.HaveAmmo[AMMO_SLOT_ROCKETS]==5) return false;

        // Otherwise pick the weapon up and let it have the ammo.
        State.HaveAmmo[AMMO_SLOT_ROCKETS]+=1;
    }
    else
    {
        // This weapon is picked up for the first time.
        State.HaveWeapons|=1 << WEAPON_SLOT_RPG;
        State.ActiveWeaponSlot   =WEAPON_SLOT_RPG;
        State.ActiveWeaponSequNr =5;    // Draw
        State.ActiveWeaponFrameNr=0.0;

        State.HaveAmmoInWeapons[WEAPON_SLOT_RPG  ] =1;
        State.HaveAmmo         [AMMO_SLOT_ROCKETS]+=0;
    }

    // Limit the amount of carryable ammo.
    if (State.HaveAmmo[AMMO_SLOT_ROCKETS]>5) State.HaveAmmo[AMMO_SLOT_ROCKETS]=5;

    return true;
}


void CarriedWeaponRPGT::ServerSide_Think(EntHumanPlayerT* Player, const PlayerCommandT& PlayerCommand, bool ThinkingOnServerSide, unsigned long ServerFrameNr, bool AnimSequenceWrap) const
{
    EntityStateT& State=Player->GetState();

    switch (State.ActiveWeaponSequNr)
    {
        case 0: // Idle1 (rocket inserted)
        case 1: // Fidget1 (rocket inserted)
            if (PlayerCommand.Keys & (PCK_Fire1 | PCK_Fire2))
            {
                State.ActiveWeaponSequNr =3;    // Fire
                State.ActiveWeaponFrameNr=0.0;

                // Important: ONLY create (throw) a new rocket IF we are on the server side!
                if (ThinkingOnServerSide)
                {
                    // Clamp 'Pitch' values larger than 45° (==8192) to 45°.
                    const unsigned short Pitch=(Player->GetPitch()>8192 && Player->GetPitch()<=16384) ? 8192 : Player->GetPitch();

                    const float ViewDirZ=-LookupTables::Angle16ToSin[Pitch];
                    const float ViewDirY= LookupTables::Angle16ToCos[Pitch];

                    // Note: There is a non-trivial relationship between heading, pitch, and the corresponding view vector.
                    // Especially does a heading and pitch of 45° NOT correspond to the view vector (1, 1, 1), and vice versa!
                    // Think carefully about this before changing the number 1010.0 below (which actually is 2.0*(400.0+100.0) (+10.0 for "safety")).
                    const VectorT ViewDir(ViewDirY*LookupTables::Angle16ToSin[Player->GetHeading()], ViewDirY*LookupTables::Angle16ToCos[Player->GetHeading()], ViewDirZ);
                    const VectorT RocketOrigin(Player->GetOrigin()-VectorT(0.0, 0.0, 200.0)+scale(ViewDir, 1010.0)+scale(State.Velocity, double(PlayerCommand.FrameTime)));
                    std::map<std::string, std::string> Props;

                    Props["classname"]="monster_rocket";

                    unsigned long RocketID=Player->GameWorld->CreateNewEntity(Props, ServerFrameNr, RocketOrigin);

                    if (RocketID!=0xFFFFFFFF)
                    {
                        IntrusivePtrT<EntRocketT> Rocket=dynamic_pointer_cast<EntRocketT>(Player->GameWorld->GetGameEntityByID(RocketID));

                        Rocket->ParentID=Player->ID;
                        Rocket->SetHeading(Player->GetHeading());
                        Rocket->SetVelocity(scale(ViewDir, 14000.0));   // Rocket has own propulsion.
                    }
                }
                break;
            }

            if (AnimSequenceWrap)
            {
                if (State.ActiveWeaponSequNr==0)
                {
                    State.ActiveWeaponSequNr=LookupTables::RandomUShort[PlayerCommand.Nr & 0xFFF] & 1;
                }
                else State.ActiveWeaponSequNr=0;     // Don't play the "Fidget" sequence repeatedly.

                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case 2: // Reload
            if (AnimSequenceWrap)
            {
                State.ActiveWeaponSequNr =0;
                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case 3: // Fire
            if (AnimSequenceWrap)
            {
                State.ActiveWeaponSequNr =2;
                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case 4: // Holster1 (rocket inserted)
            break;

        case 5: // Draw1 (rocket inserted)
            if (AnimSequenceWrap)
            {
                State.ActiveWeaponSequNr =0;
                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case 6: // Holster2 (empty)
            break;

        case 7: // Draw2 (empty)
            if (AnimSequenceWrap)
            {
                State.ActiveWeaponSequNr =8;
                State.ActiveWeaponFrameNr=0.0;
            }
            break;

        case 8: // Idle2 (empty)
        case 9: // Fidget2 (empty)
            break;
    }
}
