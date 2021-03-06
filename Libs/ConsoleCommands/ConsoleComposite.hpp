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

#ifndef CAFU_COMPOSITE_CONSOLE_HPP_INCLUDED
#define CAFU_COMPOSITE_CONSOLE_HPP_INCLUDED

#include "Console.hpp"
#include "Templates/Array.hpp"


namespace cf
{
    /// This class implements the ConsoleI interface as a composite console
    /// by sending all output to its attached sub-consoles.
    class CompositeConsoleT : public ConsoleI
    {
        public:

        /// Constructor for creating a composite console.
        CompositeConsoleT();

        /// Attaches the given console c to the set of sub-consoles.
        bool Attach(ConsoleI* c);

        /// Removes the given console c from the set of sub-consoles.
        bool Detach(ConsoleI* c);

        // Methods of the ConsoleI interface.
        void Print(const std::string& s);
        void DevPrint(const std::string& s);
        void Warning(const std::string& s);
        void DevWarning(const std::string& s);


        private:

        ArrayT<ConsoleI*> m_Consoles;   ///< The filestream output is logged to.
    };
}

#endif
