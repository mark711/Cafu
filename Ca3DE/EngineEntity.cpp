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

#include "EngineEntity.hpp"
#include "NetConst.hpp"
#include "ConsoleCommands/ConVar.hpp"
#include "MaterialSystem/Renderer.hpp"
#include "Math3D/Matrix.hpp"
#include "Network/Network.hpp"
#include "Win32/Win32PrintHelp.hpp"
#include "Ca3DEWorld.hpp"


namespace
{
    ConVarT UsePrediction("usePrediction", true, ConVarT::FLAG_MAIN_EXE, "Toggles whether client prediction is used (recommended!).");
}


/******************/
/*** Both Sides ***/
/******************/


EngineEntityT::~EngineEntityT()
{
    // Remove this entity from the script state.
    cf::UniScriptStateT& ScriptState = Entity->GetGameWorld()->GetScriptState();
    lua_State*           LuaState    = ScriptState.GetLuaState();
    cf::ScriptBinderT    Binder(LuaState);

    if (Binder.IsBound(Entity.get()))
    {
        // _G[GameEntityI->GetName()] = nil
        lua_pushnil(LuaState);
        lua_setglobal(LuaState, Entity->GetName().c_str());

        Binder.Disconnect(Entity.get());
    }
}


IntrusivePtrT<GameEntityI> EngineEntityT::GetGameEntity() const
{
    return Entity;
}


void EngineEntityT::ProcessConfigString(const void* ConfigData, const char* ConfigString)
{
    Entity->ProcessConfigString(ConfigData, ConfigString);
}


cf::Network::StateT EngineEntityT::GetState() const
{
    cf::Network::StateT     State;
    cf::Network::OutStreamT Stream(State);

    Entity->Serialize(Stream);

    return State;
}


void EngineEntityT::SetState(const cf::Network::StateT& State, bool IsIniting) const
{
    cf::Network::InStreamT Stream(State);

    Entity->Deserialize(Stream, IsIniting);
}


/*******************/
/*** Server Side ***/
/*******************/


EngineEntityT::EngineEntityT(IntrusivePtrT<GameEntityI> Entity_, unsigned long CreationFrameNr)
    : Entity(Entity_),
      EntityStateFrameNr(CreationFrameNr),
      m_BaseLine(),
      BaseLineFrameNr(CreationFrameNr),
      m_OldStates()
{
    m_BaseLine=GetState();

    for (unsigned long OldStateNr=0; OldStateNr<16 /*MUST be a power of 2*/; OldStateNr++)
        m_OldStates.PushBack(m_BaseLine);

    // For entities that are named in the map file (e.g. "Soldier_Barney"),
    // assign the alter ego to a global variable in the map script with the same name.
    if (Entity->GetName() != "")
    {
        cf::UniScriptStateT& ScriptState = Entity->GetGameWorld()->GetScriptState();
        lua_State*           LuaState    = ScriptState.GetLuaState();
        cf::ScriptBinderT    Binder(LuaState);

        // lua_getglobal(LuaState, Entity->GetName().c_str());
        // if (!lua_isnil(LuaState, -1))
        //     Console->Warning("Global variable \""+Entity->GetName()+"\" already exists, overwriting...\n");
        // lua_pop(LuaState, 1);

        Binder.Push(Entity);
        lua_setglobal(LuaState, Entity->GetName().c_str());

        // Console->DevPrint("Info: Entity \""+Entity->GetName()+"\" of class \""+EntClassName+"\" (\""+CppClassName+"\") instantiated.\n");
    }
}


void EngineEntityT::PreThink(unsigned long ServerFrameNr)
{
    // 1. Ein Entity, der für dieses zu erstellende Frame 'ServerFrameNr' erst neu erzeugt wurde, soll nicht gleich denken!
    //    Ein einfacher Vergleich '==' wäre ausreichend, '>=' nur zur Sicherheit.
    //    Diese Zeile ist nur wg. "extern" erzeugten Entities (new-joined clients) hier.
    if (BaseLineFrameNr>=ServerFrameNr) return;

    // 2. Alten 'Entity->State' des vorherigen (aber noch aktuellen!) Server-Frames erstmal speichern.
    m_OldStates[(ServerFrameNr-1) & (m_OldStates.Size()-1)] = GetState();
}


void EngineEntityT::Think(float FrameTime, unsigned long ServerFrameNr)
{
    // x. Ein Entity, der für dieses zu erstellende Frame 'ServerFrameNr' erst neu erzeugt wurde, soll nicht gleich denken!
    //    Ein einfacher Vergleich '==' wäre ausreichend, '>=' nur zur Sicherheit.
    if (BaseLineFrameNr>=ServerFrameNr) return;

    // 3. Jetzt neuen 'Entity->State' ausdenken.
    Entity->Think(FrameTime, ServerFrameNr);
    EntityStateFrameNr=ServerFrameNr;
}


