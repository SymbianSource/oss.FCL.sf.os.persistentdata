/**
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 @file
*/
#if (!defined __INITIALISE_PMA_STEP_H__)
#define __INITIALISE_PMA_STEP_H__
#include <test/testexecutestepbase.h>
#include "Te_centrep_BURSuiteStepBase.h"
#include "srvreqs.h"
#include "srvdefs.h"
#include "cachemgr.h"

class CinitialisePMAStep : public CTe_centrep_BURSuiteStepBase
	{
public:
	CinitialisePMAStep();
	~CinitialisePMAStep();
	virtual TVerdict doTestStepL();

private:
	};

_LIT(KinitialisePMAStep,"initialisePMAStep");

#endif
