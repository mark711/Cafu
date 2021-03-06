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

#include "MonsterMaker.hpp"
#include "Eagle.hpp"
#include "CompanyBot.hpp"
#include "EntityCreateParams.hpp"
#include "HumanPlayer.hpp"
#include "InfoPlayerStart.hpp"
#include "TypeSys.hpp"
#include "../../GameWorld.hpp"

#ifndef _WIN32
#define _stricmp strcasecmp
#endif

using namespace GAME_NAME;


// Implement the type info related code.
const cf::TypeSys::TypeInfoT* EntMonsterMakerT::GetType() const
{
    return &TypeInfo;
 // return &EntMonsterMakerT::TypeInfo;
}

void* EntMonsterMakerT::CreateInstance(const cf::TypeSys::CreateParamsT& Params)
{
    return new EntMonsterMakerT(*static_cast<const EntityCreateParamsT*>(&Params));
}

const cf::TypeSys::TypeInfoT EntMonsterMakerT::TypeInfo(GetBaseEntTIM(), "EntMonsterMakerT", "BaseEntityT", EntMonsterMakerT::CreateInstance, NULL /*MethodsList*/);


EntMonsterMakerT::EntMonsterMakerT(const EntityCreateParamsT& Params)
    : BaseEntityT(Params,
                  BoundingBox3dT(Vector3dT( 100.0,  100.0,  100.0),
                                 Vector3dT(-100.0, -100.0, -100.0)),
                  0),
      MonsterType(Unknown),
      MaxCreate(0),
      Delay(0.0),
      MaxAlive(0),
      CurrentlyAlive(0),
      TimeSinceLastMake(0.0)
{
    // Werte die 'PropertyPairs' aus, die von der Basis-Klasse 'BaseEntity' noch nicht ausgewertet wurden!
    for (std::map<std::string, std::string>::const_iterator It=Properties.begin(); It!=Properties.end(); ++It)
    {
        const std::string& Key  =It->first;
        const std::string& Value=It->second;

        if (Key=="monstertype")
        {
                 if (_stricmp(Value.c_str(), "CompanyBot")==0) MonsterType=CompanyBot;
            else if (_stricmp(Value.c_str(), "Butterfly" )==0) MonsterType=Butterfly;
            else if (_stricmp(Value.c_str(), "Eagle"     )==0) MonsterType=Eagle;
        }
        else if (Key=="monstercount"      ) MaxCreate=atoi(Value.c_str());
        else if (Key=="delay"             ) Delay    =float(atof(Value.c_str()));
        else if (Key=="m_imaxlivechildren") MaxAlive =atoi(Value.c_str());
    }
}


