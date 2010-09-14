/**
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
*
*/



/**
 @file TestRemovePrimaryFltStep.h
 @internalTechnology
*/
#if (!defined __TESTREMOVEPRIMARYFLT_STEP_H__)
#define __TESTREMOVEPRIMARYFLT_STEP_H__
#include <test/testexecutestepbase.h>
#include "te_uloggerclientsuitestepbase.h"


class CTestRemovePrimaryFltStep : public CTestUloggerClientApiStepBase
	{
public:
	CTestRemovePrimaryFltStep();
	~CTestRemovePrimaryFltStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();

// Please add/modify your class members here:
private:
	};

_LIT(KTestRemovePrimaryFltStep,"TestRemovePrimaryFltStep");

#endif