void EngineEntityT::WriteNewBaseLine(unsigned long SentClientBaseLineFrameNr, ArrayT< ArrayT<char> >& OutDatas) const
{
    // Nur dann etwas tun, wenn unsere 'BaseLineFrameNr' größer (d.h. jünger) als 'SentClientBaseLineFrameNr' ist,
    // d.h. unsere 'BaseLineFrameNr' noch nie / noch nicht an den Client gesendet wurde.
    if (SentClientBaseLineFrameNr>=BaseLineFrameNr) return;

    NetDataT NewBaseLineMsg;

    NewBaseLineMsg.WriteByte(SC1_EntityBaseLine);
    NewBaseLineMsg.WriteLong(Entity->GetID());
    NewBaseLineMsg.WriteLong(Entity->GetType()->TypeNr);
    NewBaseLineMsg.WriteLong(Entity->GetWorldFileIndex());
    NewBaseLineMsg.WriteDMsg(m_BaseLine.GetDeltaMessage(cf::Network::StateT() /*::ALL_ZEROS*/));

    OutDatas.PushBack(NewBaseLineMsg.Data);
}


bool EngineEntityT::WriteDeltaEntity(bool SendFromBaseLine, unsigned long ClientFrameNr, NetDataT& OutData, bool ForceInfo) const
{
    // Prüfe, ob die Voraussetzungen für die Parameter (insb. 'ClientFrameNr') eingehalten werden.
    if (!SendFromBaseLine)
    {
        // EntityStateFrameNr wird in Think() gesetzt und ist gleich der ServerFrameNr!
        // Beachte: OldStates speichert die alten Zustände von ServerFrameNr-1 bis ServerFrameNr-16.
        const unsigned long FrameDiff=EntityStateFrameNr-ClientFrameNr;

        if (FrameDiff<1 || FrameDiff>m_OldStates.Size()) return false;
    }


    const cf::Network::StateT CurrentState = GetState();
    const ArrayT<uint8_t>     DeltaMsg     = CurrentState.GetDeltaMessage(SendFromBaseLine ? m_BaseLine : m_OldStates[ClientFrameNr & (m_OldStates.Size()-1)]);

    if (cf::Network::StateT::IsDeltaMessageEmpty(DeltaMsg) && !ForceInfo) return true;

    // Write the SC1_EntityUpdate message
    OutData.WriteByte(SC1_EntityUpdate);
    OutData.WriteLong(Entity->GetID());
    OutData.WriteDMsg(DeltaMsg);

    return true;
}


#if !DEDICATED

/*******************/
/*** Client Side ***/
/*******************/


EngineEntityT::EngineEntityT(IntrusivePtrT<GameEntityI> Entity_, NetDataT& InData)
    : Entity(Entity_),
      EntityStateFrameNr(0),
      m_BaseLine(),
      BaseLineFrameNr(1234),
      m_OldStates()
{
    const cf::Network::StateT CurrentState(cf::Network::StateT() /*::ALL_ZEROS*/, InData.ReadDMsg());

    // Pass true for the IsInited parameter in order to indicate that we're constructing the entity.
    // This is done in order to have it not wrongly process the event counters.
    SetState(CurrentState, true);

    for (unsigned long OldStateNr=0; OldStateNr<32 /*MUST be a power of 2*/; OldStateNr++)
        m_OldStates.PushBack(CurrentState);

    m_BaseLine=CurrentState;
}


