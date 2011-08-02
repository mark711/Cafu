/*
=================================================================================
This file is part of Cafu, the open-source game engine and graphics engine
for multiplayer, cross-platform, real-time 3D action.
Copyright (C) 2002-2011 Carsten Fuchs Software.

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

#include "ElementsList.hpp"
#include "ChildFrame.hpp"
#include "ModelDocument.hpp"
#include "Commands/Add.hpp"
#include "Commands/Delete.hpp"
#include "Commands/Rename.hpp"
#include "Commands/Select.hpp"
#include "../ArtProvider.hpp"

#include "MaterialSystem/Material.hpp"
#include "Models/Model_cmdl.hpp"


using namespace ModelEditor;


namespace
{
    class ListContextMenuT : public wxMenu
    {
        public:

        enum
        {
            ID_MENU_INSPECT_EDIT=wxID_HIGHEST+1,
            ID_MENU_RENAME,
            ID_MENU_ADD_NEW
        };

        ListContextMenuT(ModelElementTypeT Type) : wxMenu(), ID(-1)
        {
            Append(ID_MENU_INSPECT_EDIT, "Inspect / Edit\tEnter");
            Append(ID_MENU_RENAME,       "Rename\tF2");

            if (Type==GFIX) Append(ID_MENU_ADD_NEW, "Add/create new");

            /* if (Type==MESH)
            {
                Append(..., "Remove unused Vertices");
                Append(..., "Remove unused Weights");
            } */
        }

        int GetClickedMenuItem() { return ID; }


        protected:

        void OnMenuClick(wxCommandEvent& CE) { ID=CE.GetId(); }


        private:

        int ID;

        DECLARE_EVENT_TABLE()
    };


    BEGIN_EVENT_TABLE(ListContextMenuT, wxMenu)
        EVT_MENU(wxID_ANY, ListContextMenuT::OnMenuClick)
    END_EVENT_TABLE()
}


BEGIN_EVENT_TABLE(ElementsListT, wxListView)
    EVT_SET_FOCUS           (ElementsListT::OnFocus)
    EVT_CONTEXT_MENU        (ElementsListT::OnContextMenu)
    EVT_LIST_KEY_DOWN       (wxID_ANY, ElementsListT::OnKeyDown)
    EVT_LIST_ITEM_ACTIVATED (wxID_ANY, ElementsListT::OnItemActivated)
    EVT_LIST_ITEM_SELECTED  (wxID_ANY, ElementsListT::OnSelectionChanged)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, ElementsListT::OnSelectionChanged)
    EVT_LIST_END_LABEL_EDIT (wxID_ANY, ElementsListT::OnEndLabelEdit)
END_EVENT_TABLE()


ElementsListT::ElementsListT(ChildFrameT* MainFrame, wxWindow* Parent, const wxSize& Size, ModelElementTypeT Type)
    : wxListView(Parent, wxID_ANY, wxDefaultPosition, Size, wxLC_REPORT | wxLC_EDIT_LABELS),
      m_TYPE(Type),
      m_ModelDoc(MainFrame->GetModelDoc()),
      m_MainFrame(MainFrame),
      m_IsRecursiveSelfNotify(false)
{
    wxASSERT(m_TYPE==ANIM || m_TYPE==MESH || m_TYPE==GFIX);

    // TODO: Make it up to the caller code to call this?
    // // As we are now a wxAUI pane rather than a wxDialog, explicitly set that events are not propagated to our parent.
    // SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    InsertColumn(0, "Name");
    InsertColumn(1, "#");
    if (m_TYPE==MESH) InsertColumn(2, "Material");

    m_ModelDoc->RegisterObserver(this);
    InitListItems();
}


ElementsListT::~ElementsListT()
{
    if (m_ModelDoc)
        m_ModelDoc->UnregisterObserver(this);
}


