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

#include "Loader_md5.hpp"
#include "MaterialSystem/Renderer.hpp"
#include "TextParser/TextParser.hpp"
#include "String.hpp"


LoaderMd5T::LoaderMd5T(const std::string& FileName) /*throw (ModelT::LoadError)*/
    : ModelLoaderT(FileName)
{
}


bool LoaderMd5T::UseGivenTS() const
{
    // For md5 models, the tangent space is not given by the model data from file.
    // Tell the Cafu model instance to recompute it from spatial and texture coordinates.
    return false;
}


void LoaderMd5T::Load(ModelMd5T* Model)
{
    ArrayT<std::string> ComponentFiles;

    if (cf::String::EndsWith(m_FileName, "md5"))
    {
        TextParserT TP(m_FileName.c_str());

        while (!TP.IsAtEOF()) ComponentFiles.PushBack(TP.GetNextToken());
        if (ComponentFiles.Size()<1) throw ModelT::LoadError();

        int PrefixLength=m_FileName.length()-1;
        while (PrefixLength>=0 && m_FileName[PrefixLength]!='/' && m_FileName[PrefixLength]!='\\') PrefixLength--;
        const std::string Prefix=std::string(m_FileName, 0, PrefixLength+1);

        for (unsigned long FileNr=0; FileNr<ComponentFiles.Size(); FileNr++)
        {
            // The ComponentFiles are specified relative to the parent file name,
            // thus extract the path portion from m_FileName and prepend it to the ComponentFiles.
            // If however the second character of a ComponentFiles entry is a ":", treat this as
            // a drive letter specification and thus as the special case of an absolute path.
            if (ComponentFiles[FileNr].length()>=2 && ComponentFiles[FileNr][1]==':') continue;

            ComponentFiles[FileNr]=Prefix+ComponentFiles[FileNr];
        }
    }
    else if (cf::String::EndsWith(m_FileName, "md5mesh"))
    {
        ComponentFiles.PushBack(m_FileName);
    }


    try
    {
        // Read the base mesh.
        TextParserT TP(ComponentFiles[0].c_str(), "{()}");

        // A good description of the md5 file format can be found at
        // http://www.doom3world.org/phpbb2/viewtopic.php?t=2884 (initial messages also printed out in my TechArchive),
        // and at http://wiki.doom3reference.com/wiki/Models_(folder).
        while (!TP.IsAtEOF())
        {
            const std::string Token=TP.GetNextToken();

                 if (Token=="MD5Version" ) { if (TP.GetNextToken()!="10") throw ModelT::LoadError(); }
            else if (Token=="commandline") TP.GetNextToken();       // Ignore the command line.
            else if (Token=="numJoints"  ) TP.GetNextToken();       // Ignore the given number of joints - we just load as many as we find.
            else if (Token=="numMeshes"  ) TP.GetNextToken();       // Ignore the given number of meshes - we just load as many as we find.
            else if (Token=="joints")
            {
                TP.AssertAndSkipToken("{");

                while (true)
                {
                    ModelMd5T::JointT Joint;

                    Joint.Name  =TP.GetNextToken(); if (Joint.Name=="}") break;
                    Joint.Parent=TP.GetNextTokenAsInt();
                    TP.AssertAndSkipToken("(");
                    Joint.Pos.x=TP.GetNextTokenAsFloat();
                    Joint.Pos.y=TP.GetNextTokenAsFloat();
                    Joint.Pos.z=TP.GetNextTokenAsFloat();
                    TP.AssertAndSkipToken(")");
                    TP.AssertAndSkipToken("(");
                    Joint.Qtr.x=TP.GetNextTokenAsFloat();
                    Joint.Qtr.y=TP.GetNextTokenAsFloat();
                    Joint.Qtr.z=TP.GetNextTokenAsFloat();
                    TP.AssertAndSkipToken(")");

                    // Make sure that all joints (bones) are declared in a proper hierarchical order.
                    // That is, if we traverse the m_Joints in increasing order 0, 1, 2, ..., then we are never faced with a
                    // parent that has not been seen earlier in the traversal sequence. (The -1 parent at index 0 is an exception.)
                    if (Joint.Parent>=int(Model->m_Joints.Size()))
                    {
                        printf("WARNING: Bad bone order!  %lu bones read so far, and the next (name \"%s\") is referring to parent %i.\n", Model->m_Joints.Size(), Joint.Name.c_str(), Joint.Parent);
                        throw ModelT::LoadError();  // TODO: Fix this by re-sorting the joints rather than by abortion!
                    }

                    Model->m_Joints.PushBack(Joint);
                }
            }
            else if (Token=="mesh")
            {
                Model->m_Meshes.PushBackEmpty();
                ModelMd5T::MeshT& Mesh=Model->m_Meshes[Model->m_Meshes.Size()-1];

                TP.AssertAndSkipToken("{");

                while (true)
                {
                    const std::string Token=TP.GetNextToken();

                         if (Token=="}"         ) break;
                    else if (Token=="numtris"   ) { if (Mesh.Triangles.Size()>0) throw ModelT::LoadError(); Mesh.Triangles.PushBackEmpty(TP.GetNextTokenAsInt()); }
                    else if (Token=="numverts"  ) { if (Mesh.Vertices .Size()>0) throw ModelT::LoadError(); Mesh.Vertices .PushBackEmpty(TP.GetNextTokenAsInt()); }
                    else if (Token=="numweights") { if (Mesh.Weights  .Size()>0) throw ModelT::LoadError(); Mesh.Weights  .PushBackEmpty(TP.GetNextTokenAsInt()); }
                    else if (Token=="shader")
                    {
                        Mesh.Material      =Model->GetMaterialByName(TP.GetNextToken());
                        Mesh.RenderMaterial=MatSys::Renderer!=NULL ? MatSys::Renderer->RegisterMaterial(Mesh.Material) : NULL;
                    }
                    else if (Token=="tri")
                    {
                        const unsigned long TriIdx=TP.GetNextTokenAsInt();

                        if (TriIdx>=Mesh.Triangles.Size()) throw ModelT::LoadError();

                        Mesh.Triangles[TriIdx].VertexIdx[0]=TP.GetNextTokenAsInt();
                        Mesh.Triangles[TriIdx].VertexIdx[1]=TP.GetNextTokenAsInt();
                        Mesh.Triangles[TriIdx].VertexIdx[2]=TP.GetNextTokenAsInt();
                    }
                    else if (Token=="vert")
                    {
                        const unsigned long VertIdx=TP.GetNextTokenAsInt();

                        if (VertIdx>=Mesh.Vertices.Size()) throw ModelT::LoadError();

                        TP.AssertAndSkipToken("(");
                        Mesh.Vertices[VertIdx].u=TP.GetNextTokenAsFloat();
                        Mesh.Vertices[VertIdx].v=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                        Mesh.Vertices[VertIdx].FirstWeightIdx=TP.GetNextTokenAsInt();
                        Mesh.Vertices[VertIdx].NumWeights    =TP.GetNextTokenAsInt();
                    }
                    else if (Token=="weight")
                    {
                        const unsigned long WeightIdx=TP.GetNextTokenAsInt();

                        if (WeightIdx>=Mesh.Weights.Size()) throw ModelT::LoadError();

                        Mesh.Weights[WeightIdx].JointIdx=TP.GetNextTokenAsInt();
                        Mesh.Weights[WeightIdx].Weight  =TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken("(");
                        Mesh.Weights[WeightIdx].Pos.x=TP.GetNextTokenAsFloat();
                        Mesh.Weights[WeightIdx].Pos.y=TP.GetNextTokenAsFloat();
                        Mesh.Weights[WeightIdx].Pos.z=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                    }
                    else printf("Unknown token \"%s\", skipping...\n", Token.c_str());
                }
            }
            else
            {
                // Unknown token!
                // If the next token is a block start, skip the block.
                // Otherwise it was something else that we just throw away.
                printf("Unknown token \"%s\", skipping...\n", Token.c_str());
                if (TP.GetNextToken()=="{") TP.SkipBlock("{", "}", true);
            }
        }

        if (Model->m_Joints.Size()==0) throw ModelT::LoadError();
        if (Model->m_Meshes.Size()==0) throw ModelT::LoadError();
    }
    catch (const TextParserT::ParseError&)
    {
        throw ModelT::LoadError();
    }


    Model->InitMeshes();


    // Read the individual animation sequence files.
    for (unsigned long FileNr=1; FileNr<ComponentFiles.Size(); FileNr++)
    {
        TextParserT TP(ComponentFiles[FileNr].c_str());

        try
        {
            ModelMd5T::AnimT Anim;

            Anim.FPS=24.0f;

            while (!TP.IsAtEOF())
            {
                const std::string Token=TP.GetNextToken();

                     if (Token=="MD5Version" ) { if (TP.GetNextToken()!="10") throw ModelT::LoadError(); }
                else if (Token=="commandline") TP.GetNextToken();       // Ignore the command line.
                else if (Token=="frameRate"  ) Anim.FPS=TP.GetNextTokenAsFloat();
                else if (Token=="numFrames"  )
                {
                    // Be stricter with numFrames than with some num* variables in the md5mesh file.
                    // This is required because for example there must be as many bounding boxes as frames.
                    if (Anim.Frames.Size()>0) throw ModelT::LoadError();

                    Anim.Frames.PushBackEmpty(TP.GetNextTokenAsInt());
                }
                else if (Token=="numJoints")
                {
                    // The numJoints here MUST match the numJoints in the md5mesh file!
                    const unsigned long numJoints=TP.GetNextTokenAsInt();

                    if (numJoints!=Model->m_Joints.Size()) { printf("%lu joints in md5anim file, %lu joints in md5mesh.\n", numJoints, Model->m_Joints.Size()); throw ModelT::LoadError(); }
                    if (Anim.AnimJoints.Size()>0) { printf("Anim.AnimJoints.Size()==%lu\n", Anim.AnimJoints.Size()); throw ModelT::LoadError(); }

                    Anim.AnimJoints.PushBackEmpty(numJoints);
                }
                else if (Token=="numAnimatedComponents")
                {
                    const unsigned long numAnimatedComponents=TP.GetNextTokenAsInt();

                    // This is the number of components that is animated in the frames (and thus so many values are stored with each frame).
                    // Therefore, the number of frames must have been specified already, so we can allocate all the memory here.
                    if (Anim.Frames.Size()==0) throw ModelT::LoadError();

                    for (unsigned long FrameNr=0; FrameNr<Anim.Frames.Size(); FrameNr++)
                        Anim.Frames[FrameNr].AnimData.PushBackEmpty(numAnimatedComponents);
                }
                else if (Token=="hierarchy")
                {
                    TP.AssertAndSkipToken("{");

                    for (unsigned long JointNr=0; JointNr<Anim.AnimJoints.Size(); JointNr++)
                    {
                        // Make sure that the name and parent are identical with the joint from the md5mesh file.
                        if (Model->m_Joints[JointNr].Name  !=TP.GetNextToken()) throw ModelT::LoadError();
                        if (Model->m_Joints[JointNr].Parent!=TP.GetNextTokenAsInt()) throw ModelT::LoadError();

                        Anim.AnimJoints[JointNr].Flags       =TP.GetNextTokenAsInt();
                        Anim.AnimJoints[JointNr].FirstDataIdx=TP.GetNextTokenAsInt();
                    }

                    TP.AssertAndSkipToken("}");
                }
                else if (Token=="bounds")
                {
                    TP.AssertAndSkipToken("{");

                    for (unsigned long FrameNr=0; FrameNr<Anim.Frames.Size(); FrameNr++)
                    {
                        TP.AssertAndSkipToken("(");
                        Anim.Frames[FrameNr].BB[0]=TP.GetNextTokenAsFloat();
                        Anim.Frames[FrameNr].BB[1]=TP.GetNextTokenAsFloat();
                        Anim.Frames[FrameNr].BB[2]=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                        TP.AssertAndSkipToken("(");
                        Anim.Frames[FrameNr].BB[3]=TP.GetNextTokenAsFloat();
                        Anim.Frames[FrameNr].BB[4]=TP.GetNextTokenAsFloat();
                        Anim.Frames[FrameNr].BB[5]=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                    }

                    TP.AssertAndSkipToken("}");
                }
                else if (Token=="baseframe")
                {
                    TP.AssertAndSkipToken("{");

                    for (unsigned long JointNr=0; JointNr<Anim.AnimJoints.Size(); JointNr++)
                    {
                        TP.AssertAndSkipToken("(");
                        Anim.AnimJoints[JointNr].BaseValues[0]=TP.GetNextTokenAsFloat();
                        Anim.AnimJoints[JointNr].BaseValues[1]=TP.GetNextTokenAsFloat();
                        Anim.AnimJoints[JointNr].BaseValues[2]=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                        TP.AssertAndSkipToken("(");
                        Anim.AnimJoints[JointNr].BaseValues[3]=TP.GetNextTokenAsFloat();
                        Anim.AnimJoints[JointNr].BaseValues[4]=TP.GetNextTokenAsFloat();
                        Anim.AnimJoints[JointNr].BaseValues[5]=TP.GetNextTokenAsFloat();
                        TP.AssertAndSkipToken(")");
                    }

                    TP.AssertAndSkipToken("}");
                }
                else if (Token=="frame")
                {
                    const unsigned long FrameNr=TP.GetNextTokenAsInt();

                    TP.AssertAndSkipToken("{");

                    for (unsigned long ComponentNr=0; ComponentNr<Anim.Frames[FrameNr].AnimData.Size(); ComponentNr++)
                        Anim.Frames[FrameNr].AnimData[ComponentNr]=TP.GetNextTokenAsFloat();

                    TP.AssertAndSkipToken("}");
                }
                else
                {
                    // Unknown token!
                    // If the next token is a block start, skip the block.
                    // Otherwise it was something else that we just throw away.
                    printf("Unknown token \"%s\", skipping...\n", Token.c_str());
                    if (TP.GetNextToken()=="{") TP.SkipBlock("{", "}", true);
                }
            }

            Model->m_Anims.PushBack(Anim);
        }
        catch (const TextParserT::ParseError&)
        {
            // Loading this animation sequence failed, but as the base mesh (the md5mesh file)
            // loaded properly, that is not reason enough to abort loading the entire model.
            ModelMd5T::AnimT InvalidAnim;

            InvalidAnim.FPS=-1.0f;                  // Use a negative FPS to flags this animation as invalid.
            Model->m_Anims.PushBack(InvalidAnim);   // Note that InvalidAnim.Frames.Size()==0, too.

            printf("WARNING: Loading animation sequence file %s failed just before input byte %lu!\n", ComponentFiles[FileNr].c_str(), TP.GetReadPosByte());
        }
        catch (const ModelT::LoadError&)
        {
            // Loading this animation sequence failed, but as the base mesh (the md5mesh file)
            // loaded properly, that is not reason enough to abort loading the entire model.
            ModelMd5T::AnimT InvalidAnim;

            InvalidAnim.FPS=-1.0f;                  // Use a negative FPS to flags this animation as invalid.
            Model->m_Anims.PushBack(InvalidAnim);   // Note that InvalidAnim.Frames.Size()==0, too.

            printf("WARNING: Loading animation sequence file %s failed just before input byte %lu!\n", ComponentFiles[FileNr].c_str(), TP.GetReadPosByte());
        }
    }
}