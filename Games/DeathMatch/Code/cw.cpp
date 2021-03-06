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

#include "cw.hpp"

using namespace GAME_NAME;


CarriedWeaponT::CarriedWeaponT(const CafuModelT* ViewModel, const CafuModelT* PlayerModel)
    : m_ViewModel(ViewModel),
      m_PlayerModel(PlayerModel)
{
}


CarriedWeaponT::~CarriedWeaponT()
{
}


bool CarriedWeaponT::ServerSide_PickedUpByEntity(EntHumanPlayerT* /*Player*/) const { return false; }
void CarriedWeaponT::ServerSide_Think(EntHumanPlayerT* /*Player*/, const PlayerCommandT& /*PlayerCommand*/, bool /*ThinkingOnServerSide*/, unsigned long /*ServerFrameNr*/, bool /*AnimSequenceWrap*/) const { }
void CarriedWeaponT::ClientSide_HandlePrimaryFireEvent(const EntHumanPlayerT* /*Player*/, const VectorT& /*LastSeenAmbientColor*/) const { }
void CarriedWeaponT::ClientSide_HandleSecondaryFireEvent(const EntHumanPlayerT* /*Player*/, const VectorT& /*LastSeenAmbientColor*/) const { }
void CarriedWeaponT::ClientSide_HandleStateDrivenEffects(const EntHumanPlayerT* /*Player*/) const { }