void ElementsListT::Notify_SelectionChanged(SubjectT* Subject, ModelElementTypeT Type, const ArrayT<unsigned int>& OldSel, const ArrayT<unsigned int>& NewSel)
{
    if (m_IsRecursiveSelfNotify) return;
    if (Type!=m_TYPE) return;

    m_IsRecursiveSelfNotify=true;
    Freeze();

    for (long SelNr=GetFirstSelected(); SelNr!=-1; SelNr=GetNextSelected(SelNr))
        Select(SelNr, false);

    for (unsigned long SelNr=0; SelNr<NewSel.Size(); SelNr++)
        Select(NewSel[SelNr]);

    Thaw();
    m_IsRecursiveSelfNotify=false;
}


void ElementsListT::Notify_Created(SubjectT* Subject, ModelElementTypeT Type, const ArrayT<unsigned int>& Indices)
{
    if (m_IsRecursiveSelfNotify) return;
    if (Type!=m_TYPE) return;

    InitListItems();
}


void ElementsListT::Notify_Deleted(SubjectT* Subject, ModelElementTypeT Type, const ArrayT<unsigned int>& Indices)
{
    if (m_IsRecursiveSelfNotify) return;
    if (Type!=m_TYPE) return;

    InitListItems();
}


void ElementsListT::Notify_MeshChanged(SubjectT* Subject, unsigned int MeshNr)
{
    if (m_IsRecursiveSelfNotify) return;
    if (m_TYPE!=MESH) return;

    InitListItems();
}


void ElementsListT::Notify_AnimChanged(SubjectT* Subject, unsigned int AnimNr)
{
    if (m_IsRecursiveSelfNotify) return;
    if (m_TYPE!=ANIM) return;

    InitListItems();
}


void ElementsListT::Notify_GuiFixtureChanged(SubjectT* Subject, unsigned int GuiFixtureNr)
{
    if (m_IsRecursiveSelfNotify) return;
    if (m_TYPE!=GFIX) return;

    InitListItems();
}


void ElementsListT::Notify_SubjectDies(SubjectT* dyingSubject)
{
    wxASSERT(dyingSubject==m_ModelDoc);

    m_ModelDoc=NULL;

    DeleteAllItems();
}


void ElementsListT::InitListItems()
{
    const ArrayT<unsigned int>& Sel=m_ModelDoc->GetSelection(m_TYPE);

    Freeze();
    DeleteAllItems();

    switch (m_TYPE)
    {
        case JOINT:
            wxASSERT(false);
            break;

        case MESH:
            for (unsigned long ElemNr=0; ElemNr<m_ModelDoc->GetModel()->GetMeshes().Size(); ElemNr++)
            {
                InsertItem(ElemNr, m_ModelDoc->GetModel()->GetMeshes()[ElemNr].Name);
                SetItem(ElemNr, 1, wxString::Format("%lu", ElemNr));
                SetItem(ElemNr, 2, m_ModelDoc->GetModel()->GetMeshes()[ElemNr].Material->Name);

                if (Sel.Find(ElemNr)!=-1) Select(ElemNr);
            }
            break;

        case ANIM:
            for (unsigned long ElemNr=0; ElemNr<m_ModelDoc->GetModel()->GetAnims().Size(); ElemNr++)
            {
                InsertItem(ElemNr, m_ModelDoc->GetModel()->GetAnims()[ElemNr].Name);
                SetItem(ElemNr, 1, wxString::Format("%lu", ElemNr));

                if (Sel.Find(ElemNr)!=-1) Select(ElemNr);
            }
            break;

        case GFIX:
            for (unsigned long ElemNr=0; ElemNr<m_ModelDoc->GetModel()->GetGuiFixtures().Size(); ElemNr++)
            {
                InsertItem(ElemNr, m_ModelDoc->GetModel()->GetGuiFixtures()[ElemNr].Name);
                SetItem(ElemNr, 1, wxString::Format("%lu", ElemNr));

                if (Sel.Find(ElemNr)!=-1) Select(ElemNr);
            }
            break;
    }

    // Set the widths of the columns to the width of their longest item.
    if (GetItemCount()>0)
        for (int ColNr=0; ColNr<GetColumnCount(); ColNr++)
            SetColumnWidth(ColNr, wxLIST_AUTOSIZE);

    Thaw();
}


