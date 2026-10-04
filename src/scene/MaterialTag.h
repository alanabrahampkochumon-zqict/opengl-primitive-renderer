#pragma once

#include <string>
#include <cstdint>

/// <summary>
/// Type safe version of material tags.
/// To convert to string representation use <see cref="matToTag"/>
/// </summary>
enum class MaterialTag: uint8_t
{
	WOOD, GLASS, RUBBER, SAND, GLOSSY_PLASTIC, MATTE_PLASTIC, GLOSSY_PAPER, MATTE_PAPER
};

/// Get the string value of material tag.
/// Used by object material.
inline std::string matToTag(const MaterialTag mat)
{
	switch (mat)
	{
	case MaterialTag::WOOD:
		return "wood";
	case MaterialTag::GLASS:
		return "glass";
	case MaterialTag::RUBBER:
		return "rubber";
	case MaterialTag::SAND:
		return "sand";
	case MaterialTag::GLOSSY_PLASTIC:
		return "glossy-plastic";
	case MaterialTag::MATTE_PLASTIC:
		return "matte-plastic";
	case MaterialTag::GLOSSY_PAPER:
		return "glossy-paper";
	case MaterialTag::MATTE_PAPER:
		return "matte-paper";

	default:
		throw std::runtime_error("Invalid material!");
	}
}
