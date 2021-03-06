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

#ifndef CAFU_COMMAND_HISTORY_HPP_INCLUDED
#define CAFU_COMMAND_HISTORY_HPP_INCLUDED

#include "CommandPattern.hpp"


class CommandHistoryT
{
    public:

    CommandHistoryT();
    ~CommandHistoryT();

    bool SubmitCommand(CommandT* Command);

    void Undo();
    void Redo();

    const CommandT* GetUndoCommand() const;     ///< Returns the next command in the history that can be undone, or NULL if there is none. Only commands that are shown in history are taken into account.
    const CommandT* GetRedoCommand() const;     ///< Returns the next command in the history that can be redone, or NULL if there is none. Only commands that are shown in history are taken into account.

    unsigned long GetLastSaveSuggestedCommandID() const;


    private:

    ArrayT<CommandT*> m_Commands;
    ArrayT<CommandT*> m_InvisCommands;    ///< Stores all commands not visible in the history until a visible command is added to the history (then they are moved into the normal history).
    int               m_CurrentIndex;     ///< The index of the last done command: all commands in <code>m_Commands[0 ... m_CurrentIndex]</code> are "done" (available for undo), any commands following them are "undone" (available for redo). If m_CurrentIndex is -1, there are no "done" commands at all.
    bool              m_IsRecursiveCall;  ///< In order to facilitate debugging, this member helps with detecting recursive calls to our functions. For example, when we call a commands CommandT::Undo() method, does it erroneously cause a "recursive" call back to SubmitCommand()?

    /// The command id returned when there is no current command (when the current index is -1).
    /// On creation this value is 0. It becomes the command ID of the last command removed from the history due to
    /// size limitations as set in the CaWE options by the user.
    unsigned long     m_InvalidCommandID;
};

#endif
