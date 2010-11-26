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

#include "Loader_assimp.hpp"
#include "MaterialSystem/Renderer.hpp"

#include "assimp.hpp"       // C++ importer interface
#include "aiScene.h"        // Output data structure
#include "aiPostProcess.h"  // Post processing flags


LoaderAssimpT::LoaderAssimpT(const std::string& FileName) /*throw (ModelT::LoadError)*/
    : ModelLoaderT(FileName)
{
}


bool LoaderAssimpT::UseGivenTS() const
{
    // TODO...
    return false;
}


void LoaderAssimpT::Load(ArrayT<CafuModelT::JointT>& Joints, ArrayT<CafuModelT::MeshT>& Meshes, ArrayT<CafuModelT::AnimT>& Anims)
{
    Assimp::Importer Importer;

    const aiScene* Scene=Importer.ReadFile(m_FileName,
        aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!Scene)
        throw LoadErrorT(std::string("Asset Import: ") + Importer.GetErrorString());

}