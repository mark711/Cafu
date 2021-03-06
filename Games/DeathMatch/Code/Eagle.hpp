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

#ifndef CAFU_EAGLE_HPP_INCLUDED
#define CAFU_EAGLE_HPP_INCLUDED

#include "BaseEntity.hpp"


class CafuModelT;
class SoundI;


namespace GAME_NAME
{
    class EntEagleT : public BaseEntityT
    {
        public:

        EntEagleT(const EntityCreateParamsT& Params);
        ~EntEagleT();

        void SetHeading(unsigned short h) { m_Heading = h; }

        void Think(float FrameTime, unsigned long ServerFrameNr);
        void Draw(bool FirstPersonView, float LodDist) const;
        void PostDraw(float FrameTime, bool FirstPersonView);


        const cf::TypeSys::TypeInfoT* GetType() const;
        static void* CreateInstance(const cf::TypeSys::CreateParamsT& Params);
        static const cf::TypeSys::TypeInfoT TypeInfo;


        private:

        enum FlightStateT { CruiseFlight, ControlledCruise, HalfLoopAndRoll, ClimpBackToCruiseAlt };

        const CafuModelT* m_Model;
        FlightStateT      FlightState;
        VectorT           OldOrigin;
        VectorT           LoopCenter;
        float             FigureDistance;
        float             FigureLeft;

        int               m_ModelSequNr;
        float             m_ModelFrameNr;
        float             m_TimeUntilNextCry;
        SoundI*           m_EagleCry;
    };
}

#endif
