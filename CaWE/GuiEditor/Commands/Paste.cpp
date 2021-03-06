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

#include "Paste.hpp"
#include "../GuiDocument.hpp"
#include "../Windows/EditorWindow.hpp"


using namespace GuiEditor;


CommandPasteT::CommandPasteT(GuiDocumentT* GuiDocument, const ArrayT< IntrusivePtrT<cf::GuiSys::WindowT> >& Windows, IntrusivePtrT<cf::GuiSys::WindowT> NewParent)
    : m_GuiDocument(GuiDocument),
      m_NewParent(NewParent)
{
    // Have to clone the windows here, so the command is the owner of these windows.
    for (unsigned long WinNr=0; WinNr<Windows.Size(); WinNr++)
    {
        m_Windows.PushBack(Windows[WinNr]->Clone(true));

        m_Windows[WinNr]->Rect[0]=(WinNr + 1) * 20.0f;
        m_Windows[WinNr]->Rect[1]=(WinNr + 1) * 10.0f;

        // Create editor data for the window itself and all of its children.
        GuiDocumentT::CreateSibling(m_Windows[WinNr], m_GuiDocument);

        ArrayT< IntrusivePtrT<cf::GuiSys::WindowT> > Children;
        m_Windows[WinNr]->GetChildren(Children, true);

        for (unsigned long ChildNr=0; ChildNr<Children.Size(); ChildNr++)
            GuiDocumentT::CreateSibling(Children[ChildNr], m_GuiDocument);
    }
}


bool CommandPasteT::Do()
{
    wxASSERT(!m_Done);
    if (m_Done) return false;

    for (unsigned long WinNr=0; WinNr<m_Windows.Size(); WinNr++)
        if (&m_Windows[WinNr]->GetGui() != m_GuiDocument->GetGui())
        {
            wxMessageBox("Sorry, cannot copy windows from one GUI document and paste them into another at this time.");
            return false;
        }

    for (unsigned long WinNr=0; WinNr<m_Windows.Size(); WinNr++)
    {
        m_Windows[WinNr]->m_Parent=m_NewParent;
        m_NewParent->m_Children.PushBack(m_Windows[WinNr]);
    }

    m_GuiDocument->UpdateAllObservers_Created(m_Windows);

    m_Done=true;
    return true;
}


void CommandPasteT::Undo()
{
    wxASSERT(m_Done);
    if (!m_Done) return;

    for (unsigned long WinNr=0; WinNr<m_Windows.Size(); WinNr++)
    {
        m_Windows[WinNr]->m_Parent=NULL;

        m_NewParent->m_Children.RemoveAtAndKeepOrder(m_NewParent->m_Children.Find(m_Windows[WinNr]));
    }

    m_GuiDocument->UpdateAllObservers_Deleted(m_Windows);

    m_Done=false;
}


wxString CommandPasteT::GetName() const
{
    if (m_Windows.Size()>1)
        return "Paste windows";
    else
        return "Paste window";
}
