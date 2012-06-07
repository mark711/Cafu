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

#include "Butterfly.hpp"
#include "TypeSys.hpp"
#include "EntityCreateParams.hpp"
#include "Libs/LookupTables.hpp"
#include "../../GameWorld.hpp"
#include "Models/Model_cmdl.hpp"


// Implement the type info related code.
const cf::TypeSys::TypeInfoT* EntButterflyT::GetType() const
{
    return &TypeInfo;
 // return &EntButterflyT::TypeInfo;
}

void* EntButterflyT::CreateInstance(const cf::TypeSys::CreateParamsT& Params)
{
    return new EntButterflyT(*static_cast<const EntityCreateParamsT*>(&Params));
}

const cf::TypeSys::TypeInfoT EntButterflyT::TypeInfo(GetBaseEntTIM(), "EntButterflyT", "BaseEntityT", EntButterflyT::CreateInstance, NULL /*MethodsList*/);


EntButterflyT::EntButterflyT(const EntityCreateParamsT& Params)
    : BaseEntityT(Params,
                  0,
                  EntityStateT(Params.Origin+VectorT(0.0, 500.0, 0.0),          // Beachte die Abhängigkeit von den in Think() definierten Konstanten!
                               VectorT(),
                               BoundingBox3T<double>(VectorT( 100.0,  100.0,  100.0),
                                                     VectorT(-100.0, -100.0, -100.0)),
                               16384,                                           // Beachte die Abhängigkeit von den in Think() definierten Konstanten!
                               0,
                               0,
                               0,
                               0,
                               0,       // ModelIndex
                               0,       // ModelSequNr
                               0.0,     // ModelFrameNr
                               20,      // Health
                               0,       // Armor
                               0,       // HaveItems
                               0,       // HaveWeapons
                               0,       // ActiveWeaponSlot
                               0,       // ActiveWeaponSequNr
                               0.0)),   // ActiveWeaponFrameNr
      m_Model(Params.GameWorld->GetModel("Games/DeathMatch/Models/LifeForms/Butterfly/Butterfly.cmdl")),
      ArcCenter(Params.Origin),
      ArcPos(0)
{
}


void EntButterflyT::Think(float FrameTime, unsigned long /*ServerFrameNr*/)
{
    const float DegPerSecond=7300.0;    // Entspricht 0.7 RadPerSecond oder 40 GradPerSecond
    const float ArcRadius   =500.0;

    ArcPos+=(unsigned short)(DegPerSecond*FrameTime);   // "wraps" automagically.

    // Info: Die Bogenlänge zwischen 'ArcPos' und 'ArcPos+1' bei 'ArcRadius==500.0' beträgt 0.048,
    // die "Auflösung" ist also mehr als ausreichend!
    State.Origin.x=ArcCenter.x+LookupTables::Angle16ToSin[ArcPos]*ArcRadius;
    State.Origin.y=ArcCenter.y+LookupTables::Angle16ToCos[ArcPos]*ArcRadius;
    State.Origin.z=ArcCenter.z+LookupTables::Angle16ToSin[(unsigned short)(ArcPos*2)]*ArcRadius*0.2;

    State.Heading=ArcPos+16384;
}


void EntButterflyT::Draw(bool /*FirstPersonView*/, float LodDist) const
{
    AnimPoseT* Pose=m_Model->GetSharedPose(m_Model->GetAnimExprPool().GetStandard(State.ModelSequNr, State.ModelFrameNr));
    int        SkinNr=-1;   // -1 is the default skin.

    if (m_Model->GetSkins().Size() > 0)
        SkinNr = ID % m_Model->GetSkins().Size();

    Pose->Draw(SkinNr, LodDist);
}


void EntButterflyT::PostDraw(float FrameTime, bool /*FirstPersonView*/)
{
    // Implicit simple "mini-prediction".
    IntrusivePtrT<AnimExprStandardT> AnimExpr=m_Model->GetAnimExprPool().GetStandard(State.ModelSequNr, State.ModelFrameNr);

    AnimExpr->SetForceLoop(true);
    AnimExpr->AdvanceTime(FrameTime);

    State.ModelFrameNr=AnimExpr->GetFrameNr();
}
