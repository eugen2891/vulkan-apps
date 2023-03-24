#pragma once

#include <cstdint>

struct DDSHeader;

enum class DDSFormat
{
	Unknown,
	BC1,
	BC2,
	BC3,
	BC4,
	BC5
};

class DDSFile
{
public:
	explicit DDSFile(const char* fileName);
	~DDSFile();
private:
	void readHeader();
	void readDX10Header();
	void* m_handle = nullptr;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_depth = 1;
	DDSFormat m_format = DDSFormat::Unknown;
	bool m_compressed = false;
};
