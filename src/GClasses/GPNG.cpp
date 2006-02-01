/*
	Copyright (C) 2006, Mike Gashler

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	see http://www.gnu.org/copyleft/lesser.html
*/

#include "GPNG.h"
#ifdef WIN32
#include "png.h"
#else
#include <png.h>
#endif // !WIN32
#include "GMacros.h"
#include "GImage.h"

class GPNGReader
{
public:
	png_structp m_pReadStruct;
	png_infop m_pInfoStruct;
	png_infop m_pEndInfoStruct;
	const unsigned char* m_pData;
	int m_nPos;

	GPNGReader(const unsigned char* pData)
	{
		m_pData = pData;
		m_nPos = 0;
		m_pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!m_pReadStruct)
		{
			m_pReadStruct = NULL;
			GAssert(false, "Failed to create the read struct");
			return;	
		}
		m_pInfoStruct = png_create_info_struct(m_pReadStruct);
		m_pEndInfoStruct = png_create_info_struct(m_pReadStruct);
	}

	~GPNGReader()
	{
		if(m_pReadStruct)
			png_destroy_read_struct(&m_pReadStruct, &m_pInfoStruct, &m_pEndInfoStruct);
	}

	void ReadBytes(unsigned char* pBuf, int nBytes)
	{
		memcpy(pBuf, m_pData + m_nPos, nBytes);
		m_nPos += nBytes;
	}
};

void readFunc(png_struct* pReadStruct, png_bytep pBuf, png_size_t nSize)
{
	GPNGReader* pReader = (GPNGReader*)png_get_io_ptr(pReadStruct);
	pReader->ReadBytes((unsigned char*)pBuf, (int)nSize);
}

bool LoadPng(GImage* pImage, const unsigned char* pData, int nDataSize)
{
	// Check for the PNG signature
	if(nDataSize < 8 || png_sig_cmp((png_bytep)pData, 0, 8) != 0)
	{
		GAssert(false, "Not a PNG file");
		return false;
	}

	// Read all PNG data up until the image data chunk.
	GPNGReader reader(pData);
	png_set_read_fn(reader.m_pReadStruct, (png_voidp)&reader, (png_rw_ptr)readFunc);
	png_read_info(reader.m_pReadStruct, reader.m_pInfoStruct);

	// Get the image data
	int depth, color;
	unsigned long width, height;
	png_get_IHDR(reader.m_pReadStruct, reader.m_pInfoStruct, &width, &height, &depth, &color, NULL, NULL, NULL);
GAssert(depth == 8, "todo: remove this line");
	pImage->SetSize(width, height);

	// Set gamma correction
	double dGamma;
	if (png_get_gAMA(reader.m_pReadStruct, reader.m_pInfoStruct, &dGamma))
		png_set_gamma(reader.m_pReadStruct, 2.2, dGamma);
	else
		png_set_gamma(reader.m_pReadStruct, 2.2, 1.0 / 2.2); // 1.0 = viewing gamma, 2.2 = screen gamma

	// Update the 'info' struct with the gamma information
	png_read_update_info(reader.m_pReadStruct, reader.m_pInfoStruct);

	// Tell it to expand palettes to full channels
	png_set_expand(reader.m_pReadStruct);
	png_set_gray_to_rgb(reader.m_pReadStruct);

	// Allocate the row pointers
	unsigned long rowbytes = png_get_rowbytes(reader.m_pReadStruct, reader.m_pInfoStruct);
	unsigned long channels = rowbytes / width;
	ArrayHolder<unsigned char*> hData(new unsigned char[rowbytes * height]);
	png_bytep pRawData = (png_bytep)hData.Get();
	unsigned int i;
	{
		ArrayHolder<unsigned char*> hRows(new unsigned char[sizeof(png_bytep) * height]);
		png_bytep* pRows = (png_bytep*)hRows.Get();
		for(i = 0; i < height; i++)
			pRows[i] = pRawData + i * rowbytes;
		png_read_image(reader.m_pReadStruct, pRows);
	}

	// Copy to the GImage
	unsigned long nPixels = width * height;
	GColor* pRGBQuads = pImage->GetRGBQuads();
	unsigned char *pBytes = pRawData;
	if(channels > 3)
	{
		GAssert(channels == 4, "unexpected number of channels");
		for(i = 0; i < nPixels; i++)
		{
			*pRGBQuads = gARGB(pBytes[3], pBytes[0], pBytes[1], pBytes[2]);
			pBytes += channels;
			pRGBQuads++;
		}
	}
	else
	{
		GAssert(channels == 3, "unexpected number of channels");
		int alpha;
		for(i = 0; i < nPixels; i++)
		{
			if(pBytes[0] == 0 && pBytes[1] == 0xff && pBytes[2] == 0) // If the color is pure green (0x00ff00)
				alpha = 0;
			else
				alpha = 0xff;
			*pRGBQuads = gARGB(alpha, pBytes[0], pBytes[1], pBytes[2]);
			pBytes += channels;
			pRGBQuads++;
		}
	}

	// Check for additional tags
	png_read_end(reader.m_pReadStruct, reader.m_pEndInfoStruct);

	return true;
}

