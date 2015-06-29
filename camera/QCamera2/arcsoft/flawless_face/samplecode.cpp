// samplecode.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "flawlessface.h"

MVoid setOffScreen(ASVLOFFSCREEN *pImg, MUInt8 *pData, 
				   MInt32 iWidth, MInt32 iHeight)
{	
	pImg->u32PixelArrayFormat = ASVL_PAF_NV21;
	pImg->i32Width = i32Width;
	pImg->i32Height = i32Height;
	pImg->pi32Pitch[0] = i32Width;
	pImg->pi32Pitch[1] = i32Width;
	pImg->ppu8Plane[0] = pData;
	pImg->ppu8Plane[1] = pData + iWidth*iHeight;
}

MVoid processImage(MTChar *pFilename, MInt32 iWidth, MInt32 iHeight)
{//data format ASVL_PAF_NV21
	CFlawlessFace	flawlessface;
	MRESULT ret = flawlessface.Init();
	if (ret != MOK)
		return;

	ASVLOFFSCREEN	tScreenSrc;
	LPASVLOFFSCREEN	pScreenDst;

	MInt32	iFrameLength = iWidth*iHeight*3/2;
	MUInt8	*pData = malloc(iFrameLength*sizeof(MUInt8));
	FILE	*stream = fopen(pFilename, "r");
	while (fread(pData, 1, iFrameLength, stream) == iFrameLength)
	{
		setOffScreen(tScreenSrc, pData, iWidth, iHeight);
		pScreenDst = &tScreenSrc;
		flawlessface.SetSkinSoftenLevel(30);
		flawlessface.SetSkinBrightLevel(30);
		flawlessface.SetEyeBrightLevel(30);
		flawlessface.EnableDeblemish(MTrue);
		flawlessface.EnableDepouch(MTrue);
		ret = flawlessface.Process(&tScreenSrc, pScreenDst);
		if (ret != MOK)
		{
		}
		////////////////////////////////////////////////////////////////
		//other operations
		//display(pScreenDst);
		/////////////////////////////////////////////////////////////////
	}

	free(pData);
	fclose(stream);
	flawlessface.Uninit();
}
