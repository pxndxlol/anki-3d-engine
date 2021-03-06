// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class StencilStateInfo;

enum class TransientBufferType
{
	UNIFORM,
	STORAGE,
	VERTEX,
	TRANSFER,
	COUNT
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TransientBufferType, inline)

/// Convert buff usage to TransientBufferType.
inline TransientBufferType bufferUsageToTransient(BufferUsageBit bit)
{
	if(!!(bit & BufferUsageBit::UNIFORM_ALL))
	{
		return TransientBufferType::UNIFORM;
	}
	else if(!!(bit & BufferUsageBit::STORAGE_ALL))
	{
		return TransientBufferType::STORAGE;
	}
	else if((bit & BufferUsageBit::VERTEX) != BufferUsageBit::NONE)
	{
		return TransientBufferType::VERTEX;
	}
	else
	{
		ANKI_ASSERT(!!(bit & (BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::TEXTURE_UPLOAD_SOURCE)));
		return TransientBufferType::TRANSFER;
	}
}

/// Internal function that logs a shader error.
void logShaderErrorCode(const CString& error, const CString& source, GenericMemoryPoolAllocator<U8> alloc);

inline void checkTextureSurface(TextureType type, U depth, U mipCount, U layerCount, const TextureSurfaceInfo& surf)
{
	ANKI_ASSERT(surf.m_level < mipCount);
	switch(type)
	{
	case TextureType::_2D:
		ANKI_ASSERT(surf.m_depth == 0 && surf.m_face == 0 && surf.m_layer == 0);
		break;
	case TextureType::CUBE:
		ANKI_ASSERT(surf.m_depth == 0 && surf.m_face < 6 && surf.m_layer == 0);
		break;
	case TextureType::_3D:
		ANKI_ASSERT(surf.m_depth < depth && surf.m_face == 0 && surf.m_layer == 0);
		break;
	case TextureType::_2D_ARRAY:
		ANKI_ASSERT(surf.m_depth == 0 && surf.m_face == 0 && surf.m_layer < layerCount);
		break;
	case TextureType::CUBE_ARRAY:
		ANKI_ASSERT(surf.m_depth == 0 && surf.m_face < 6 && surf.m_layer < layerCount);
		break;
	default:
		ANKI_ASSERT(0);
	};
}

/// Check the validity of the structure.
Bool textureInitInfoValid(const TextureInitInfo& inf);

Bool framebufferInitInfoValid(const FramebufferInitInfo& inf);

/// Compute the size of a single surface.
void getFormatInfo(const PixelFormat& fmt, U& texelComponents, U& texelBytes, U& blockSize, U& blockBytes);

/// Compute the size of the surface.
PtrSize computeSurfaceSize(U width, U height, const PixelFormat& fmt);

/// Compute the size of the volume.
PtrSize computeVolumeSize(U width, U height, U depth, const PixelFormat& fmt);

Bool stencilTestDisabled(const StencilStateInfo& inf);

} // end namespace anki
