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

#include "EntityCreateParams.hpp"

using namespace GAME_NAME;


EntityCreateParamsT::EntityCreateParamsT(
    const unsigned long                       ID_,
    const std::map<std::string, std::string>& Properties_,
    const cf::SceneGraph::GenericNodeT*       RootNode_,
    const cf::ClipSys::CollisionModelT*       CollisionModel_,
    const unsigned long                       WorldFileIndex_,
    const unsigned long                       MapFileIndex_,
    cf::GameSys::GameWorldI*                  GameWorld_,
    const Vector3dT&                          Origin_)
    : ID(ID_),
      Properties(Properties_),
      RootNode(RootNode_),
      CollisionModel(CollisionModel_),
      WorldFileIndex(WorldFileIndex_),
      MapFileIndex(MapFileIndex_),
      GameWorld(GameWorld_),
      Origin(Origin_)
{
}
