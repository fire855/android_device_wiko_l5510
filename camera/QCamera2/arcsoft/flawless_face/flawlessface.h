/*----------------------------------------------------------------------------------------------
 *
 * This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and
 * confidential information.
 *
 * The information and code contained in this file is only for authorized ArcSoft employees
 * to design, create, modify, or review.
 *
 * DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
 *
 * If you are not an intended recipient of this file, you must not copy, distribute, modify,
 * or take any action in reliance on it.
 *
 * If you have received this file in error, please immediately notify ArcSoft and
 * permanently delete the original and any copy of any file and any printout thereof.
 *
 *-------------------------------------------------------------------------------------------------*/
/*
 * flawlessface.h
 *
 * Reference:
 *
 * Description:
 *
 * Create by ykzhu 2014-5-21
 *
 */

#ifndef _FLAWLESS_FACE_H_
#define _FLAWLESS_FACE_H_

#include "amcomdef.h"
#include "merror.h"
#include "asvloffscreen.h"

#define ERR_FACE_NO_FOUND -7000

class CFlawlessFace
{
public:
	CFlawlessFace();
	virtual ~CFlawlessFace();

	MLong Init();
	MLong Uninit();

	MVoid SetSkinSoftenLevel(MLong lLevel);
	MVoid SetSkinBrightLevel(MLong lLevel);
	MVoid SetSlenderFaceLevel(MLong lLevel);
	MVoid SetEyeEnlargmentLevel(MLong lLevel);
	MVoid SetEyeBrightLevel(MLong lLevel);
	MVoid EnableDeblemish(MBool bEnable);
	MVoid EnableDepouch(MBool bEnable);

	MLong Process(LPASVLOFFSCREEN pImgSrc, LPASVLOFFSCREEN pImgDst);

private:
	MLong InitEngine();
	MLong UninitEngine();

private:
	MBool	m_bInit;
	MHandle	m_hFFEngine;
	MLong	m_lSkinSoftenLevel;
	MLong	m_lSkinBrightLevel;
	MLong	m_lSlenderFaceLevel;
	MLong	m_lEyeEnlargmentLevel;
	MLong	m_lEyeBrightLevel;
	MBool	m_bDeblemish;
	MBool	m_bDepouch;
};

#endif /* _FLAWLESS_FACE_H_ */
