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

#ifndef CAFU_ITEM_HPP_INCLUDED
#define CAFU_ITEM_HPP_INCLUDED

#include "../../BaseEntity.hpp"


class CafuModelT;
class EntityCreateParamsT;
class SoundI;


class EntItemT : public BaseEntityT
{
    public:

    EntItemT(const EntityCreateParamsT& Params, const std::string& ModelName);
    ~EntItemT();

    virtual void Think(float FrameTime, unsigned long ServerFrameNr);

    virtual void ProcessEvent(unsigned int EventType, unsigned int NumEvents);
    virtual void Draw(bool FirstPersonView, float LodDist) const;
    virtual void PostDraw(float FrameTime, bool FirstPersonView);


    const cf::TypeSys::TypeInfoT* GetType() const;
    static void* CreateInstance(const cf::TypeSys::CreateParamsT& Params);
    static const cf::TypeSys::TypeInfoT TypeInfo;


    protected:

    enum EventTypesT { EVENT_TYPE_PICKED_UP, EVENT_TYPE_RESPAWN, NUM_EVENT_TYPES };

    static const char StateOfExistance_Active;
    static const char StateOfExistance_NotActive;

    const CafuModelT* m_ItemModel;
    float             m_TimeLeftNotActive;

    SoundI* PickUp;
    SoundI* Respawn;
};

#endif
