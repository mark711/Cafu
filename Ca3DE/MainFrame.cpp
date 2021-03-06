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

#include "MainFrame.hpp"
#include "AppCafu.hpp"
#include "MainCanvas.hpp"


BEGIN_EVENT_TABLE(MainFrameT, wxFrame)
    EVT_CLOSE(MainFrameT::OnClose)
END_EVENT_TABLE()


MainFrameT::MainFrameT(cf::GameSys::GameInfoI* GameInfo)
    : wxFrame(NULL /*parent*/, wxID_ANY, wxString("Cafu Engine - ") + __DATE__),
      m_MainCanvas(NULL)
{
#ifdef __WXMSW__
    SetIcon(wxIcon("aaaa", wxBITMAP_TYPE_ICO_RESOURCE));
#endif

    // Create sizer and insert canvas.
    wxBoxSizer* Sizer=new wxBoxSizer(wxHORIZONTAL);

    m_MainCanvas=new MainCanvasT(this, GameInfo);
    Sizer->Add(m_MainCanvas, 1, wxEXPAND, 0);

    this->SetSizer(Sizer);
    this->Layout();

    // Show the frame - it is not shown by default...
    if (wxGetApp().IsCustomVideoMode()) ShowFullScreen(true); else Show();
}


void MainFrameT::OnClose(wxCloseEvent& CE)
{
    if (!CE.CanVeto())
    {
        Destroy();
        return;
    }

    // All child frames were successfully closed.
    // It's safe now to also close this parent window, which will gracefully exit the application.
    Destroy();
}