void ElementsListT::OnFocus(wxFocusEvent& FE)
{
    m_MainFrame->SetLastUsedType(m_TYPE);
    FE.Skip();
}


void ElementsListT::OnContextMenu(wxContextMenuEvent& CE)
{
    ListContextMenuT ContextMenu(m_TYPE);

    PopupMenu(&ContextMenu);

    switch (ContextMenu.GetClickedMenuItem())
    {
        case ListContextMenuT::ID_MENU_INSPECT_EDIT:
            // Make sure that the AUI pane for the inspector related to this elements list is shown.
            m_MainFrame->ShowRelatedInspector(GetParent());
            break;

        case ListContextMenuT::ID_MENU_RENAME:
        {
            const long SelNr=GetFirstSelected();

            if (SelNr!=-1) EditLabel(SelNr);
            break;
        }

        case ListContextMenuT::ID_MENU_ADD_NEW:
        {
            if (m_TYPE==GFIX)
            {
                ArrayT<CafuModelT::GuiFixtureT> GuiFixtures;

                GuiFixtures.PushBackEmpty();
                GuiFixtures[0].Name="New GUI Fixture";

                m_MainFrame->SubmitCommand(new CommandAddT(m_ModelDoc, GuiFixtures));
            }
            break;
        }
    }
}


void ElementsListT::OnKeyDown(wxListEvent& LE)
{
    switch (LE.GetKeyCode())
    {
        case WXK_F2:
        {
            const long SelNr=LE.GetIndex();

            if (SelNr!=-1) EditLabel(SelNr);
            break;
        }

        default:
            LE.Skip();
            break;
    }
}


void ElementsListT::OnItemActivated(wxListEvent& LE)
{
    // This is called when the item has been activated (ENTER or double click).
    if (m_ModelDoc==NULL) return;

    // Make sure that the AUI pane for the inspector related to this elements list is shown.
    m_MainFrame->ShowRelatedInspector(GetParent());
}


void ElementsListT::OnSelectionChanged(wxListEvent& LE)
{
    if (m_ModelDoc==NULL) return;
    if (m_IsRecursiveSelfNotify) return;

    m_IsRecursiveSelfNotify=true;

    // Get the currently selected list items and update the document selection accordingly.
    ArrayT<unsigned int> NewSel;

    for (long SelNr=GetFirstSelected(); SelNr!=-1; SelNr=GetNextSelected(SelNr))
        NewSel.PushBack(SelNr);

    m_MainFrame->SubmitCommand(CommandSelectT::Set(m_ModelDoc, m_TYPE, NewSel));

    m_IsRecursiveSelfNotify=false;
}


void ElementsListT::OnEndLabelEdit(wxListEvent& LE)
{
    const unsigned int Index=LE.GetIndex();

    if (LE.IsEditCancelled()) return;

    m_IsRecursiveSelfNotify=true;
    m_MainFrame->SubmitCommand(new CommandRenameT(m_ModelDoc, m_TYPE, Index, LE.GetLabel()));
    m_IsRecursiveSelfNotify=false;
}


BEGIN_EVENT_TABLE(ElementsPanelT, wxPanel)
    EVT_BUTTON(ID_BUTTON_ADD,    ElementsPanelT::OnButton)
    EVT_BUTTON(ID_BUTTON_UP,     ElementsPanelT::OnButton)
    EVT_BUTTON(ID_BUTTON_DOWN,   ElementsPanelT::OnButton)
    EVT_BUTTON(ID_BUTTON_DELETE, ElementsPanelT::OnButton)
    EVT_UPDATE_UI_RANGE(ID_BUTTON_ADD, ID_BUTTON_DELETE, ElementsPanelT::OnButtonUpdate)
END_EVENT_TABLE()


