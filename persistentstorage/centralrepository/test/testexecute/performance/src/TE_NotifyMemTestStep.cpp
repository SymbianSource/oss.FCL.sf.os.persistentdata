// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "TE_NotifyMemTestStep.h"
#include "TE_PerfTestUtilities.h"
#include "t_cenrep_helper.h"
#include <centralrepository.h>
#include "srvreqs.h"
#include "srvPerf.h"
#include "srvdefs.h"
#include "cachemgr.h"

//--------------------------------
// class CNotifyMemTestStep
//--------------------------------

#if defined(__CENTREP_SERVER_MEMTEST__) 
const TUid KUidTestRepository1 				= { 0x00000001 };
const TUid KUidCacheTestRepositorySmTxt		= { 0x00000100 };
const TUid KUidCacheTestRepositoryMedTxt    = { 0x10057522 };
const TUid KUidCacheTestRepositorySmCre		= { 0x22222221 };
const TUid KUidCacheTestRepositoryLrgCre	= { 0xCCCCCC02 };

const TInt KKey1 = 1;
const TInt KKey2 = 0x201;
const TInt KKey3 = 8585472;
const TInt KKey4 = 22;
#endif

#ifdef __CENTREP_SERVER_MEMTEST__
void CNotifyMemTestStep::ProcessMemTestResults(TAny* aRawData, TInt32 aRawCount, RArray<TMemTestResult>& aMemTestResults)
	{
	ASSERT(aRawCount);
	ASSERT(aRawCount>=3);
	
	for(TInt i=0; i+2<aRawCount;)
		{
		TInt32 value1 = static_cast<TInt32*>(aRawData)[i++];
		TInt32 value2 = static_cast<TInt32*>(aRawData)[i++];
		TInt32 value3 = static_cast<TInt32*>(aRawData)[i++];
		
		if(value1 == KMagicMemTestOutOfBounds && value2 == KMagicMemTestOutOfBounds && value2 == KMagicMemTestOutOfBounds)
			{
			WARN_PRINTF1(_L("Memory test was not able to store all the requested values. Only the stored values will be reported."));
			}
		else
			{
			TMemTestResult result(value1, value2, ETrue);
			if(result.iLocation == EMemLcnOnDemand)
				{
				result.iInitHeapSize = 0;
				result.iFinalHeapSize = value3;
				aMemTestResults.Append(result);
				}
			}
		}
	}
#endif //__CENTREP_SERVER_MEMTEST__

/**
@SYMTestCaseID		SYSLIBS-CENTRALREPOSITORY-PT-1658
@SYMTestCaseDesc	Notify-only client optimization RAM usage test
@SYMTestPriority	High
@SYMTestActions		Retrieve memory usage data collected by CentRep server, analyze, and
	print out a summary during certain stages of the lifetime of session objects.
@SYMTestExpectedResults This test always pass. Engineers working on CentRep 
	improvement will run this suite before and after submission to gauge actual
	savings in RAM usage of repositories.
@SYMPREQ			PREQ 1228 Symbian OS v9.3 System Quality: Performance, ROM and RAM targets 
*/	
TVerdict CNotifyMemTestStep::doTestStepL()
	{
#if defined(__CENTREP_SERVER_MEMTEST__)

	SetTestStepResult(EFail);

	TInt testSteps = 0;
	TInt r;
	
	// Make sure cache is empty before we start
#ifdef __CENTREP_SERVER_CACHETEST__
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TESTL(r==KErrNone);
	r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TESTL(r==KErrNone);
#else //__CENTREP_SERVER_CACHETEST__
	CleanupRepositoryCache();
#endif //__CENTREP_SERVER_CACHETEST__
	//Reset RAM test data
	r = SetGetParameters(TIpcArgs(ERestartMemTests));
	TESTL(r==KErrNone);

	// Start of test
	Logger().WriteFormat(_L("Step %d, RAM at the beginning of test"),testSteps);
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

	CRepository* repository1;
	User::LeaveIfNull(repository1 = CRepository::NewLC(KUidTestRepository1));
	CRepository* repository2;
	User::LeaveIfNull(repository2 = CRepository::NewLC(KUidCacheTestRepositorySmTxt));
	CRepository* repository3;
	User::LeaveIfNull(repository3 = CRepository::NewLC(KUidCacheTestRepositoryMedTxt));
	CRepository* repository4;
	User::LeaveIfNull(repository4 = CRepository::NewLC(KUidCacheTestRepositorySmCre));
	CRepository* repository5;
	User::LeaveIfNull(repository5 = CRepository::NewLC(KUidCacheTestRepositoryLrgCre));

	// Reps automatically loaded by Init
	Logger().WriteFormat(_L("Step %d, RAM after sessions created"),testSteps);	
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

	TInt i;
	r = repository1->Get(KKey1, i);
	TESTL(r==KErrNone);
	r = repository2->Get(KKey2, i);
	TESTL(r==KErrNone);
	r = repository3->Get(KKey4, i);
	TESTL(r==KErrNone);
	r = repository4->Get(KKey1, i);
	TESTL(r==KErrNone);
	r = repository5->Get(KKey3, i);
	TESTL(r==KErrNone);

	// Get's don't change memory usage because reps are already loaded/in cache
	Logger().WriteFormat(_L("Step %d, RAM after Get() calls"),testSteps);		
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

#ifdef __CENTREP_SERVER_CACHETEST__
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TESTL(r==KErrNone);
	r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TESTL(r==KErrNone);
#else //__CENTREP_SERVER_CACHETEST__
	CleanupRepositoryCache();
#endif //__CENTREP_SERVER_CACHETEST__

	// Memory freed after reps are evicted
	Logger().WriteFormat(_L("Step %d, RAM after delaying for eviction"),testSteps);			
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);
	
	r = repository1->Get(KKey1, i);
	TESTL(r==KErrNone);
	r = repository2->Get(KKey2, i);
	TESTL(r==KErrNone);
	r = repository3->Get(KKey4, i);
	TESTL(r==KErrNone);
	r = repository4->Get(KKey1, i);
	TESTL(r==KErrNone);
	r = repository5->Get(KKey3, i);
	TESTL(r==KErrNone);

	// This time Get's trigger loads 
	Logger().WriteFormat(_L("Step %d, RAM after Get() calls"),testSteps);			
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

	CleanupStack::PopAndDestroy(5);

	// Closing sessions don't unload the reps because of coarse grained caching
	Logger().WriteFormat(_L("Step %d, RAM after sessions are closed"),testSteps);				
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

#ifdef __CENTREP_SERVER_CACHETEST__
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TESTL(r==KErrNone);
	r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TESTL(r==KErrNone);
#else //__CENTREP_SERVER_CACHETEST__
	CleanupRepositoryCache();
#endif //__CENTREP_SERVER_CACHETEST__

	// Reps are unloaded by cache manager
	Logger().WriteFormat(_L("Step %d, RAM after delaying for eviction"),testSteps);					
	r = SetGetParameters(TIpcArgs(ESingleMemTest, testSteps++));
	TESTL(r==KErrNone);

	// call inherited function to get the results
	SetTestStepResult(CMemTestStep::doTestStepL());
	
	return TestStepResult();
#else // defined(__CENTREP_SERVER_MEMTEST__)

	_LIT(KWarnMemTestMacroOff, "Memory test macro __CENTREP_SERVER_MEMTEST__ is disabled. Test not run.");
	WARN_PRINTF1(KWarnMemTestMacroOff);
 
	return EPass;	
# endif  // defined(__CENTREP_SERVER_MEMTEST__)



	}
