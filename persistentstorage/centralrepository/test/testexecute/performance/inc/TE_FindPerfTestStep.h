// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// TE_MemTestStep.h
// 
//
 
#ifndef __TE_FINDPERFTESTSTEP_H__
#define __TE_FINDPERFTESTSTEP_H__

#include <test/testexecutestepbase.h>

_LIT(KFindPerfTestName, "CentRepFindPerfTest");

class CFindPerfTestStep : public CTestStep
	{
public:
	CFindPerfTestStep();
	virtual TVerdict doTestStepL(void);
	};

#endif // __TE_FINDPERFTESTSTEP_H__