// -----------------------------------------------------------------------

class GPNGWriter
{
public:
	png_structp m_pWriteStruct;
	png_infop m_pInfoStruct;

	GPNGWriter()
	{
		m_pWriteStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(m_pWriteStruct == NULL)
		{
			GAssert(false, "Failed to create write struct. Out of mem?");
			return;
		}
		m_pInfoStruct = png_create_info_struct(m_pWriteStruct);
		if(m_pInfoStruct == NULL)
		{
			GAssert(false, "Failed to create info struct. Out of mem?");
			return;
		}
	}

	~GPNGWriter()
	{
		png_destroy_write_struct(&m_pWriteStruct, &m_pInfoStruct);
	}
};


bool SavePng(GImage* pImage, FILE* pFile, bool bIncludeAlphaChannel)
{
	GPNGWriter writer;

	// Allocate the row pointers
	unsigned long width = pImage->GetWidth();
	unsigned long height = pImage->GetHeight();
	unsigned long channels = bIncludeAlphaChannel ? 4 : 3;
	unsigned long rowbytes = width * channels;
	ArrayHolder<unsigned char*> hData(new unsigned char[rowbytes * height]);
	png_bytep pRawData = (png_bytep)hData.Get();
	unsigned int i;
	ArrayHolder<unsigned char*> hRows(new unsigned char[sizeof(png_bytep) * height]);
	png_bytep* pRows = (png_bytep*)hRows.Get();
	for(i = 0; i < height; i++)
		pRows[i] = pRawData + i * rowbytes;

	// Copy to the GImage
	unsigned long nPixels = width * height;
	GColor* pRGBQuads = pImage->GetRGBQuads();
	GColor col;
	unsigned char *pBytes = pRawData;
	if(channels > 3)
	{
		GAssert(channels == 4, "unexpected number of channels");
		for(i = 0; i < nPixels; i++)
		{
			col = *(pRGBQuads++);
			*(pBytes++) = gRed(col);
			*(pBytes++) = gGreen(col);
			*(pBytes++) = gBlue(col);
			*(pBytes++) = gAlpha(col);
		}
	}
	else
	{
		GAssert(channels == 3, "unexpected number of channels");
		for(i = 0; i < nPixels; i++)
		{
			col = *(pRGBQuads++);
			*(pBytes++) = gRed(col);
			*(pBytes++) = gGreen(col);
			*(pBytes++) = gBlue(col);
		}
	}

	// Set the jump value
	if(setjmp(png_jmpbuf(writer.m_pWriteStruct)))
	{
		GAssert(false, "Failed to set the jump value for writing");
		return false;
	}

	// Init the IO
	png_init_io(writer.m_pWriteStruct, pFile);

	// Write image stats and settings
	png_set_IHDR(writer.m_pWriteStruct, writer.m_pInfoStruct,
			width, height, 8,
			bIncludeAlphaChannel ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(writer.m_pWriteStruct, writer.m_pInfoStruct);

	// Write the image data
	png_write_image(writer.m_pWriteStruct, pRows);
	png_write_end(writer.m_pWriteStruct, writer.m_pInfoStruct);

	return true;
}
