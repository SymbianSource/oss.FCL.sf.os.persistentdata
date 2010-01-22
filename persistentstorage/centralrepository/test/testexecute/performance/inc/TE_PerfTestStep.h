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
 
#ifndef __TE_PERFTESTSTEP_H__
#define __TE_PERFTESTSTEP_H__

#include <e32std.h>
#include <test/testexecutestepbase.h>

class TRepStatistics
	{
public:
#ifdef __WINS__
	#ifdef _DEBUG
		enum { KTicks2LoadFromCache = 70 }; // emulator UDEB
	#else
		enum { KTicks2LoadFromCache = 40 }; // emulator UREL
	#endif
#else
	#ifdef _DEBUG
		enum { KTicks2LoadFromCache = 90 }; // armv5 UDEB
	#else
		enum { KTicks2LoadFromCache = 60 }; // armv5 UREL
	#endif
#endif

	inline TRepStatistics();
	inline TRepStatistics(const TRepStatistics& aRepStatistics);
    inline TRepStatistics(TUint32 aRepUid, TUint32 aLoadTicks);
    inline TBool MatchUid(TUint32 aRepUid) const;
    void AddData(TUint32 aElapsedTicks);

	TUint32 iRepUid;
	TUint   iUseCount; // how many times the rep is open
	TUint32 iSumLoadTicks;
	TUint   iCacheMisses; // how many times rep need to be reloaded
	};

//-----------------------------------------------

class TIpcStatistics
	{
public:
	inline TIpcStatistics();
	void AddData(TUint32 aElapsedTicks);

	TUint iUseCount;
	TUint32 iSumElapsedTicks;
	};

//-----------------------------------------------

_LIT(KGetPerfTestResults, "GetPerfTestResults");
_LIT(KIniTestMode, "TestMode");
_LIT(KResultsSection, "Output");

class CPerfTestStep : public CTestStep
	{
public:
	enum TTestModes
		{
		ETiming,
		EBoot
		};

	inline CPerfTestStep(){SetTestStepName(KGetPerfTestResults);};
	virtual TVerdict doTestStepL(void);

private:
	void ConvertTickToSecAndMilli(TUint32 aTick, TUint& aSec, TUint& aMilliSec);

private:
	TInt iTickFreq;
	};

#include "TE_PerfTestStep.inl"

#endif