ElementsPanelT::ElementsPanelT(ChildFrameT* MainFrame, const wxSize& Size, ModelElementTypeT Type)
    : wxPanel(MainFrame, -1, wxDefaultPosition, Size),
      m_TYPE(Type),
      m_ModelDoc(MainFrame->GetModelDoc()),
      m_MainFrame(MainFrame),
      m_List(NULL)
{
    // As we are a wxAUI pane rather than a wxDialog, explicitly set that events are not propagated to our parent.
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    wxBoxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *item7 = new wxBoxSizer( wxHORIZONTAL );

    wxButton* button1 = new wxButton(this, ID_BUTTON_ADD, wxT("add"), wxDefaultPosition, wxSize(22, -1), wxBU_EXACTFIT | wxBU_NOTEXT );
    button1->SetBitmap(wxArtProvider::GetBitmap("list-add", wxART_BUTTON));
    item7->Add(button1, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );

    wxButton* button2 = new wxButton(this, ID_BUTTON_UP, wxT("up"), wxDefaultPosition, wxSize(22, -1), wxBU_EXACTFIT | wxBU_NOTEXT );
    button2->SetBitmap(wxArtProvider::GetBitmap("list-selection-up", wxART_BUTTON));
    item7->Add(button2, 1, wxALIGN_CENTER|wxRIGHT, 5 );

    wxButton* button3 = new wxButton(this, ID_BUTTON_DOWN, wxT("down"), wxDefaultPosition, wxSize(22, -1), wxBU_EXACTFIT | wxBU_NOTEXT );
    button3->SetBitmap(wxArtProvider::GetBitmap("list-selection-down", wxART_BUTTON));
    item7->Add(button3, 1, wxALIGN_CENTER|wxRIGHT, 5 );

    wxButton* button4 = new wxButton(this, ID_BUTTON_DELETE, wxT("del"), wxDefaultPosition, wxSize(22, -1), wxBU_EXACTFIT | wxBU_NOTEXT );
    button4->SetBitmap(wxArtProvider::GetBitmap("list-remove", wxART_BUTTON));
    item7->Add(button4, 1, wxALIGN_CENTER|wxRIGHT, 5 );

    item0->Add( item7, 0, wxEXPAND | wxTOP | wxBOTTOM, 3 );

    m_List=new ElementsListT(MainFrame, this, /*ID_LISTVIEW,*/ wxDefaultSize, Type);
    item0->Add(m_List, 1, wxEXPAND, 0 );

    this->SetSizer( item0 );
    item0->SetSizeHints(this);
}


void ElementsPanelT::OnButton(wxCommandEvent& Event)
{
    switch (Event.GetId())
    {
        case ID_BUTTON_ADD:
        {
            if (m_TYPE==GFIX)
            {
                ArrayT<CafuModelT::GuiFixtureT> GuiFixtures;

                GuiFixtures.PushBackEmpty();
                GuiFixtures[0].Name="New GUI Fixture";

                m_MainFrame->SubmitCommand(new CommandAddT(m_ModelDoc, GuiFixtures));
            }
            break;
        }

        case ID_BUTTON_DELETE:
        {
            CommandDeleteT* DelCmd=new CommandDeleteT(m_ModelDoc, m_TYPE, m_ModelDoc->GetSelection(m_TYPE));
            bool            Result=DelCmd->Do();

            if (DelCmd->GetMessage()!="") wxMessageBox(DelCmd->GetMessage(), "Delete");
            if (Result) m_MainFrame->SubmitCommand(DelCmd); else delete DelCmd;
            break;
        }
    }
}


void ElementsPanelT::OnButtonUpdate(wxUpdateUIEvent& UE)
{
    switch (UE.GetId())
    {
        case ID_BUTTON_ADD:
        {
            UE.Enable(m_TYPE==GFIX);
            break;
        }

        case ID_BUTTON_UP:
        {
            UE.Enable(false);
            break;
        }

        case ID_BUTTON_DOWN:
        {
            UE.Enable(false);
            break;
        }

        case ID_BUTTON_DELETE:
        {
            UE.Enable(m_ModelDoc->GetSelection(m_TYPE).Size()>0);
            break;
        }
    }
}
