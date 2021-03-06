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

#include "Corpse.hpp"
#include "EntityCreateParams.hpp"
#include "HumanPlayer.hpp"
#include "GameImpl.hpp"
#include "TypeSys.hpp"
#include "MaterialSystem/Renderer.hpp"
#include "Models/Model_cmdl.hpp"
#include "Network/State.hpp"

using namespace GAME_NAME;


// Implement the type info related code.
const cf::TypeSys::TypeInfoT* EntCorpseT::GetType() const
{
    return &TypeInfo;
 // return &EntCorpseT::TypeInfo;
}

void* EntCorpseT::CreateInstance(const cf::TypeSys::CreateParamsT& Params)
{
    return new EntCorpseT(*static_cast<const EntityCreateParamsT*>(&Params));
}

const cf::TypeSys::TypeInfoT EntCorpseT::TypeInfo(GetBaseEntTIM(), "EntCorpseT", "BaseEntityT", EntCorpseT::CreateInstance, NULL /*MethodsList*/);


EntCorpseT::EntCorpseT(const EntityCreateParamsT& Params)
    : BaseEntityT(Params,
                  BoundingBox3dT(Vector3dT()),
                  0),
      m_ModelIndex(0),
      m_ModelSequNr(0),
      m_ModelFrameNr(0.0f)
{
}


void EntCorpseT::AdoptState(const EntHumanPlayerT* Player)
{
    m_Origin       = Player->GetOrigin()+VectorT(0.0, 0.0, Player->GetDimensions().Min.z+1728.8);
    m_Dimensions   = BoundingBox3dT(Vector3dT());
    m_Heading      = Player->GetHeading();
    m_Pitch        = 0;
    m_Bank         = 0;
    m_ModelIndex   = Player->GetState().ModelIndex;
    m_ModelSequNr  = Player->GetState().ModelSequNr;
    m_ModelFrameNr = Player->GetState().ModelFrameNr;
}


void EntCorpseT::DoSerialize(cf::Network::OutStreamT& Stream) const
{
    Stream << m_ModelIndex;
    Stream << m_ModelSequNr;
    Stream << m_ModelFrameNr;
}


void EntCorpseT::DoDeserialize(cf::Network::InStreamT& Stream)
{
    Stream >> m_ModelIndex;
    Stream >> m_ModelSequNr;
    Stream >> m_ModelFrameNr;
}


void EntCorpseT::Think(float /*FrameTime*/, unsigned long /*ServerFrameNr*/)
{
    // TODO: Disappear when some condition is met (timeout, not in anyones PVS, alpha fade-out, too many corpses, ...)
    // TODO: Decompose to gibs when hit by a rocket.
}


void EntCorpseT::Draw(bool /*FirstPersonView*/, float LodDist) const
{
    MatSys::Renderer->GetCurrentLightSourcePosition()[2]+=32.0f;
    MatSys::Renderer->GetCurrentEyePosition        ()[2]+=32.0f;
    MatSys::Renderer->Translate(MatSys::RendererI::MODEL_TO_WORLD, 0.0f, 0.0f, -32.0f);

    const CafuModelT* Model=GameImplT::GetInstance().GetPlayerModel(m_ModelIndex);
    AnimPoseT*        Pose =Model->GetSharedPose(Model->GetAnimExprPool().GetStandard(m_ModelSequNr, m_ModelFrameNr));

    Pose->Draw(-1 /*default skin*/, LodDist);

    // if (m_HaveWeapons & (1 << m_ActiveWeaponSlot))
    // {
    //     const CafuModelT* WeaponModel=...;
    //     AnimPoseT*        WeaponPose =WeaponModel->GetSharedPose(0, 0.0f);
    //
    //     WeaponPose->SetSuperPose(Pose);
    //     WeaponPose->Draw(-1 /*default skin*/, LodDist);
    //     WeaponPose->SetSuperPose(NULL);
    // }
}
