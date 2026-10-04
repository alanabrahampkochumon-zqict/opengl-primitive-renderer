///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <imgui.h>
#include <glm/gtx/transform.hpp>

#define ENABLE_IMGUI // Enables ImGui for building the scene faster


// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}


/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height <<
			", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return (textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return (textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return (false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return (true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next Draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/
	// Create all the textures specified in the textures vector, by loading
	// their respective assets.
	for (const auto& texture : gTextures)
	{
		CreateGLTexture(texture.path.c_str(), texture.tag);
	}


	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// Load the texture
	LoadSceneTextures();
	// Setup the materials and lights
	DefineObjectMaterials();
	SetupSceneLights();

	// Load the meshes
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();

	// Setup the objects with transform
	// It is removed from the render function since creating and
	// destroyed objects every frame is very inefficient.
	PrepareMeshes();
}

/***********************************************************
* DefineObjectMaterials()
 *
* This method is used for configuring the various material
* settings for all of the objects in the 3D scene.
***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Wood has some patina which gives some soft satin like glow
	// To reflect that we can mix some brown in the color as well as
	// increase the shininess factor a bit
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.6f, 0.45f, 0.2f);
	woodMaterial.ambientStrength = 0.25f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.15f, 0.1f, 0.08f);
	woodMaterial.shininess = 5.0f;
	woodMaterial.tag = matToTag(MaterialTag::WOOD);
	m_objectMaterials.push_back(woodMaterial);

	// Glass is very reflective and has a light blue tint
	// So we have put a large shininess factor and has put a bit more
	// emphasis on the blue channel
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.45f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.4f);
	glassMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.4f);
	glassMaterial.shininess = 100.0;
	glassMaterial.tag = matToTag(MaterialTag::GLASS);
	m_objectMaterials.push_back(glassMaterial);

	// Rubber is not relatively glossy comparable to glass so we have
	// applied a large shininess factor. Moreover, due to off-white color of rubber
	// we have used a white color for ambiance, specular and diffuse color.
	OBJECT_MATERIAL rubberMaterial;
	rubberMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	rubberMaterial.ambientStrength = 0.4f;
	rubberMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	rubberMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	rubberMaterial.shininess = 20.0;
	rubberMaterial.tag = matToTag(MaterialTag::RUBBER);
	m_objectMaterials.push_back(rubberMaterial);

	// Sand material is very rough, so almost not specular reflection
	// Moreover, we have applied some ambient light to ensure that the material is not
	// dark and has a silhouette visible against the dark background
	// A slight bluish tint to diffuse and ambient color can add to the material's
	// texture which is not as blue as the original image.
	OBJECT_MATERIAL sandMaterial;
	sandMaterial.ambientColor = glm::vec3(0.2f, 0.35f, 0.6f);
	sandMaterial.ambientStrength = 0.3f;
	sandMaterial.diffuseColor = glm::vec3(0.2f, 0.35f, 0.6f);
	sandMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	sandMaterial.shininess = 0.01f;
	sandMaterial.tag = matToTag(MaterialTag::SAND);
	m_objectMaterials.push_back(sandMaterial);

	// We can use the same colors for paper regardless of their glossiness
	constexpr glm::vec3 plasticAmbientColor{1.0f};
	constexpr glm::vec3 plasticDiffuseColor{1.0f};
	constexpr glm::vec3 plasticSpecularColor{1.0f};

	// Since there are two kinds of plastic one glossy and one matte, we have divided it into two
	OBJECT_MATERIAL glossyPlasticMaterial;
	glossyPlasticMaterial.ambientColor = plasticAmbientColor;
	glossyPlasticMaterial.ambientStrength = 0.4f;
	glossyPlasticMaterial.diffuseColor = plasticDiffuseColor;
	glossyPlasticMaterial.specularColor = plasticSpecularColor;
	glossyPlasticMaterial.shininess = 50.0f;
	glossyPlasticMaterial.tag = matToTag(MaterialTag::GLOSSY_PLASTIC);
	m_objectMaterials.push_back(glossyPlasticMaterial);


	OBJECT_MATERIAL mattePlasticMaterial;
	mattePlasticMaterial.ambientColor = plasticAmbientColor;
	mattePlasticMaterial.ambientStrength = 0.35f;
	mattePlasticMaterial.diffuseColor = plasticDiffuseColor;
	mattePlasticMaterial.specularColor = plasticSpecularColor;
	mattePlasticMaterial.shininess = 0.5f;
	mattePlasticMaterial.tag = matToTag(MaterialTag::MATTE_PLASTIC);
	m_objectMaterials.push_back(mattePlasticMaterial);


	// We can use the same colors for paper regardless of their glossiness
	constexpr glm::vec3 paperAmbientColor{1.0f};
	constexpr glm::vec3 paperDiffuseColor{0.5f};
	constexpr glm::vec3 paperSpecularColor{1.0f};

	OBJECT_MATERIAL glossyPaperMaterial;
	glossyPaperMaterial.ambientColor = paperAmbientColor;
	glossyPaperMaterial.ambientStrength = 0.1f;
	glossyPaperMaterial.diffuseColor = paperDiffuseColor;
	glossyPaperMaterial.specularColor = paperSpecularColor;
	glossyPaperMaterial.shininess = 35.0f;
	glossyPaperMaterial.tag = matToTag(MaterialTag::GLOSSY_PAPER);
	m_objectMaterials.push_back(glossyPaperMaterial);


	OBJECT_MATERIAL mattePaperMaterial;
	mattePaperMaterial.ambientColor = paperAmbientColor;
	mattePaperMaterial.ambientStrength = 0.25f;
	mattePaperMaterial.diffuseColor = paperDiffuseColor;
	mattePaperMaterial.specularColor = paperSpecularColor;
	mattePaperMaterial.shininess = 0.1f;
	mattePaperMaterial.tag = matToTag(MaterialTag::MATTE_PAPER);
	m_objectMaterials.push_back(mattePaperMaterial);
}

void SceneManager::SetupSceneLights()
{
	// Backlight with a higher ambient light contribution lighting up the back of the scene
	m_pShaderManager->setVec3Value("lightSources[0].position", -5.0f, 20.0f, -13.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.05f);

	// Two lights are place on either sides are spotlight with a smaller ambient factor but
	// using a brown diffuse and specular colors.
	m_pShaderManager->setVec3Value("lightSources[1].position", -19.0f, 18.0f, -10.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.50f, 0.50f, 0.50f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 1.0f, 0.9f, 0.8f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 1.0f, 0.9f, 0.8f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.25f);

	m_pShaderManager->setVec3Value("lightSources[2].position", 19.0f, 13.5f, -6.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.15f, 0.15f, 0.15f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.8f, 0.8f, 0.6f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 1.0f, 0.9f, 0.8f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.25f);

	// Spotlight near the hour glass since it's a bit too much in shadows
	m_pShaderManager->setVec3Value("lightSources[3].position", 22.0f, 0.0f, 12.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);
	m_pShaderManager->setBoolValue("bUseLighting", true);
}


void SceneManager::PrepareMeshes()
{
	// Mesh has a default rotation of 0deg on all axes, a scale of 1 and is located
	// at the origin.
	Mesh groundPlane{Shape::Plane, "Ground Plane"};
	// Since our ground plane at the origin but scaled, we only need to update its scale and position.
	groundPlane.m_scale = glm::vec3{22.0f, 1.0f, 11.0f};
	groundPlane.m_position = glm::vec3{0.0f, -0.25f, -1.0f};
	groundPlane.m_texture = Texture::DARK_WOOD;
	/// Scale down the texture to have a similar grain size as the hour glass.
	groundPlane.m_uvScale = glm::vec2(4.0f);
	groundPlane.m_material = MaterialTag::WOOD;
	// add ground plane to meshes
	m_meshes.push_back(groundPlane);


	Mesh lightOne{Shape::HalfSphere, "Light Amb"};
	lightOne.m_material = MaterialTag::WOOD;
	lightOne.m_position = { -5.0f, 18.0f, -12.0f };
	m_meshes.push_back(lightOne);

	Mesh lightTwo{ Shape::HalfSphere, "Light Left" };
	lightTwo.m_material = MaterialTag::MATTE_PLASTIC;
	lightTwo.m_position = { -19.0f, 13.5f, -5.0f };
	m_meshes.push_back(lightTwo);

	Mesh lightThree{ Shape::HalfSphere, "Light Right" };
	lightThree.m_material = MaterialTag::MATTE_PLASTIC;
	lightThree.m_position = { 19.0f, 13.5f, -3.0f };
	m_meshes.push_back(lightThree);

	AddBookToScene();
	AddLaptopToScene();
	AddStickNoteToScene();
	AddPenHolderToScene();
}

void SceneManager::AddPensScene()
{
	glm::vec3 penScale = {0.20f, 3.0f, 0.20f};

	// Both body and cap can share rotations but will need to have different scales and positions
	Mesh penOne{Shape::Cylinder, "Pen One"};
	penOne.m_scale = penScale;
	penOne.m_position = glm::vec3{14.60f, 4.50f, 5.50f};
	penOne.m_rotation = glm::vec3{335.0f, 0.0f, 345.0f};
	penOne.m_texture = Texture::BROWN_PEN;
	penOne.m_material = MaterialTag::MATTE_PLASTIC;


	Mesh penTwo{Shape::Cylinder, "Pen Two"};
	penTwo.m_scale = penScale;
	penTwo.m_position = glm::vec3{14.60f, 4.225f, 5.5f};
	penTwo.m_rotation = glm::vec3{0.0f, 0.0f, 25.0f};
	penTwo.m_texture = Texture::BLUE_PEN;
	penTwo.m_material = MaterialTag::MATTE_PLASTIC;

	m_meshes.push_back(penOne);
	m_meshes.push_back(penTwo);
}


void SceneManager::AddPenHolderToScene()
{
	// We can use very light blue color with transparency to mimic transparency.
	glm::vec4 lightBlue = glm::vec4(0.75f, 0.95f, 1.0f, 0.15f);

	/// Position for the entire holder
	// float penHolderZ = 2.0f;
	// Mesh has a default rotation of 0deg on all axes, a scale of 1 and is located
	// at the origin.
	Mesh holderBase{Shape::Box, "Pen Holder Base"};
	holderBase.m_scale = glm::vec3{8.0f, 0.25f, 2.0f};
	// Small y-offset to keep the meshes from colliding and creating glimmering effect.
	holderBase.m_position = glm::vec3{12.0f, 0.0f, 5.0f};
	holderBase.m_texture = Texture::LIGHT_WOOD;
	holderBase.m_material = MaterialTag::WOOD;

	Mesh penHolderBody{Shape::Cylinder, "Pen Holder Body"};
	penHolderBody.m_position = {14.5f, 0.15f, 5.0f};
	penHolderBody.m_scale = {0.95f, 5.0f, 0.95f};
	penHolderBody.m_texture = Texture::LIGHT_WOOD;
	penHolderBody.m_material = MaterialTag::WOOD;


	///  /\ =  = /\ Mesh that captures the stand for the axis of hour glass
	/// /  \    /  \
	/// ============
	Mesh hourGlassLeftStand{Shape::Prism, "Hour Glass Left Stand"};
	hourGlassLeftStand.m_position = glm::vec3{9.25f, 2.15f, 5.0f};
	hourGlassLeftStand.m_rotation = glm::vec3{90.0f, 180.0f, 270.0f};
	hourGlassLeftStand.m_scale = glm::vec3{1.40f, 0.25, 4.0f};
	hourGlassLeftStand.m_texture = Texture::LIGHT_WOOD;
	hourGlassLeftStand.m_material = MaterialTag::WOOD;

	Mesh hourGlassRightStand{Shape::Prism, "Hour Glass Right Stand"};
	hourGlassRightStand.m_position = glm::vec3{12.0, 2.15f, 5.0f};
	// Since the stands are mirror opposites, we can just duplicate their scale and rotation
	hourGlassRightStand.m_rotation = hourGlassLeftStand.m_rotation;
	hourGlassRightStand.m_scale = hourGlassLeftStand.m_scale;
	hourGlassRightStand.m_texture = Texture::LIGHT_WOOD;
	hourGlassRightStand.m_material = MaterialTag::WOOD;


	Mesh holderPinLeft{Shape::Cylinder, "Hour Glass Holder Pin Left"};
	holderPinLeft.m_position = glm::vec3{9.75f, 3.5f, 5.0f};
	holderPinLeft.m_rotation = glm::vec3{0.0f, 0.0f, 90.0f};
	holderPinLeft.m_scale = glm::vec3{0.1f, 0.6f, 0.1f};
	holderPinLeft.m_texture = Texture::LIGHT_WOOD;
	holderPinLeft.m_material = MaterialTag::WOOD;

	Mesh holderPinRight{Shape::Cylinder, "Hour Glass Holder Pin Right"};
	// Since rotation and scale are similar for holder pins we use the left side's params
	// for the right side.
	holderPinRight.m_position = glm::vec3{12.10f, 3.5f, 5.0f};
	holderPinRight.m_rotation = holderPinLeft.m_rotation;
	holderPinRight.m_scale = holderPinLeft.m_scale;
	holderPinRight.m_texture = Texture::LIGHT_WOOD;
	holderPinRight.m_material = MaterialTag::WOOD;

	// ======
	// ||  ||
	// ||  ||
	// ======
	// Since there is no rotation for top and bottom frame we can leave it at zero
	Mesh hourGlassTopFrame{Shape::Box, "Hour Glass Top Frame"};
	hourGlassTopFrame.m_position = glm::vec3{10.60f, 6.25f, 5.0f};
	hourGlassTopFrame.m_scale = glm::vec3{2.25f, 0.2f, 1.75f};
	hourGlassTopFrame.m_texture = Texture::LIGHTER_WOOD;
	hourGlassTopFrame.m_material = MaterialTag::WOOD;


	Mesh hourGlassBottomFrame{Shape::Box, "Hour Glass Bottom Frame"};
	hourGlassBottomFrame.m_position = glm::vec3{10.60f, 1.25f, 5.0f};
	// Since scale is similar for top and bottom frames we use the top side's params
	// for the bottom side.
	hourGlassBottomFrame.m_scale = hourGlassTopFrame.m_scale;
	hourGlassBottomFrame.m_texture = Texture::LIGHTER_WOOD;
	hourGlassBottomFrame.m_material = MaterialTag::WOOD;


	// Since there is no rotation for top and bottom frame we can leave it at zero
	Mesh hourGlassLeftFrame{Shape::Box, "Hour Glass Left Frame"};
	hourGlassLeftFrame.m_position = glm::vec3{11.50f, 3.75f, 5.0f};
	hourGlassLeftFrame.m_scale = glm::vec3{0.15f, 5.0f, 1.5f};
	hourGlassLeftFrame.m_texture = Texture::LIGHTER_WOOD;
	hourGlassLeftFrame.m_material = MaterialTag::WOOD;

	Mesh hourGlassRightFrame{Shape::Box, "Hour Glass Right Frame"};
	hourGlassRightFrame.m_position = glm::vec3{9.70f, 3.75f, 5.0f};
	// Since scale and rotation is similar for left and right frames we use the left side's params
	// for the right side.
	hourGlassRightFrame.m_scale = hourGlassLeftFrame.m_scale;
	hourGlassRightFrame.m_texture = Texture::LIGHTER_WOOD;
	hourGlassRightFrame.m_material = MaterialTag::WOOD;


	/// Glass Top and Bottom Piece
	///  ___
	/// /   \ TOP DOME
	/// \   / TOP MIDDLE
	///  \ /
	///  / \  BOTTOM MIDDLE
	/// /   \
	/// \   / BOTTOM DOME
	///  ---
	Mesh hourGlassTopDome{Shape::HalfSphere, "Hour Glass Top Dome"};
	hourGlassTopDome.m_position = glm::vec3{10.6f, 5.25f, 5.0f};
	hourGlassTopDome.m_rotation = glm::vec3{0.0f, 0.0f, 0.0f};
	hourGlassTopDome.m_scale = glm::vec3{0.75f, 0.75f, 0.75f};
	hourGlassTopDome.m_albedo = lightBlue;
	hourGlassTopDome.m_material = MaterialTag::GLASS;

	Mesh hourGlassBottomDome{Shape::HalfSphere, "Hour Glass Bottom Dome"};
	hourGlassBottomDome.m_position = glm::vec3{10.6f, 2.25f, 5.0f};
	hourGlassBottomDome.m_rotation = glm::vec3{180.0f, 0.0f, 0.0f};
	// Since scale is the same top and bottom dome, we can share params
	hourGlassBottomDome.m_scale = hourGlassTopDome.m_scale;
	hourGlassBottomDome.m_texture = Texture::BLUE_SAND;
	hourGlassBottomDome.m_material = MaterialTag::SAND;

	// Since the middle meshes are essentially rotated at the top and bottom domes
	// respectively was can take their positions and it will make updating the position
	// easier later on.
	Mesh hourGlassTopMiddle{Shape::TaperedCylinder, "Hour Glass Top Middle"};
	hourGlassTopMiddle.m_position = hourGlassTopDome.m_position;
	hourGlassTopMiddle.m_rotation = glm::vec3{180.0f, 0.0f, 0.0f};
	hourGlassTopMiddle.m_scale = glm::vec3{0.75f, 1.50f, 0.75f};
	hourGlassTopMiddle.m_albedo = lightBlue;
	hourGlassTopMiddle.m_material = MaterialTag::GLASS;

	Mesh hourGlassBottomMiddle{Shape::TaperedCylinder, "Hour Glass Bottom Middle"};
	hourGlassBottomMiddle.m_position = hourGlassBottomDome.m_position;
	hourGlassBottomMiddle.m_rotation = glm::vec3{0.0f, 0.0f, 0.0f};
	// Since scale is the same for top and bottom middle pieces, we can share params
	hourGlassBottomMiddle.m_scale = hourGlassTopMiddle.m_scale;
	hourGlassBottomMiddle.m_albedo = lightBlue;
	hourGlassBottomMiddle.m_material = MaterialTag::GLASS;


	/// Top and bottom bushes that snuggly fit the glass in place
	/// NOTE: Rotation is not required so we don't need to specify it
	Mesh hourGlassTopBush{Shape::Torus, "Hour Glass Top Bush"};
	hourGlassTopBush.m_position = glm::vec3{10.6f, 1.50f, 5.0f};
	hourGlassTopBush.m_rotation = glm::vec3{90.0f, 180.0f, 1.0f};
	hourGlassTopBush.m_scale = glm::vec3{0.5f, 0.5f, 0.75f};
	hourGlassTopBush.m_texture = Texture::WHITE_RUBBER;
	hourGlassTopBush.m_material = MaterialTag::RUBBER;


	Mesh hourGlassBottomBush{Shape::Torus, "Hour Glass Bottom Bush"};
	hourGlassBottomBush.m_position = glm::vec3{10.6f, 6.0f, 5.0f};
	// Since bushes are mirrored rotation and scale can be shared
	hourGlassBottomBush.m_rotation = hourGlassTopBush.m_rotation;
	hourGlassBottomBush.m_scale = hourGlassTopBush.m_scale;
	hourGlassBottomBush.m_texture = Texture::WHITE_RUBBER;
	hourGlassBottomBush.m_material = MaterialTag::RUBBER;

	m_meshes.push_back(holderBase);
	m_meshes.push_back(penHolderBody);
	m_meshes.push_back(hourGlassLeftStand);
	m_meshes.push_back(hourGlassRightStand);
	m_meshes.push_back(holderPinLeft);
	m_meshes.push_back(holderPinRight);
	m_meshes.push_back(hourGlassTopFrame);
	m_meshes.push_back(hourGlassBottomFrame);
	m_meshes.push_back(hourGlassLeftFrame);
	m_meshes.push_back(hourGlassRightFrame);
	m_meshes.push_back(hourGlassTopDome);
	m_meshes.push_back(hourGlassBottomDome);
	m_meshes.push_back(hourGlassTopMiddle);
	m_meshes.push_back(hourGlassBottomMiddle);
	m_meshes.push_back(hourGlassTopBush);
	m_meshes.push_back(hourGlassBottomBush);

	AddPensScene();
}


void SceneManager::AddLaptopToScene()
{
	// To create contrast we need to create two colors for top and bottom panels
	glm::vec4 blackBody = glm::vec4(0.03f, 0.03f, 0.03f, 1.0f);
	glm::vec4 keyBlack = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	glm::vec4 blackTop = glm::vec4(0.04f, 0.04f, 0.04f, 1.0f);
	glm::vec4 blackInset = glm::vec4(0.015f, 0.015f, 0.015f, 1.0f);

	// Bottom Part of the laptop
	Mesh laptopBase{Shape::Box, "Laptop Base"};
	laptopBase.m_scale = glm::vec3{24.0f, 0.40f, 14.0f};
	laptopBase.m_position = glm::vec3{-6.0f, 0.0f, -3.0f};
	laptopBase.m_albedo = blackBody;
	laptopBase.m_material = MaterialTag::MATTE_PLASTIC;

	Mesh laptopPalmRest{Shape::Box, "Laptop Palm Rest"};
	laptopPalmRest.m_scale = glm::vec3{24.0f, 0.10f, 4.0f};
	laptopPalmRest.m_position = glm::vec3{-6.0f, 0.25f, 2.0f};
	laptopPalmRest.m_albedo = blackBody;
	laptopPalmRest.m_material = MaterialTag::MATTE_PLASTIC;

	// Mousepad is place to the left side and not on dead center in the real image.
	Mesh laptopMousePad{Shape::Box, "Laptop Mouse Pad"};
	laptopMousePad.m_scale = glm::vec3{8.0f, 0.05f, 3.50f};
	laptopMousePad.m_position = glm::vec3{-8.0f, 0.35f, 1.5f};
	laptopMousePad.m_albedo = blackInset;
	laptopMousePad.m_material = MaterialTag::GLOSSY_PLASTIC;

	// Keyboard
	// Since individually laying out all the keys are a lot of repeated code
	// we can just create a 5 row, 20 column layout of boxes that captures the idea
	// Delta represent where the next button should be placed
	// However since the first row has a different height, we can use padding a padding instead of delta
	float ySpacing = 0.0f;
	constexpr float xSpacing = 1.40f;
	float yDelta = 0.25f;
	// Since function keys are smaller than normal keys we can adjust the first row to be smaller
	constexpr float fnBtnHeight = 0.55f;
	constexpr float normalBtnHeight = 0.95f;
	for (size_t i = 0; i < 6; ++i) // Row
	{
		// For updating scale of first row, we need to cache the variable
		// This will lead to less performance hits from branch mispredictions,
		// if the conditional has been placed inside the inner loop
		glm::vec3 scale;
		if (i == 0)
		{
			scale = glm::vec3{1.25f, 0.05f, fnBtnHeight};
		}
		else
		{
			scale = glm::vec3{1.25f, 0.05f, normalBtnHeight};
		}

		for (size_t j = 0; j < 14; ++j) // Column
		{
			const std::string meshName = std::string("Laptop Key ") + std::to_string(i) + std::string(", ") +
				std::to_string(j);
			Mesh laptopKey{Shape::Box, meshName};

			laptopKey.m_scale = scale;
			laptopKey.m_position = glm::vec3{
				-15.0f + (static_cast<float>(j) * xSpacing), 0.35f, -8.0f + ySpacing
			};

			laptopKey.m_albedo = keyBlack;
			laptopKey.m_material = MaterialTag::GLOSSY_PLASTIC;

			m_meshes.emplace_back(laptopKey);
		}
		// Update the spacing after the adding the meshes
		if (i == 0)
		{
			ySpacing += fnBtnHeight + yDelta;
		}
		else
		{
			ySpacing += normalBtnHeight + yDelta;
		}
	}

	// Screen
	Mesh laptopTop{Shape::Box, "Laptop Top"};
	laptopTop.m_scale = glm::vec3{24.0f, 0.20f, 16.0f};
	laptopTop.m_position = glm::vec3{-6.0f, 8.15f, -10.60f};
	laptopTop.m_rotation = glm::vec3{85.0f, 0.0f, 0.0f}; // Slight tilt on the x-axis
	laptopTop.m_albedo = blackTop;
	laptopTop.m_material = MaterialTag::MATTE_PLASTIC;

	Mesh laptopScreen{Shape::Box, "Laptop Screen"};
	laptopScreen.m_scale = glm::vec3{22.40f, 0.10f, 14.0f};
	laptopScreen.m_position = glm::vec3{-6.0f, 8.30f, -10.45f};
	laptopScreen.m_rotation = glm::vec3{85.0f, 0.0f, 0.0f};
	laptopScreen.m_albedo = blackInset;
	laptopScreen.m_material = MaterialTag::GLOSSY_PLASTIC;

	m_meshes.emplace_back(laptopBase);
	m_meshes.emplace_back(laptopPalmRest);
	m_meshes.emplace_back(laptopMousePad);
	m_meshes.emplace_back(laptopTop);
	m_meshes.emplace_back(laptopScreen);
}


void SceneManager::AddBookToScene()
{
	glm::vec4 bookSpineColor = glm::vec4(0.55f, 0.71f, 0.30f, 1.0f);
	glm::vec4 bookPageColor = glm::vec4(0.95f); // Off-white color for pages.

	// Book top and bottom needs to be of the same scale
	// so we can use a shared value.
	glm::vec3 bookCoverScale = glm::vec3{13.6f, 0.10f, 13.0f};

	Mesh bookBottom{Shape::Box, "Book Bottom"};
	bookBottom.m_scale = bookCoverScale;
	bookBottom.m_position = glm::vec3{13.285f, 0.0f, -3.0f};
	bookBottom.m_texture = Texture::BOOK_COVER_PLAIN;
	bookBottom.m_material = MaterialTag::GLOSSY_PAPER;

	Mesh bookPages{Shape::Box, "Book Pages"};
	bookPages.m_scale = glm::vec3{13.0f, 0.75f, 12.0f};
	bookPages.m_position = glm::vec3{13.0f, 0.45f, -2.75f};
	bookPages.m_albedo = bookPageColor;
	bookPages.m_material = MaterialTag::MATTE_PAPER;

	// The UV coordinates of the book doesn't go beyond [0.0, 1.0] range
	// so it will compress the texture giving black streaks on the sides if we use the original texture.
	// To compensate, we can use a color only texture for the hard cover top and bottom
	// and create a plane that can take the image texture with the cover write-ups
	Mesh bookTop{Shape::Box, "Book Top"};
	bookTop.m_scale = bookCoverScale;
	bookTop.m_position = glm::vec3{13.285f, 0.90f, -3.0f};
	bookTop.m_texture = Texture::BOOK_COVER_PLAIN;
	bookTop.m_material = MaterialTag::GLOSSY_PAPER;

	Mesh bookTopImage{Shape::Box, "Book Top Image"};
	bookTopImage.m_scale = {bookCoverScale.x, 0.01f, bookCoverScale.z};
	bookTopImage.m_position = glm::vec3{13.285f, 0.95f, -3.0f};
	bookTopImage.m_texture = Texture::BOOK_COVER;
	bookTopImage.m_material = MaterialTag::GLOSSY_PAPER;

	Mesh bookSpine{Shape::Box, "Book Spine"};
	bookSpine.m_scale = glm::vec3{0.10f, 1.0f, 13.0f};
	bookSpine.m_position = glm::vec3{6.45f, 0.45f, -3.0f};
	bookSpine.m_albedo = bookSpineColor;
	bookSpine.m_material = MaterialTag::GLOSSY_PAPER;

	m_meshes.push_back(bookBottom);
	m_meshes.push_back(bookPages);
	m_meshes.push_back(bookTop);
	m_meshes.push_back(bookTopImage);
	m_meshes.push_back(bookSpine);
}


void SceneManager::AddStickNoteToScene()
{
	glm::vec4 bottomStickyNoteColor = glm::vec4(0.75f, 0.77f, 0.47f, 1.0f);
	glm::vec4 topStickyNoteColor = glm::vec4(1.0f, 0.89f, 0.75f, 1.0f);


	Mesh noteBottom{Shape::Box, "Sticky Note Bottom"};
	noteBottom.m_scale = glm::vec3{4.0f, 0.80f, 3.0f};
	noteBottom.m_position = glm::vec3{2.20f, 0.15f, 7.0f};
	noteBottom.m_albedo = bottomStickyNoteColor;
	noteBottom.m_material = MaterialTag::MATTE_PAPER;

	Mesh noteTop{Shape::Box, "Sticky Note Top"};
	noteTop.m_scale = glm::vec3{4.0f, 0.35f, 3.0f};
	noteTop.m_position = glm::vec3{2.20f, 0.70f, 7.0f};
	noteTop.m_albedo = topStickyNoteColor;
	noteTop.m_material = MaterialTag::MATTE_PAPER;

	m_meshes.push_back(noteBottom);
	m_meshes.push_back(noteTop);
}


/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
#ifdef ENABLE_IMGUI
	ImGui::Begin("Scene");
#endif
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/

	// Loop through each mesh and Draw them after applying appropriate transforms.
	for (auto& mesh : m_meshes)
	{
		// Set up the imgui window for manipulating the transforms
#ifdef ENABLE_IMGUI
		ImGui::PushID(mesh.m_meshName.c_str()); // Assign the id and increment.
		ImGui::TextColored(ImVec4(1, 1, 0, 1), mesh.m_meshName.c_str());
		ImGui::BeginChild(mesh.m_meshName.c_str());
		ImGui::DragFloat3("Scale", value_ptr(mesh.m_scale), 0.1f, 0, FLT_MAX);
		ImGui::DragFloat3("Position", value_ptr(mesh.m_position), 1.0f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Rotation", value_ptr(mesh.m_rotation), 5.0f, 0.0f, 360.0f);
		ImGui::ColorEdit4("Color", value_ptr(mesh.m_albedo));
		ImGui::EndChild();
		ImGui::PopID();
#endif

		// Set the transformations
		SetTransformations(mesh.m_scale, mesh.m_rotation.x, mesh.m_rotation.y, mesh.m_rotation.z, mesh.m_position);
		// Set the colors
		// If the mesh has a texture, then render it
		if (mesh.m_texture != Texture::NO_TEXTURE)
		{
			SetTextureUVScale(mesh.m_uvScale.x, mesh.m_uvScale.y);
			SetShaderTexture(getTextureTag(mesh.m_texture));
		}
		else
			SetShaderColor(mesh.m_albedo.r, mesh.m_albedo.g, mesh.m_albedo.b, mesh.m_albedo.a);
		SetShaderMaterial(matToTag(mesh.m_material));
		// Note: Turning off blending can help but the rigid shapes of cone break the shape immersion.
		//       Moreover, tapered cylinder has artifacting with transparency even with depth mask
		//       turned off.
		// Depth masks can create problem with transparent(alpha < 1) colors
		// so for glass material we need to disable depth mask and later reset it to true.
		if (mesh.m_texture == Texture::NO_TEXTURE && mesh.m_material == MaterialTag::GLASS)
		{
			glDepthMask(GL_FALSE);
		}
		// Draw the mesh
		mesh.Draw(m_basicMeshes);
		// Reset the blend mask
		if (mesh.m_texture == Texture::NO_TEXTURE && mesh.m_material == MaterialTag::GLASS)
		{
			glDepthMask(GL_TRUE);
		}
	}
#ifdef ENABLE_IMGUI
	ImGui::End();
#endif
}
