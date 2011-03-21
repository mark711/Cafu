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

#include "ConsoleFile.hpp"


using namespace cf;


ConsoleFileT::ConsoleFileT(const std::string& FileName)
    : m_File(FileName.c_str()),
      m_AutoFlush(false)
{
}


void ConsoleFileT::Flush()
{
    m_File.flush();
}


void ConsoleFileT::Print(const std::string& s)
{
    m_File << s;
    if (m_AutoFlush) Flush();
}


void ConsoleFileT::DevPrint(const std::string& s)
{
    m_File << "[Dev] " << s;
    if (m_AutoFlush) Flush();
}


void ConsoleFileT::Warning(const std::string& s)
{
    m_File << "Warning: " << s;
    if (m_AutoFlush) Flush();
}


void ConsoleFileT::DevWarning(const std::string& s)
{
    m_File << "[Dev] Warning: " << s;
    if (m_AutoFlush) Flush();
}
