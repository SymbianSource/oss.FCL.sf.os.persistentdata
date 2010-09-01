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
 
#ifndef __TE_PERFTESTCOMPARESTEP_H__
#define __TE_PERFTESTCOMPARESTEP_H__

#include "TE_PerfTestCacheStepBase.h"

//--------------------------------
// class CPerfTestCompareStep
//--------------------------------
_LIT(KPerfTestCompareStep, "PerfTestCompareStep");

class CPerfTestCompareStep : public CTestStep
	{
public:
	CPerfTestCompareStep();
	virtual TVerdict doTestStepL(void);

protected:
	class TSummary
		{
	public:
		TBuf<20> iName;
		TUint iUseCount;
		TUint32 iSumElapsedTicks;
		};
	void ReadResults(RArray<TSummary> &aArray, TBuf<KMaxFileName> &aString);
	RArray<TSummary> iResults1;
	RArray<TSummary> iResults2;
	};

#endif
