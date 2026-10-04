#pragma once

#include <string>
#include <vector>
#include <cstdint>

/// <summary>
/// Returns all the available textures.
/// Use getTextureTag to get the texture tag.
/// </summary>
enum class Texture:uint8_t
{
	BLUE_SAND,
	DARK_WOOD,
	LIGHT_WOOD,
	LIGHTER_WOOD,
	WHITE_RUBBER,
	BOOK_COVER_PLAIN,
	BOOK_COVER,
	BLUE_PEN,
	BROWN_PEN,
	NO_TEXTURE = UINT8_MAX // Sentinel value used for indicating there is no texture.
};


/// Structure for grouping texture filenames and texture tags
/// used for loading the texture.
struct TextureParam
{
	std::string path;
	std::string tag;
};


/// Path to the texture with a terminating /
constexpr auto gTexturePath = "../Utilities/textures/";

/// Vector of all available textures
const std::vector<TextureParam> gTextures = {
	{gTexturePath + std::string("BlueSand.jpg"), "Blue Sand"},
	{gTexturePath + std::string("WoodDark.jpg"), "Dark Wood"},
	{gTexturePath + std::string("WoodLight.jpg"), "Light Wood"},
	{gTexturePath + std::string("WoodLighter.png"), "Lighter Wood"},
	{gTexturePath + std::string("WhiteRubber.png"), "White Rubber"},
	{gTexturePath + std::string("BookCoverPlain.png"), "Book Cover Plain"},
	{gTexturePath + std::string("BookCover.jpg"), "Book Cover Textured"},
	{gTexturePath + std::string("BluePen.png"), "Blue Pen"},
	{gTexturePath + std::string("BrownPen.png"), "Brown Pen"},
};

/// Gets the texture name give a texture.
inline std::string getTextureTag(Texture texture)
{
	return gTextures[static_cast<uint8_t>(texture)].tag;
}
