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

#ifndef _CAFU_MODEL_HPP_
#define _CAFU_MODEL_HPP_

#include "Templates/Array.hpp"
#include "MaterialSystem/Mesh.hpp"
#include "Math3D/Matrix.hpp"
#include "Math3D/Vector3.hpp"
#include "Model.hpp"


class MaterialT;
class ModelLoaderT;
namespace MatSys { class RenderMaterialT; }


/// This class represents a native Cafu model.
class CafuModelT : public ModelT
{
    public:

    /// A joint is a bone in the hierarchical skeleton of the model.
    struct JointT
    {
        std::string Name;
        int         Parent;
        Vector3fT   Pos;
        Vector3fT   Qtr;
    };


    /// A mesh consisting of a material and a list of vertices.
    struct MeshT
    {
        /// A single triangle.
        struct TriangleT
        {
            int       VertexIdx[3];     ///< The indices to the three vertices that define this triangle.

            int       NeighbIdx[3];     ///< The array indices of the three neighbouring triangles at the edges 01, 12 and 20. -1 indicates no neighbour, -2 indicates more than one neighbour.
            bool      Polarity;         ///< True if this triangle has positive polarity (texture is not mirrored), or false if it has negative polarity (texture is mirrored, SxT points inward).

            Vector3fT Draw_Normal;      ///< The draw normal for this triangle, required for the shadow-silhouette determination.
        };

        /// A single vertex.
        struct VertexT
        {
            float       u;              ///< Texture coordinate u.
            float       v;              ///< Texture coordinate v.
            int         FirstWeightIdx;
            int         NumWeights;

            bool        Polarity;       ///< True if this vertex belongs to triangles with positive polarity, false if it belongs to triangles with negative polarity. Note that a single vertex cannot belong to triangles of both positive and negative polarity (but a GeoDup of this vertex can belong to the other polarity).
            ArrayT<int> GeoDups;        ///< This array contains the indices of vertices that are geometrical duplicates of this vertex, see AreVerticesGeoDups() for more information. The indices are stored in increasing order, and do *not* include the index of "this" vertex. Note that from the presence of GeoDups in a cmdl/md5 file we can *not* conclude that a break in the smoothing was intended by the modeller. Cylindrically wrapping seams are one counter-example.

            Vector3fT   Draw_Pos;       ///< Position of this vertex.
            Vector3fT   Draw_Normal;    ///< Vertex normal.
            Vector3fT   Draw_Tangent;   ///< Vertex tangent.
            Vector3fT   Draw_BiNormal;  ///< Vertex binormal.
        };

        /// A weight is a fixed position in the coordinate system of a joint.
        struct WeightT
        {
            int       JointIdx;
            float     Weight;
            Vector3fT Pos;
        };


        /// Determines whether the two vertices with array indices Vertex1Nr and Vertex2Nr are geometrical duplicates of each other.
        /// Two distinct vertices are geometrical duplicates of each other if
        /// (a) they have the same NumWeights and the same FirstWeightIdx, or
        /// (b) they have the same NumWeights and the referred weights have the same contents.
        /// The two vertices may in general have different uv-coordinates, which are not considered for the geodup property.
        /// Note that if Vertex1Nr==Vertex2Nr, true is returned (case (a) above).
        /// @param Vertex1Nr Array index of first vertex.
        /// @param Vertex2Nr Array index of second vertex.
        /// @return Whether the vertices are geometrical duplicate of each other.
        bool AreGeoDups(int Vertex1Nr, int Vertex2Nr) const;

        MaterialT*               Material;       ///< The material of this mesh.
        MatSys::RenderMaterialT* RenderMaterial; ///< The render material used to render this mesh.
        ArrayT<TriangleT>        Triangles;      ///< List of triangles this mesh consists of.
        ArrayT<VertexT>          Vertices;       ///< List of vertices this mesh consists of.
        ArrayT<WeightT>          Weights;        ///< List of weights that are attached to the skeleton (hierarchy of bones/joints).
    };


    /// There is one AnimT object per .md5anim file.
    /// With the help of one AnimT structure we can obtain an array of joints (ArrayT<JointT>, like m_Joints) for each frame of that animation sequence.
    struct AnimT
    {
        struct AnimJointT
        {
         // std::string   Name;             ///< Checked to be identical with the name  of the base mesh at load time.
         // int           Parent;           ///< Checked to be identical with the value of the base mesh at load time.
            float         BaseValues[6];    ///< One position and one quaternion triple that define the common base frame that is shared across all frames.
            unsigned long Flags;
            int           FirstDataIdx;     ///< If f is the current frame, this is the index into Frames[f].AnimData[...].
         // int           NumDatas;         ///< There are so many data values as there are bits set in Flags.
        };

        /// A keyframe in the animation.
        struct FrameT
        {
            float         BB[6];            ///< The bounding box of the model in this frame.
            ArrayT<float> AnimData;
        };


        float              FPS;             ///< Framerate for this animation sequence.
        ArrayT<AnimJointT> AnimJoints;      ///< AnimJoints.Size() == m_Joints.Size()
        ArrayT<FrameT>     Frames;          ///< List of frames this animation consists of.
    };


    /// The constructor. Creates a new Cafu model from a file as directed by the given model loader.
    /// @param Loader   The model loader that actually imports the file and fills in the model data.
    CafuModelT(ModelLoaderT& Loader);

    /// The destructor.
    ~CafuModelT();

    // The ModelT interface.
    const std::string& GetFileName() const;
    void               Draw(int SequenceNr, float FrameNr, float LodDist, const ModelT* SubModel=NULL) const;
    bool               GetGuiPlane(int SequenceNr, float FrameNr, float LodDist, Vector3fT& GuiOrigin, Vector3fT& GuiAxisX, Vector3fT& GuiAxisY) const;
    void               Print() const;
    int                GetNrOfSequences() const;
    const float*       GetSequenceBB(int SequenceNr, float FrameNr) const;
 // float              GetNrOfFrames(int SequenceNr) const;
    float              AdvanceFrameNr(int SequenceNr, float FrameNr, float DeltaTime, bool Loop=true) const;


    private:

    void InitMeshes();                                                      ///< An auxiliary method for the constructors.
    void UpdateCachedDrawData(int SequenceNr, float FrameNr) const;         ///< A private auxiliary method.


    const std::string     m_FileName;             ///< File name of this model.
    const bool            m_UseGivenTangentSpace;
 // const bool            m_CastShadows;          ///< Should this model cast shadows?
    ArrayT<JointT>        m_Joints;               ///< Array of joints of this model.
    mutable ArrayT<MeshT> m_Meshes;               ///< Array of (sub)meshes of this model.
    float                 m_BaseBB[6];
    ArrayT<AnimT>         m_Anims;                ///< Array of animations of this model.

    mutable ArrayT<MatrixT>       m_Draw_JointMatrices;
    mutable ArrayT<MatSys::MeshT> m_Draw_Meshes;

    mutable int   m_Draw_CachedDataAtSequNr;
    mutable float m_Draw_CachedDataAtFrameNr;
};

#endif
