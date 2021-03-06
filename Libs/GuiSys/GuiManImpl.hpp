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

#ifndef CAFU_GUISYS_GUIMAN_IMPL_HPP_INCLUDED
#define CAFU_GUISYS_GUIMAN_IMPL_HPP_INCLUDED

#include "GuiMan.hpp"
#include "Templates/Array.hpp"


namespace cf
{
    namespace GuiSys
    {
        class GuiResourcesT;


        /// This class implements the GuiManI interface.
        class GuiManImplT : public GuiManI
        {
            public:

            /// The constructor.
            /// The MatSys *must* be initialized *before* this constructor is called (i.e. a GuiManImplT is instantiated)!
            GuiManImplT(GuiResourcesT& GuiRes);

            /// The destructor.
            ~GuiManImplT();


            // Implement all the (pure) virtual methods of the GuiManI interface.
            GuiI* Register(const std::string& GuiScriptName);
            GuiI* Register(GuiI* NewGui);
            void Free(GuiI* Gui);
            GuiI* Find(const std::string& GuiScriptName, bool AutoRegister=false);
            void BringToFront(GuiI* Gui);
            GuiI* GetTopmostActiveAndInteractive();
            void ReloadAllGuis();
            void RenderAll();
            void ProcessDeviceEvent(const CaKeyboardEventT& KE);
            void ProcessDeviceEvent(const CaMouseEventT& ME);
            void DistributeClockTickEvents(float t);


            private:

            GuiManImplT(const GuiManImplT&);            ///< Use of the Copy Constructor    is not allowed.
            void operator = (const GuiManImplT&);       ///< Use of the Assignment Operator is not allowed.

            GuiResourcesT& m_GuiResources;      ///< The provider for resources (fonts and models) that are used in the GUIs created by this GuiMan.
            ArrayT<GuiI*>  Guis;
            bool           SuppressNextChar;    ///< Whether the next character (CaKeyboardEventT::CKE_CHAR) event should be suppressed. This is true whenever the preceeding CaKeyboardEventT::CKE_KEYDOWN event was positively processed.
        };
    }
}

#endif
