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

#ifndef _MODELEDITOR_SCENE_VIEW_3D_HPP_
#define _MODELEDITOR_SCENE_VIEW_3D_HPP_

#include "../Generic3DWindow.hpp"
#include "Renderer3D.hpp"


namespace MatSys { class RenderMaterialT; }


namespace ModelEditor
{
    class ChildFrameT;


    class SceneView3DT : public Generic3DWindowT
    {
        public:

        SceneView3DT(ChildFrameT* Parent);


        private:

        // Implement virtual methods of Generic3DViewT base class.
        virtual Vector3fT GetRefPtWorld(const wxPoint& RefPtWin) const;
        virtual void      InfoCameraChanged();
        virtual void      InfoRightMouseClick(wxMouseEvent& ME);

        /// Renders a single pass of the scene.
        void RenderPass() const;

        ChildFrameT*  m_Parent;
        Renderer3DT   m_Renderer;           ///< Performs the 3D rendering in our window.
        unsigned long m_TimeOfLastPaint;    ///< The time at which the OnPaint() event handler was last called.

        // Event handlers.
        void OnKeyDown      (wxKeyEvent&   KE);
        void OnMouseLeftDown(wxMouseEvent& ME); ///< We also handle "double-click" events in this method (use ME.ButtonDClick() for distinction).
        void OnMouseLeftUp  (wxMouseEvent& ME);
        void OnMouseMove    (wxMouseEvent& ME);
        void OnPaint        (wxPaintEvent& PE);
        void OnIdle         (wxIdleEvent&  IE);

        DECLARE_EVENT_TABLE()
    };
}

#endif
