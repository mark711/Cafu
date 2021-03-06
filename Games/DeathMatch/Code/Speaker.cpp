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

#include "Speaker.hpp"

#include "EntityCreateParams.hpp"
#include "TypeSys.hpp"
#include "Libs/LookupTables.hpp"

#include "Network/State.hpp"
#include "SoundSystem/SoundSys.hpp"
#include "SoundSystem/Sound.hpp"
#include "SoundSystem/SoundShaderManager.hpp"
#include "UniScriptState.hpp"

#include <iostream>

extern "C"
{
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

using namespace GAME_NAME;


// Implement the type info related code.
const cf::TypeSys::TypeInfoT* EntSpeakerT::GetType() const
{
    return &TypeInfo;
}

void* EntSpeakerT::CreateInstance(const cf::TypeSys::CreateParamsT& Params)
{
    return new EntSpeakerT(*static_cast<const EntityCreateParamsT*>(&Params));
}

// Create method list for scripting.
const luaL_Reg EntSpeakerT::MethodsList[]=
{
    { "Play",  EntSpeakerT::Play },
    { "Stop",  EntSpeakerT::Stop },
    { NULL, NULL }
};

const cf::TypeSys::TypeInfoT EntSpeakerT::TypeInfo(GetBaseEntTIM(), "EntSpeakerT", "BaseEntityT", EntSpeakerT::CreateInstance, MethodsList);


EntSpeakerT::EntSpeakerT(const EntityCreateParamsT& Params)
    : BaseEntityT(Params,
                  BoundingBox3dT(Vector3dT(0.0, 0.0, 0.0),
                                 Vector3dT(0.0, 0.0, 0.0)),
                  NUM_EVENT_TYPES),
      m_Interval(GetProp("interval", 0.0f)),
      m_TimeUntilNextSound(m_Interval),
      m_Sound(SoundSystem->CreateSound3D(SoundShaderManager->GetSoundShader(GetProp("soundshader", ""))))
{
    m_AutoPlay=GetProp("autoplay", 0) != 0; // Set initial playback state.

    m_Sound->SetInnerVolume   (GetProp("innerVolume",   0.5f));
    m_Sound->SetOuterVolume   (GetProp("outerVolume",   0.0f));
    m_Sound->SetInnerConeAngle(GetProp("innerCone",   360.0f));
    m_Sound->SetOuterConeAngle(GetProp("outerCone",   360.0f));
}


EntSpeakerT::~EntSpeakerT()
{
    SoundSystem->DeleteSound(m_Sound);
}


void EntSpeakerT::DoSerialize(cf::Network::OutStreamT& Stream) const
{
    Stream << m_AutoPlay;
}


void EntSpeakerT::DoDeserialize(cf::Network::InStreamT& Stream)
{
    Stream >> m_AutoPlay;
}


void EntSpeakerT::PostDraw(float FrameTime, bool FirstPersonView)
{
    m_Sound->SetPosition(m_Origin);

    const float ViewDirZ=-LookupTables::Angle16ToSin[m_Pitch];
    const float ViewDirY= LookupTables::Angle16ToCos[m_Pitch];

    const Vector3dT Direction(ViewDirY*LookupTables::Angle16ToSin[m_Heading], ViewDirY*LookupTables::Angle16ToCos[m_Heading], ViewDirZ);

    m_Sound->SetDirection(Direction);

    if (!m_AutoPlay || m_Interval==0.0f)
    {
        m_TimeUntilNextSound=m_Interval;
        return;
    }

    m_TimeUntilNextSound-=FrameTime;
    if (m_TimeUntilNextSound<0.0f)
    {
        m_TimeUntilNextSound=m_Interval;
        if (m_Interval<=0.0f) m_AutoPlay = false;   // Only play this sound once per trigger if interval is 0.0f.
        m_Sound->Play();
    }
}


void EntSpeakerT::ProcessEvent(unsigned int EventType, unsigned int /*NumEvents*/)
{
    switch (EventType)
    {
        case EVENT_TYPE_PLAY:
            m_Sound->Play();
            break;

        case EVENT_TYPE_STOP:
            m_Sound->Stop();
            break;
    }
}


int EntSpeakerT::Play(lua_State* LuaState)
{
    cf::ScriptBinderT Binder(LuaState);
    IntrusivePtrT<EntSpeakerT> Ent=Binder.GetCheckedObjectParam< IntrusivePtrT<EntSpeakerT> >(1);

    Ent->m_AutoPlay = true;
    Ent->PostEvent(EVENT_TYPE_PLAY);

    return 0;
}


int EntSpeakerT::Stop(lua_State* LuaState)
{
    cf::ScriptBinderT Binder(LuaState);
    IntrusivePtrT<EntSpeakerT> Ent=Binder.GetCheckedObjectParam< IntrusivePtrT<EntSpeakerT> >(1);

    Ent->m_AutoPlay = false;
    Ent->PostEvent(EVENT_TYPE_STOP);

    return 0;
}
