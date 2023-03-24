#include "DDSFile.hpp"
#include "Debug.hpp"

#include <cstdio>

#include <dxgiformat.h>

enum DDSFlags
{
	eDDSFlags_Caps        = 0x1,
	eDDSFlags_Height      = 0x2,
	eDDSFlags_Width       = 0x4,
	eDDSFlags_Pitch       = 0x8,
	eDDSFlags_PixelFormat = 0x1000,
	eDDSFlags_MipMapCount = 0x20000,
	eDDSFlags_LinearSize  = 0x80000,
	eDDSFlags_Depth		  = 0x800000
};

enum DDSPixelFormatFlags
{
	eDDSPixelFormatFlags_AlphaPixels = 0x1,
	eDDSPixelFormatFlags_Alpha       = 0x2,
	eDDSPixelFormatFlags_FourCC      = 0x4,
	eDDSPixelFormatFlags_RGB         = 0x40,
	eDDSPixelFormatFlags_YUV         = 0x200,
	eDDSPixelFormatFlags_Luminance   = 0x20000
};

enum DDSPixelFormatFourCC : uint32_t
{
	eDDSPixelFormatFourCC_DXT1 = 0x31545844,
	eDDSPixelFormatFourCC_DXT2 = 0x32545844,
	eDDSPixelFormatFourCC_DXT3 = 0x33545844,
	eDDSPixelFormatFourCC_DXT4 = 0x34545844,
	eDDSPixelFormatFourCC_DXT5 = 0x35545844,
	eDDSPixelFormatFourCC_DX10 = 0x30315844
};

static const uint32_t DDSMagicNumber = 0x20534444u;

enum class DDSDimension
{
	Texture1D = 2,
	Texture2D = 3,
	Texture3D = 4
};

struct DDSPixelFormat
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDSHeader
{
	uint32_t magicNum;
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDSPixelFormat ddspf;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
};

struct DDSHeaderDX10
{
	DXGI_FORMAT format;
	DDSDimension dimension;
	uint32_t miscFlags1;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

DDSFile::DDSFile(const char* fileName)
{
	FILE* file = nullptr;
	fopen_s(&file, fileName, "rb");
	if (file)
	{
		m_handle = file;
		readHeader();
	}
}

DDSFile::~DDSFile()
{
	if (m_handle)
	{
		FILE* file = static_cast<FILE*>(m_handle);
		m_handle = nullptr;
		fclose(file);
	}
}

void DDSFile::readHeader()
{
	DDSHeader header{};
	FILE* file = static_cast<FILE*>(m_handle);
	fread_s(&header, sizeof(header), sizeof(header), 1, file);

	BreakIfNot(header.magicNum == DDSMagicNumber);
	if (header.ddspf.dwFlags & eDDSPixelFormatFlags_FourCC)
	{
		switch (header.ddspf.dwFourCC)
		{
		case eDDSPixelFormatFourCC_DXT1:
			m_format = DDSFormat::BC1;
			break;
		case eDDSPixelFormatFourCC_DXT2:
			m_format = DDSFormat::BC2;
			break;
		case eDDSPixelFormatFourCC_DXT3:
			m_format = DDSFormat::BC3;
			break;
		case eDDSPixelFormatFourCC_DXT4:
			m_format = DDSFormat::BC4;
			break;
		case eDDSPixelFormatFourCC_DXT5:
			m_format = DDSFormat::BC5;
			break;
		case eDDSPixelFormatFourCC_DX10:
			readDX10Header();
			break;
		default:
			BreakIfNot(0);
			break;
		}
		//calculate top mip size
		m_compressed = true;
	}
	else
	{
		BreakIfNot(header.flags & eDDSFlags_Pitch);
		m_compressed = false;
	}
	if (header.flags & eDDSFlags_Depth)
	{
		m_depth = header.dwDepth;
	}

	m_height = header.height;
	m_width = header.dwWidth;
}

void DDSFile::readDX10Header()
{
	DDSHeaderDX10 header{};
	FILE* file = static_cast<FILE*>(m_handle);
	fread_s(&header, sizeof(header), sizeof(header), 1, file);
	switch (header.format)
	{
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC1_TYPELESS:
		m_format = DDSFormat::BC1;
		break;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC2_TYPELESS:
		m_format = DDSFormat::BC2;
		break;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
		m_format = DDSFormat::BC3;
		break;
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
	case DXGI_FORMAT_BC4_TYPELESS:
		m_format = DDSFormat::BC4;
		break;
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC5_TYPELESS:
		m_format = DDSFormat::BC5;
		break;
	default:
		break;
	}
}
