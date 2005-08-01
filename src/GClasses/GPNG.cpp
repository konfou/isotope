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
	Holder<unsigned char*> hData(new unsigned char[rowbytes * height]);
	png_bytep pRawData = (png_bytep)hData.Get();
	unsigned int i;
	{
		Holder<unsigned char*> hRows(new unsigned char[sizeof(png_bytep) * height]);
		png_bytep* pRows = (png_bytep*)hRows.Get();
		for(i = 0; i < height; i++)
			pRows[i] = pRawData + i * rowbytes;
		png_read_image(reader.m_pReadStruct, pRows);
	}

	// Copy to the GImage
	unsigned long nPixels = width * height;
	GColor* pRGBQuads = pImage->GetRGBQuads();
	unsigned char *pBytes = pRawData;
	for(i = 0; i < nPixels; i++)
	{
		*pRGBQuads = gRGB(pBytes[0], pBytes[1], pBytes[2]); // todo: include the alpha channel too
		pBytes += channels;
		pRGBQuads++;
	}

	// Check for additional tags
	png_read_end(reader.m_pReadStruct, reader.m_pEndInfoStruct);

	return true;
}