bool EngineEntityT::ParseServerDeltaUpdateMessage(unsigned long DeltaFrameNr, unsigned long ServerFrameNr, const ArrayT<uint8_t>* DeltaMessage)
{
    // Sanity-Check: Wir wollen, daß 'DeltaFrameNr<=EntityStateFrameNr<ServerFrameNr' gilt.
    // Wäre 'DeltaFrameNr>EntityStateFrameNr', so sollten wir gegen einen State dekomprimieren, der in der Zukunft liegt.
    // Wäre 'EntityStateFrameNr>=ServerFrameNr', so sollten wir uns in einen State begeben, der schon Vergangenheit ist.
    // Dies hält auch für den Spezialfall 'DeltaFrameNr==0' (Delta-Dekompression gegen die BaseLine).
    // Im Normalfall 'DeltaFrameNr>0' müssen wir unten außerdem noch sicherstellen, daß der DeltaState nicht zu weit in der Vergangenheit liegt.
    //
    // ONE possible reason for DeltaFrameNr>EntityStateFrameNr is related to the way how baselines are sent,
    // see EntityManager.cpp, EntityManagerT::ParseServerDeltaUpdateMessage() for a description, which is essentially repeated here:
    // When a client joins a level, there can be a LOT of entities. Usually, not all baselines of all entities fit into a single
    // realiable message at once, and thus the server sends them in batches, contained in subsequent realiable messages.
    // Between realiable messages however, the server sends also SC1_EntityUpdate messages.
    // These messages can already refer to entities that the client knows nothing about, because it has not yet seen the (reliable)
    // introductory baseline message.
    // Then, the entities that the client already knows about normally receive and process delta updates here in this function,
    // the others don't (because their non-presence is already detected in EntityManagerT::ParseServerDeltaUpdateMessage()).
    // However, the frame counters increase normally, as if all entities were present. When finally the remaining entities
    // arrive (because their baseline got finally through), these entities are likely to have DeltaFrameNr>EntityStateFrameNr.
    // I turn the "WARNING" into an "INFO", so that ordinary users get a better impression. ;)
    if (DeltaFrameNr>EntityStateFrameNr)   { EnqueueString("CLIENT INFO: %s, L %u: DeltaFrameNr>EntityStateFrameNr (%lu>%lu)\n"  , __FILE__, __LINE__, DeltaFrameNr, EntityStateFrameNr); return false; }
    if (EntityStateFrameNr>=ServerFrameNr) { EnqueueString("CLIENT WARNING: %s, L %u: EntityStateFrameNr>=ServerFrameNr (%lu>%lu)\n", __FILE__, __LINE__, EntityStateFrameNr, ServerFrameNr); return false; }


    // Determine the source state to delta-decompress against (an old state or the baseline).
    const cf::Network::StateT* DeltaState=NULL;

    if (DeltaFrameNr>0)
    {
        // Der oben angekündigte Test, ob der DeltaState nicht schon zu weit in der Vergangenheit liegt.
        // Einen gültigen State können wir dann nicht mehr produzieren, und dem Calling-Code muß klar sein oder klar werden,
        // daß er gegen die BaseLines komprimierte Messages anfordern muß.
        if (EntityStateFrameNr-DeltaFrameNr >= m_OldStates.Size())
        {
            EnqueueString("CLIENT WARNING: %s, L %u: Delta state too old!\n", __FILE__, __LINE__);
            return false;
        }

        DeltaState = &m_OldStates[DeltaFrameNr & (m_OldStates.Size()-1)];
    }
    else
    {
        DeltaState = &m_BaseLine;
    }

    // Set the result as the new entity state, and record it in the m_OldStates for future reference.
    EntityStateFrameNr=ServerFrameNr;

    const cf::Network::StateT NewState = DeltaMessage ? cf::Network::StateT(*DeltaState, *DeltaMessage) : *DeltaState;

    m_OldStates[EntityStateFrameNr & (m_OldStates.Size()-1)] = NewState;
    SetState(NewState);
    return true;
}


bool EngineEntityT::Repredict(const ArrayT<PlayerCommandT>& PlayerCommands, unsigned long RemoteLastIncomingSequenceNr, unsigned long LastOutgoingSequenceNr)
{
    if (!UsePrediction.GetValueBool())
        return false;

    if (LastOutgoingSequenceNr-RemoteLastIncomingSequenceNr>PlayerCommands.Size())
    {
        EnqueueString("WARNING - Prediction impossible: Last ack'ed PlayerCommand is too old (%u, %u)!\n", RemoteLastIncomingSequenceNr, LastOutgoingSequenceNr);
        return false;
    }

    /*
     * This assumes that this method is immediately called after ParseServerDeltaUpdateMessage(),
     * where the state of this entity has been set to the state of the latest server frame,
     * and that every in-game packet from the server contains a delta update message for our local client!
     */

    // Unseren Entity über alle relevanten (d.h. noch nicht bestätigten) PlayerCommands unterrichten.
    // Wenn wir auf dem selben Host laufen wie der Server (z.B. Single-Player Spiel oder lokaler Client bei non-dedicated-Server Spiel),
    // werden die Netzwerk-Nachrichten in Nullzeit (im Idealfall über Memory-Buffer) versandt.
    // Falls dann auch noch der Server mit full-speed läuft, sollte daher immer RemoteLastIncomingSequenceNr==LastOutgoingSequenceNr sein,
    // was impliziert, daß dann keine Prediction stattfindet (da nicht notwendig!).
    for (unsigned long SequenceNr=RemoteLastIncomingSequenceNr+1; SequenceNr<=LastOutgoingSequenceNr; SequenceNr++)
        Entity->ProcessConfigString(&PlayerCommands[SequenceNr & (PlayerCommands.Size()-1)], "PlayerCommand");

    Entity->Think(-2.0, 0);
    return true;
}