void EntMonsterMakerT::Think(float FrameTime, unsigned long ServerFrameNr)
{
    if (MonsterType==Unknown) return;       // Cannot create monsters of the 'Unknown' type
    if (!MaxCreate) return;                 // We have created all children, create no more
    if (CurrentlyAlive>=MaxAlive) return;   // Do not create more children than are allowed at one time

    TimeSinceLastMake+=FrameTime;
    if (TimeSinceLastMake<Delay) return;    // No time to create next monster yet

    // Wir wollen nur dann einen neuen Entity erzeugen, wenn die geplante Position frei ist.
    // Beachte jedoch: Das kann auch schiefgehen, wenn der erzeugte Entity in seinem Konstruktor eine andere als die geplante Position wählt!
    // Ein Beispiel dafür ist der CompanyBot, der nachträglich seine Höhe über Grund korrigiert.
    const ArrayT<unsigned long>& AllEntityIDs=GameWorld->GetAllEntityIDs();
    BoundingBox3T<double>        OurRelPositionBB;
    unsigned long                EntityIDNr;

    switch (MonsterType)
    {
        // Das ist ein häßlicher Hack: Muß hier im voraus die Dimensions-BoundingBox des CompanyBot-Konstruktors kennen!
        case CompanyBot: OurRelPositionBB=BoundingBox3dT(Vector3dT(300.0, 300.0, 100.0), Vector3dT(-300.0, -300.0, -1728.8)); break;
        case Butterfly : OurRelPositionBB=BoundingBox3dT(Vector3dT(100.0, 600.0, 100.0), Vector3dT(-100.0,  400.0,  -100.0)); break;
        case Eagle     : OurRelPositionBB=BoundingBox3dT(Vector3dT(100.0, 100.0, 100.0), Vector3dT(-100.0, -100.0,  -100.0)); break;
        default        : break;
    }

    for (EntityIDNr=0; EntityIDNr<AllEntityIDs.Size(); EntityIDNr++)
    {
        IntrusivePtrT<GameEntityI> BaseEntity=GameWorld->GetGameEntityByID(AllEntityIDs[EntityIDNr]);
        if (BaseEntity==NULL) continue;

        if (BaseEntity->GetType()==&EntMonsterMakerT::TypeInfo   ) continue;
        if (BaseEntity->GetType()==&EntInfoPlayerStartT::TypeInfo) continue;

        BoundingBox3T<double> BaseEntityBB=BaseEntity->GetDimensions();

        BaseEntityBB.Min=BaseEntityBB.Min+BaseEntity->GetOrigin()-m_Origin;
        BaseEntityBB.Max=BaseEntityBB.Max+BaseEntity->GetOrigin()-m_Origin;

        if (OurRelPositionBB.GetEpsilonBox(1.0).Intersects(BaseEntityBB)) return;
    }

    switch (MonsterType)
    {
        case CompanyBot:
        {
            // Finde den ersten HumanPlayer-Entity, der 'alive' und ein Stück von uns entfernt ist.
            for (EntityIDNr=0; EntityIDNr<AllEntityIDs.Size(); EntityIDNr++)
            {
                bool IsAlive=false;

                IntrusivePtrT<BaseEntityT> BaseEntity=static_pointer_cast<BaseEntityT>(GameWorld->GetGameEntityByID(AllEntityIDs[EntityIDNr]));
                if (BaseEntity==NULL) continue;

                if (BaseEntity->GetType()!=&EntHumanPlayerT::TypeInfo) continue;

                BaseEntity->ProcessConfigString(&IsAlive, "IsAlive?");
                if (!IsAlive) continue;

                // Do not spawn until player is beyond safety distance (VectorDistance(BaseEntity->GetOrigin(), m_Origin)<5000 ohne Quadratwurzel!)
                VectorT Dist=BaseEntity->GetOrigin()-m_Origin;
                Dist.z=0;   // Temporary fix for broken CompanyBot constructor. See there!
                if (dot(Dist, Dist)<6500.0*6500.0) return;

                // Gefunden!
                break;
            }

            // Gab es so einen Entity?
            if (EntityIDNr>=AllEntityIDs.Size()) return;

            std::map<std::string, std::string> Props; Props["classname"]="monster_companybot";
            unsigned long   NewCompanyBotID=GameWorld->CreateNewEntity(Props, ServerFrameNr, m_Origin);
            IntrusivePtrT<EntCompanyBotT> NewCompanyBot=dynamic_pointer_cast<EntCompanyBotT>(GameWorld->GetGameEntityByID(NewCompanyBotID));

            if (!NewCompanyBot.IsNull()) NewCompanyBot->SetHeading(m_Heading);
            break;
        }

        case Butterfly:
        {
            std::map<std::string, std::string> Props; Props["classname"]="monster_butterfly";

            GameWorld->CreateNewEntity(Props, ServerFrameNr, m_Origin);
            break;
        }

        case Eagle:
        {
            std::map<std::string, std::string> Props; Props["classname"]="monster_eagle";

            unsigned long NewEagleID=GameWorld->CreateNewEntity(Props, ServerFrameNr, m_Origin);
            IntrusivePtrT<EntEagleT> NewEagle=dynamic_pointer_cast<EntEagleT>(GameWorld->GetGameEntityByID(NewEagleID));

            if (!NewEagle.IsNull()) NewEagle->SetHeading(m_Heading);
            break;
        }

        default: break;
    }

    MaxCreate--;
    CurrentlyAlive++;
    TimeSinceLastMake=0;
}
