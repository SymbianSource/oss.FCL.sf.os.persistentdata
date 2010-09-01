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
//
 
#ifndef __TE_DEFECTTESTSTEP_H__
#define __TE_DEFECTTESTSTEP_H__

#include <test/testexecutestepbase.h>

_LIT(KPerfTestDefectStepDEF057491, "CentRepDefectTest057491");
_LIT(KPerfTestDefectStepDEF059633, "CentRepDefectTest059633");

class CPerfTestDefectStep057491 : public CTestStep
	{
public:
	CPerfTestDefectStep057491();
	virtual TVerdict doTestStepL(void);
	};

class CPerfTestDefectStep059633 : public CTestStep
	{
public:
	CPerfTestDefectStep059633();
	virtual TVerdict doTestStepL(void);
	};

#endif // __TE_DEFECTTESTSTEP_H__
