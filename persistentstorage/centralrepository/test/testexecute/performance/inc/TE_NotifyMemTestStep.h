/**
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* This file define all the common values thoughout your test project
* 
*
*/



/**
 @file
*/
#ifndef __TE_NOTIFYMEMTESTSTEP_H__
#define __TE_NOTIFYMEMTESTSTEP_H__

#include <test/testexecutestepbase.h>
#include "TE_MemTestStep.h"

_LIT(KNotifyMemTestStep, "NotifyMemTestStep");

class CNotifyMemTestStep : public CMemTestStep
	{
public:
	inline CNotifyMemTestStep(){SetTestStepName(KNotifyMemTestStep);};
	virtual TVerdict doTestStepL(void);
protected:
#ifdef __CENTREP_SERVER_MEMTEST__
	virtual void ProcessMemTestResults(TAny* aRawData, TInt32 aRawCount, RArray<TMemTestResult>& aMemTestResults);
#endif //__CENTREP_SERVER_MEMTEST__
	};

#endif //__TE_NOTIFYMEMTESTSTEP_H__
