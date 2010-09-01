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
 
#ifndef __TE_PERFTESTCACHEENABLEDSTEP_H__
#define __TE_PERFTESTCACHEENABLEDSTEP_H__

#include "TE_PerfTestCacheStepBase.h"

//--------------------------------
// class CPerfTestCacheEnabledStep
//--------------------------------
_LIT(KPerfTestCacheEnabledStep, "PerfTestCacheEnabledStep");

class CPerfTestCacheEnabledStep : public CPerfTestCacheStepBase
	{
public:
	CPerfTestCacheEnabledStep();
	virtual TVerdict doTestStepL(void);
	};

#endif