void EngineEntityT::Predict(const PlayerCommandT& PlayerCommand, unsigned long OutgoingSequenceNr)
{
    if (!UsePrediction.GetValueBool())
        return;

    Entity->ProcessConfigString(&PlayerCommand, "PlayerCommand");
    Entity->Think(-1.0, 0);
}


void EngineEntityT::GetCamera(Vector3dT& Origin, unsigned short& Heading, unsigned short& Pitch, unsigned short& Bank) const
{
    Origin = Entity->GetOrigin();
    Entity->GetCameraOrientation(Heading, Pitch, Bank);
}


bool EngineEntityT::GetLightSourceInfo(unsigned long& DiffuseColor, unsigned long& SpecularColor, VectorT& Position, float& Radius, bool& CastsShadows) const
{
    return Entity->GetLightSourceInfo(DiffuseColor, SpecularColor, Position, Radius, CastsShadows);
}


void EngineEntityT::Draw(bool FirstPersonView, const VectorT& ViewerPos) const
{
    MatSys::Renderer->PushMatrix(MatSys::RendererI::MODEL_TO_WORLD);
    MatSys::Renderer->PushLightingParameters();

    unsigned short Ent_Heading;
    unsigned short Ent_Pitch;
    unsigned short Ent_Bank;

    Entity->GetBodyOrientation(Ent_Heading, Ent_Pitch, Ent_Bank);

    // Get the currently set lighting parameters.
    const float* PosL=MatSys::Renderer->GetCurrentLightSourcePosition();
    VectorT      LightSourcePos   =VectorT(PosL[0], PosL[1], PosL[2]);
    float        LightSourceRadius=MatSys::Renderer->GetCurrentLightSourceRadius();

    const float* PosE=MatSys::Renderer->GetCurrentEyePosition();
    VectorT      EyePos=VectorT(PosE[0], PosE[1], PosE[2]);


    // Starting from world space, compute the position of the light source in model space.
    LightSourcePos=LightSourcePos-Entity->GetOrigin();         // Convert into unrotated model space.
    LightSourcePos=LightSourcePos.GetRotZ(-90.0+float(Ent_Heading)/8192.0*45.0);
    LightSourcePos=scale(LightSourcePos, 1.0/25.4);

    // Don't forget to scale the radius of the light source appropriately down (into model space), too.
    LightSourceRadius/=25.4f;


    // Do the same for the eye: Starting from world space, compute the position of the eye in model space.
    EyePos=EyePos-Entity->GetOrigin();         // Convert into unrotated model space.
    EyePos=EyePos.GetRotZ(-90.0+float(Ent_Heading)/8192.0*45.0);
    EyePos=scale(EyePos, 1.0/25.4);


    // Set the modified (now in model space) lighting parameters.
    MatSys::Renderer->SetCurrentLightSourcePosition(float(LightSourcePos.x), float(LightSourcePos.y), float(LightSourcePos.z));
    MatSys::Renderer->SetCurrentLightSourceRadius(LightSourceRadius);
    MatSys::Renderer->SetCurrentEyePosition(float(EyePos.x), float(EyePos.y), float(EyePos.z));


    // Set the ambient light color for this entity.
    // Paradoxically, this is not a global, but rather a per-entity value that is derived from the lightmaps that are close to that entity.
    const Vector3fT AmbientEntityLight=Entity->GetGameWorld()->GetAmbientLightColorFromBB(Entity->GetDimensions(), Entity->GetOrigin());
    MatSys::Renderer->SetCurrentAmbientLightColor(AmbientEntityLight.x, AmbientEntityLight.y, AmbientEntityLight.z);


    MatSys::Renderer->Translate(MatSys::RendererI::MODEL_TO_WORLD, float(Entity->GetOrigin().x), float(Entity->GetOrigin().y), float(Entity->GetOrigin().z));
    MatSys::Renderer->RotateZ  (MatSys::RendererI::MODEL_TO_WORLD, 90.0f-float(Ent_Heading)/8192.0f*45.0f);
    MatSys::Renderer->Scale    (MatSys::RendererI::MODEL_TO_WORLD, 25.4f);

    Entity->Draw(FirstPersonView, (float)length(ViewerPos-Entity->GetOrigin()));


    MatSys::Renderer->PopLightingParameters();
    MatSys::Renderer->PopMatrix(MatSys::RendererI::MODEL_TO_WORLD);
}


void EngineEntityT::PostDraw(float FrameTime, bool FirstPersonView)
{
    if (!FirstPersonView)
    {
        // Using !FirstPersonView is a hack to exclude "our" entity, which is predicted already,
        // from being interpolated (whereas other player entities should be interpolated normally).
        Entity->Interpolate(FrameTime);
    }

    Entity->PostDraw(FrameTime, FirstPersonView);
}

#endif   /* !DEDICATED */
