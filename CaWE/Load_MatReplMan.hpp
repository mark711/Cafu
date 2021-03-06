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

#ifndef CAFU_LOAD_MATERIAL_REPL_MAN_HPP_INCLUDED
#define CAFU_LOAD_MATERIAL_REPL_MAN_HPP_INCLUDED

#include "wx/string.h"
#include "Templates/Array.hpp"


class EditorMaterialI;


/// This class manages the replacement of materials in imported maps with native Cafu materials.
/// Materials in the imported map file are replaced by Cafu materials listed in MaterialsReplacePool,
/// except those that contain a string that is mentioned in the ExceptionList.
class MatReplaceManT
{
    public:

    MatReplaceManT(const wxString& OtherGameName_, const ArrayT<EditorMaterialI*>& Materials_);

    wxString GetReplacement(const wxString& D3MaterialName);


    private:

    const wxString   OtherGameName;
    wxString         MaterialPrefix;
    ArrayT<wxString> SubstitutePool;
    ArrayT<wxString> ExceptionList;
    unsigned long    SubstitutePool_NextNr;
    ArrayT<wxString> AlreadyReplaced_OldName;
    ArrayT<wxString> AlreadyReplaced_NewName;
};

#endif
