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

#ifndef CAFU_COMMAND_MODIFY_MODEL_HPP_INCLUDED
#define CAFU_COMMAND_MODIFY_MODEL_HPP_INCLUDED

#include "../CommandPattern.hpp"
#include "Models/AnimExpr.hpp"


class MapDocumentT;
class MapModelT;


class CommandModifyModelT : public CommandT
{
    public:

    CommandModifyModelT(MapDocumentT& MapDoc, MapModelT* Model, const wxString& ModelFileName,
        const wxString& CollisionModelFileName, const wxString& Label, float Scale,
        IntrusivePtrT<AnimExprStandardT> AnimExpr, float FrameTimeOff, float FrameTimeScale, bool Animated);

    // CommandT implementation.
    bool Do();
    void Undo();
    wxString GetName() const;


    private:

    MapDocumentT&                    m_MapDoc;
    MapModelT*                       m_Model;
    const wxString                   m_NewModelFileName;
    const wxString                   m_OldModelFileName;
    const wxString                   m_NewCollModelFileName;
    const wxString                   m_OldCollModelFileName;
    const wxString                   m_NewLabel;
    const wxString                   m_OldLabel;
    float                            m_NewScale;
    float                            m_OldScale;
    IntrusivePtrT<AnimExprStandardT> m_NewAnimExpr;
    IntrusivePtrT<AnimExprStandardT> m_OldAnimExpr;
    float                            m_NewFrameTimeOff;
    float                            m_OldFrameTimeOff;
    float                            m_NewFrameTimeScale;
    float                            m_OldFrameTimeScale;
    bool                             m_NewAnimated;
    bool                             m_OldAnimated;
};

#endif
