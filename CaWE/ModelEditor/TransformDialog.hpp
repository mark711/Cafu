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

#ifndef CAFU_MODELEDITOR_TRANSFORM_DIALOG_HPP_INCLUDED
#define CAFU_MODELEDITOR_TRANSFORM_DIALOG_HPP_INCLUDED

#include "ObserverPattern.hpp"
#include "wx/wx.h"


class wxNotebook;


namespace ModelEditor
{
    class ChildFrameT;
    class ModelDocumentT;

    class TransformDialogT : public wxPanel, public ObserverT
    {
        public:

        TransformDialogT(ChildFrameT* Parent, const wxSize& Size);
        ~TransformDialogT();

        // ObserverT implementation.
        void Notify_SelectionChanged(SubjectT* Subject, ModelElementTypeT Type, const ArrayT<unsigned int>& OldSel, const ArrayT<unsigned int>& NewSel);
        void Notify_SubjectDies(SubjectT* dyingSubject);


        private:

        /// IDs for the controls whose events we are interested in.
        enum
        {
            ID_BUTTON_RESET=wxID_HIGHEST+1,
            ID_BUTTON_APPLY
        };

        void OnButton(wxCommandEvent& Event);

        ModelDocumentT* m_ModelDoc;
        ChildFrameT*    m_Parent;
        wxNotebook*     m_Notebook;

        float           m_Values[3][3];     ///< Translate, rotate and scale: three values each.

        DECLARE_EVENT_TABLE()
    };
}

#endif
