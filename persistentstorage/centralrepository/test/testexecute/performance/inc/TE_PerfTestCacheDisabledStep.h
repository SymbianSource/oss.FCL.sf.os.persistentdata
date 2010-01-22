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
// TE_PerfTestCacheEnabledStep.h
// 
//
 
#ifndef __TE_PERFTESTCACHEDISABLEDSTEP_H__
#define __TE_PERFTESTCACHEDISABLEDSTEP_H__

#include "TE_PerfTestCacheStepBase.h"

//--------------------------------
// class CPerfTestCacheDisabledStep
//--------------------------------

_LIT(KPerfTestCacheDisabledStep, "PerfTestCacheDisabledStep");

class CPerfTestCacheDisabledStep : public CPerfTestCacheStepBase
	{
public:
	CPerfTestCacheDisabledStep();
	virtual TVerdict doTestStepL(void);
	};

#endif
