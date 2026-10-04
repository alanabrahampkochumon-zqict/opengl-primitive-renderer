#pragma once

#include "3dshapes/ShapeMeshes.h"
#include "MaterialTag.h"
#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>

/// Encapsulation for each shape.
enum class Shape : uint8_t
{
    Box,
    Prism,
    Plane,
    Cylinder,
    HalfSphere,
    Cone,
    Torus,
    TaperedCylinder
};

/// Represents a single shape in a scene encapsulating its transforms and color.
class Mesh
{
public:
    glm::vec3 m_position{ 0.0f }; /// XYZ Position(Default: origin<0, 0, 0>)
    glm::vec3 m_rotation{ 0.0f }; /// XYZ Rotation(Default: zero rotation)
    glm::vec3 m_scale{ 1.0f };    /// XYZ Scale (Default: 1.0)

    /// Color of the mesh in RGBA (Default: White)
    /// Value must be between 0.0f and 1.0f
    glm::vec4 m_albedo{ 1.0f, 1.0f, 1.0f, 1.0f };
    // In order to ensure that texture and color can co-exist,
    // we assign a sentinel value to texture initially. If
    // there is no texture then we will render with the albedo.
    Texture m_texture{ Texture::NO_TEXTURE };
    glm::vec2 m_uvScale{ 1.0f, 1.0f }; // Texture scale

    Shape m_shape;          /// The primitive shape of the mesh
    MaterialTag m_material; /// The shader material for the mesh.
    std::string m_meshName; //// The string literal to identify the shaped. Used by IMGUI id, so must be unique.


    /// <summary>
    /// Instantiate a shape mesh.
    /// </summary>
    /// <param name="shape">The shape type corresponding to this mesh.</param>
    /// <param name="meshName">The mesh name to identify the mesh. Must be unique.</param>
    Mesh(const Shape shape, std::string meshName): m_shape{ shape }, m_meshName(std::move(meshName)) {}


    /// <summary>
    ///  Render a shape using the ShapeMeshes instance.
    /// </summary>
    /// <remarks>The class expects the shape to be loaded in memory in the calling function.</remarks>
    /// <param name="shapeMeshes">The shape meshes instance response for loading and drawing shapes.</param>
    void Draw(ShapeMeshes* shapeMeshes) const
    {
        /// Draw the shape
        switch (m_shape)
        {
            case Shape::Plane:
                shapeMeshes->DrawPlaneMesh();
                break;
            case Shape::Box:
                shapeMeshes->DrawBoxMesh();
                break;
            case Shape::Prism:
                shapeMeshes->DrawPrismMesh();
                break;
            case Shape::Cylinder:
                shapeMeshes->DrawCylinderMesh();
                break;
            case Shape::HalfSphere:
                shapeMeshes->DrawHalfSphereMesh();
                break;
            case Shape::Cone:
                shapeMeshes->DrawConeMesh();
                break;
            case Shape::Torus:
                shapeMeshes->DrawTorusMesh();
                break;
            case Shape::TaperedCylinder:
                shapeMeshes->DrawTaperedCylinderMesh();
                break;
        }
    }
};
