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

#include <centralrepository.h>
#include "t_cenrep_helper.h"
#include "srvPerf.h"
#include "srvreqs.h"
#include "TE_MemTestStep.h"

#ifdef __CENTREP_SERVER_MEMTEST__
// Method used for TIdentityRelation for TMemTestResult arrays
TBool MatchMemTestResult(const TMemTestResult& aKey, const TMemTestResult& aIndexItem)
	{
	return aIndexItem.iLocation   == aKey.iLocation   && 
		   aIndexItem.iIdentifier == aKey.iIdentifier && 
		   aIndexItem.iIsComplete == aKey.iIsComplete;
	}

void CMemTestStep::ProcessMemTestResults(TAny* aRawData, TInt32 aRawCount, RArray<TMemTestResult>& aMemTestResults)
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
			TMemTestResult result(value1, value2, EFalse);
			//look for not completed memTestResult that match the criteria
			TIdentityRelation<TMemTestResult> identity(MatchMemTestResult);
			
			TInt idx = aMemTestResults.Find(result, identity);
			if(idx == KErrNotFound)
				{
				result.iInitHeapSize = value3;
				aMemTestResults.Append(result);
				}
			else
				{
				TMemTestResult& foundResult = aMemTestResults[idx];
				foundResult.iFinalHeapSize = value3;
				foundResult.iIsComplete = ETrue;
				}
			}
		}
	}
#endif //__CENTREP_SERVER_MEMTEST__

CMemTestStep::CMemTestStep()
	{
	SetTestStepName(KMemTestName);
	}
	
// doTestStepL
// This test fetches the memory data collected by
// CentRep server, processes it and reports the result.
TVerdict CMemTestStep::doTestStepL()
	{
#ifdef __CENTREP_SERVER_MEMTEST__
	SetTestStepResult(EFail);
	// Pause a few seconds to ensure startup sequence is finished.
#ifdef __WINS__
 	const TTimeIntervalMicroSeconds32 interval = 12000000;
#else
 	const TTimeIntervalMicroSeconds32 interval = 4000000;
#endif
 	User::After(interval);

	TAny* buf = User::AllocL((KMemBufMaxEntry) * sizeof(TInt32));
	CleanupStack::PushL(buf);
	TPtr8 pBuf(static_cast<TUint8*>(buf), (KMemBufMaxEntry) * sizeof(TInt32));
	TInt numEntries;
	TPckg<TInt> pNumEntries(numEntries);
	TInt error = SetGetParameters(TIpcArgs(EGetMemResults, &pNumEntries, &pBuf));

	if(error == KErrNone && numEntries > 0)
		{
		RArray<TMemTestResult> memTestResults;
		
		ProcessMemTestResults(buf, numEntries, memTestResults);
		
		TInt count = memTestResults.Count();
		for(TInt i=0; i<count; i++)
			{
			TMemTestResult& result = memTestResults[i];
			if(result.iIsComplete)
				{
				if(result.iLocation == EMemLcnRepositoryOpen)
					{
					Logger().WriteFormat(_L("Memory usage for repository [%08X]: %d"), 
										 result.iIdentifier, 
										 result.iFinalHeapSize - result.iInitHeapSize);
					}
				else if(result.iLocation == EMemLcnOnDemand)
					{
					Logger().WriteFormat(_L("Heap memory usage of Centrep server at step %d: %d"), 
										 result.iIdentifier, 					
										 result.iFinalHeapSize);
					static TInt prevResult;
					switch (result.iIdentifier)
						{
						case 1: TESTL(result.iFinalHeapSize > prevResult); break;
						case 3: TESTL(result.iFinalHeapSize < prevResult); break;
						case 4: TESTL(result.iFinalHeapSize > prevResult); break;						
						}
					prevResult = result.iFinalHeapSize;
					}
				else
					{
					Logger().WriteFormat(_L("Memory usage at location[%d] for identifier[%d]: %d"),
										 result.iLocation, 
										 result.iIdentifier, 
										 result.iFinalHeapSize - result.iInitHeapSize);
					}
				}
			else
				{
				Logger().WriteFormat(_L("Incomplete memory reading at location[%d] for identifier[%d]"),
									 result.iLocation, 
									 result.iIdentifier);
				}
			}
		}
	
    CleanupStack::PopAndDestroy();

	if(error == KErrNone)
		{
		SetTestStepResult(EPass);
		}
#else
	WARN_PRINTF1(_L("Memory test macro __CENTREP_SERVER_MEMTEST__ is not defined. Test was not run."));
#endif //__CENTREP_SERVER_MEMTEST__

	return TestStepResult();
	}

