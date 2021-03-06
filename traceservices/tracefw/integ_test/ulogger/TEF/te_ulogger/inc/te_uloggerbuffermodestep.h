/**
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @file te_uloggerbuffermodestep.h
 @internalTechnology
*/
#if (!defined __TE_ULOGGERBUFFERMODESTEP_H__)
#define __TE_ULOGGERBUFFERMODESTEP_H__
#include <test/testexecutestepbase.h>
#include "te_uloggermclsuitestepbase.h"
#include <e32base.h>
#include <e32math.h>
#include <uloggerclient.h>
#include "te_utracecmds.h"
#include "te_uloggermclsuitedefs.h"

class CULoggerBufferModeStep : public CTe_ULoggerMCLSuiteStepBase
	{
public:
	CULoggerBufferModeStep();
	~CULoggerBufferModeStep();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepL();
	virtual TVerdict doTestStepPostambleL();
	void writeToBuffer(const TPtrC iLit, TInt iTraceNumber ,RULogger* iLogger);

// Please add/modify your class members here:
private:
	};

_LIT(KULoggerBufferModeStep,"ULoggerBufferModeStep");

#endif
