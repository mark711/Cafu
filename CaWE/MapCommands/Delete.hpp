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

#ifndef CAFU_COMMAND_DELETE_HPP_INCLUDED
#define CAFU_COMMAND_DELETE_HPP_INCLUDED

#include "../CommandPattern.hpp"


class MapDocumentT;
class MapElementT;
class MapEntityT;
class MapEntityBaseT;
class MapPrimitiveT;
class CommandSelectT;


class CommandDeleteT : public CommandT
{
    public:

    /// Constructor to delete an individual object.
    CommandDeleteT(MapDocumentT& MapDoc, MapElementT* DeleteElem);

    /// Constructor to delete an array of objects.
    CommandDeleteT(MapDocumentT& MapDoc, const ArrayT<MapElementT*>& DeleteElems);

    /// Destructor.
    ~CommandDeleteT();

    // Implementation of the CommandT interface.
    bool Do();
    void Undo();
    wxString GetName() const;


    private:

    void Init(const ArrayT<MapElementT*>& DeleteElems);

    MapDocumentT&           m_MapDoc;
    ArrayT<MapEntityT*>     m_DeleteEnts;           ///< The entities to delete.
    ArrayT<MapPrimitiveT*>  m_DeletePrims;          ///< The primitives to delete.
    ArrayT<MapEntityBaseT*> m_DeletePrimsParents;   ///< The parents of the above primitives (the world or any custom entity).
    CommandSelectT*         m_CommandSelect;        ///< The command that unselects all elements before they are deleted.
};

#endif
