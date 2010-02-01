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

/**************************/
/*** MatSys Font (Code) ***/
/**************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Font.hpp"
#include "MaterialSystem/Material.hpp"
#include "MaterialSystem/MaterialManager.hpp"
#include "MaterialSystem/Mesh.hpp"
#include "MaterialSystem/Renderer.hpp"
#include "Math3D/Matrix.hpp"
#include "OpenGL/OpenGLWindow.hpp"

#if defined(_WIN32)
    #if defined(_MSC_VER)
        #define vsnprintf _vsnprintf
    #endif
#endif


FontT::FontT(const std::string& MaterialName)
    : RenderMaterial(MatSys::Renderer->RegisterMaterial(MaterialManager->GetMaterial(MaterialName)))
{
}


FontT::~FontT()
{
    MatSys::Renderer->FreeMaterial(RenderMaterial);
}


// The copy constructor.
FontT::FontT(const FontT& Other)
    : RenderMaterial(MatSys::Renderer->RegisterMaterial(MatSys::Renderer->GetMaterialFromRM(Other.RenderMaterial)))
{
}


// The assignment operator.
FontT& FontT::operator = (const FontT& Other)
{
    // Make sure that self-assignment is handled properly.
    if (this==&Other) return *this;

    MatSys::Renderer->FreeMaterial(RenderMaterial);
    RenderMaterial=MatSys::Renderer->RegisterMaterial(MatSys::Renderer->GetMaterialFromRM(Other.RenderMaterial));
    return *this;
}


void FontT::Print(int PosX, int PosY, unsigned long Color, const char* PrintString, ...)
{
    if (!PrintString) return;

    va_list ArgList;
    char    PrintBuffer[256];

    va_start(ArgList, PrintString);
        vsnprintf(PrintBuffer, 256, PrintString, ArgList);
    va_end(ArgList);

    AccPrintBegin();
    AccPrint(PosX, PosY, Color, PrintBuffer);
    AccPrintEnd();
}


void FontT::AccPrintBegin()
{
    // Save the current matrices.
    MatSys::Renderer->PushMatrix(MatSys::RendererI::PROJECTION    );
    MatSys::Renderer->PushMatrix(MatSys::RendererI::MODEL_TO_WORLD);
    MatSys::Renderer->PushMatrix(MatSys::RendererI::WORLD_TO_VIEW );

    MatSys::Renderer->SetMatrix(MatSys::RendererI::PROJECTION,     MatrixT::GetProjOrthoMatrix(0.0f, float(SingleOpenGLWindow->GetWidth()), float(SingleOpenGLWindow->GetHeight()), 0.0f, -1.0f, 1.0f));
 // MatSys::Renderer->SetMatrix(MatSys::RendererI::MODEL_TO_WORLD, MatrixT());      // Set below, in AccPrint().
    MatSys::Renderer->SetMatrix(MatSys::RendererI::WORLD_TO_VIEW,  MatrixT());
}


void FontT::AccPrint(int PosX, int PosY, unsigned long Color, const char* PrintString, ...)
{
    if (!PrintString) return;

    va_list ArgList;
    char    PrintBuffer[256];

    va_start(ArgList, PrintString);
        vsnprintf(PrintBuffer, 256, PrintString, ArgList);
    va_end(ArgList);


    MatSys::Renderer->SetMatrix(MatSys::RendererI::MODEL_TO_WORLD, MatrixT::GetTranslateMatrix(Vector3fT(float(PosX), float(PosY), 0.0f)));
    MatSys::Renderer->SetCurrentAmbientLightColor(char((Color >> 16) & 0xFF)/255.0f, char((Color >> 8) & 0xFF)/255.0f, char(Color & 0xFF)/255.0f);
    MatSys::Renderer->SetCurrentMaterial(RenderMaterial);


    static MatSys::MeshT TextMesh(MatSys::MeshT::Quads);
    TextMesh.Vertices.Overwrite();
    TextMesh.Vertices.PushBackEmpty(4*strlen(PrintBuffer));

    for (unsigned long c=0; PrintBuffer[c]; c++)
    {
        const float CoordX=float(PrintBuffer[c] &  0xF)/16.0f;      // PrintBuffer[c] % 16
        const float CoordY=float(PrintBuffer[c] >>   4)/16.0f;      // PrintBuffer[c] / 16
        const float Size  =16.0/256.0;

        TextMesh.Vertices[4*c+0].SetOrigin( 0+c*10,  0); TextMesh.Vertices[4*c+0].SetTextureCoord(CoordX     , CoordY     );
        TextMesh.Vertices[4*c+1].SetOrigin(16+c*10,  0); TextMesh.Vertices[4*c+1].SetTextureCoord(CoordX+Size, CoordY     );
        TextMesh.Vertices[4*c+2].SetOrigin(16+c*10, 16); TextMesh.Vertices[4*c+2].SetTextureCoord(CoordX+Size, CoordY+Size);
        TextMesh.Vertices[4*c+3].SetOrigin( 0+c*10, 16); TextMesh.Vertices[4*c+3].SetTextureCoord(CoordX     , CoordY+Size);
    }

    MatSys::Renderer->RenderMesh(TextMesh);
}


void FontT::AccPrintEnd()
{
    // Restore the previously active matrices.
    MatSys::Renderer->PopMatrix(MatSys::RendererI::PROJECTION    );
    MatSys::Renderer->PopMatrix(MatSys::RendererI::MODEL_TO_WORLD);
    MatSys::Renderer->PopMatrix(MatSys::RendererI::WORLD_TO_VIEW );
}