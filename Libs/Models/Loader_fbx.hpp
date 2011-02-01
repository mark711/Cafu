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

#ifndef _FBX_MODEL_LOADER_HPP_
#define _FBX_MODEL_LOADER_HPP_

#include "Loader.hpp"


/// This class uses the Autodesk FBX SDK in order to load a model file into a new Cafu model.
class LoaderFbxT : public ModelLoaderT
{
    public:

    /// The constructor for loading an Autodesk FBX (.fbx) model file into a new Cafu model.
    /// @param FileName   The name of the file to load.
    LoaderFbxT(const std::string& FileName) /*throw (ModelT::LoadError)*/;

    /// The destructor.
    ~LoaderFbxT();

    bool UseGivenTS() const;
    void Load(ArrayT<CafuModelT::JointT>& Joints, ArrayT<CafuModelT::MeshT>& Meshes, ArrayT<CafuModelT::AnimT>& Anims);
    void Load(ArrayT<CafuModelT::GuiLocT>& GuiLocs);


    private:

    LoaderFbxT(const LoaderFbxT&);          ///< Use of the Copy Constructor    is not allowed.
    void operator = (const LoaderFbxT&);    ///< Use of the Assignment Operator is not allowed.

    class FbxSceneT;
    FbxSceneT* m_FbxScene;    ///< We use the PIMPL idiom because we cannot forward-declare the FBX SDK classes without hard-wiring the version dependent name of their namespace.
};

#endif
