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

#ifndef CAFU_GUIEDITOR_COMMAND_SCALE_HPP_INCLUDED
#define CAFU_GUIEDITOR_COMMAND_SCALE_HPP_INCLUDED

#include "../../CommandPattern.hpp"

#include "Templates/Array.hpp"
#include "Templates/Pointer.hpp"
#include "Math3D/Vector3.hpp"


namespace cf { namespace GuiSys { class WindowT; } }


namespace GuiEditor
{
    class GuiDocumentT;

    class CommandScaleT : public CommandT
    {
        public:

        /// Scales a set of windows.
        /// @param GuiDocument The GUI document the windows are from.
        /// @param Windows The windows to scale.
        /// @param Positions Depending on parameter Done these are the positions where the windows should be moved or the original
        ///                  window positions (since window origin is the top left corner of the window, scaling may change positions).
        /// @param Sizes Depending on parameter Done these are the sizes the windows should be scaled to or the original window sizes.
        /// @param Done Determines whether the windows have already been scaled or should be scaled by the command on first do.
        CommandScaleT(GuiDocumentT* GuiDocument, const ArrayT< IntrusivePtrT<cf::GuiSys::WindowT> >& Windows, const ArrayT<Vector3fT>& Positions, const ArrayT<Vector3fT>& Sizes, bool Done=false);

        // CommandT implementation.
        bool Do();
        void Undo();
        wxString GetName() const;


        private:

        GuiDocumentT* m_GuiDocument;

        const ArrayT< IntrusivePtrT<cf::GuiSys::WindowT> > m_Windows;

        ArrayT<Vector3fT> m_OldPositions;
        ArrayT<Vector3fT> m_NewPositions;
        ArrayT<Vector3fT> m_OldSizes;
        ArrayT<Vector3fT> m_NewSizes;
    };
}

#endif
