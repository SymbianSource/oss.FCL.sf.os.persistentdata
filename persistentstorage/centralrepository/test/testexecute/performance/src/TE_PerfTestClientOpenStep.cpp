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

#include "TE_PerfTestClientOpenStep.h"
#include "TE_PerfTestUtilities.h"
#include "t_cenrep_helper.h"
#include "srvreqs.h"
#include "srvdefs.h"
#include <centralrepository.h>
#include "cachemgr.h"

#ifdef __CENTREP_SERVER_CACHETEST__

const TInt KMaxRepositories = 5;
const TInt KIterationCount = 100;
const TUid KUidClientOpenTestRepositoryIds[KMaxRepositories] = { 0x00000106,
                                                                 0x00000107,
                                                                 0x00000108,
                                                                 0x00000109,
                                                                 0x0000010A };
#endif //__CENTREP_SERVER_CACHETEST__

CPerfTestClientOpenStep::CPerfTestClientOpenStep()
	{
	SetTestStepName(KPerfTestClientOpenName);
	}
	
// doTestStepL
// This test fetches the memory data collected by
// CentRep server, processes it and reports the result.
TVerdict CPerfTestClientOpenStep::doTestStepL()
	{
	SetTestStepResult(EFail);

#ifndef __CENTREP_SERVER_CACHETEST__
	WARN_PRINTF1(_L("Test macro __CENTREP_SERVER_CACHETEST__ is not defined. Unable to disable cache. Test was not run."));
#else
	CRepository* repositories[KMaxRepositories];
	TReal openResults[KMaxRepositories];
	TReal closeResults[KMaxRepositories];
	TInt res = KErrNone;
	
	//enable cache back.
	res = SetGetParameters(TIpcArgs(EDisableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TESTL(res == KErrNone);
	
	for(TInt i=0; i<KMaxRepositories; i++)
		{
		openResults[i] = 0;
		closeResults[i] = 0;
		}
	
	for(TInt k=0; k<KIterationCount; k++)
		{
		//all of these operations are repeated for 100 times. Otherwise the measurement value
		//is too small to have an accurate value
		for(TInt i=0; i<KMaxRepositories; i++)
			{
			TUint32 startTick = 0;
			TUint32 endTick = 0;
			
			//open repositories and measure how long it takes to open them.
			for(TInt j=0; j<=i; j++)
				{
				startTick = User::FastCounter();
				TRAP(res, repositories[j] = CRepository::NewL(KUidClientOpenTestRepositoryIds[j]));
				endTick = User::FastCounter();
				openResults[i] += endTick - startTick;
				
				TESTL(res == KErrNone);
				TESTL(repositories[j] != NULL);
				CleanupStack::PushL(repositories[j]);
				}

			//close repositories in the reverse order, make sure cleanup stack is happy.
			for(TInt j=i; j>=0; j--)
				{
				startTick = User::FastCounter();
				CleanupStack::PopAndDestroy(repositories[j]);
				endTick = User::FastCounter();
				closeResults[i] += endTick - startTick;

				repositories[j] = NULL;
				}
			}
		}
		
	for(TInt i=0; i<KMaxRepositories; i++)
		{
		INFO_PRINTF4(_L("Total time spent to open %d times %d repositories is %f[ns]"), KIterationCount, i+1, FastCountToMicrosecondsInReal(openResults[i]));
		INFO_PRINTF4(_L("Total time spent to close %d times %d repositories is %f[ns]"), KIterationCount, i+1, FastCountToMicrosecondsInReal(closeResults[i]));
		}

	//enable cache back.
	res = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TESTL(res == KErrNone);
	
#endif	
	SetTestStepResult(EPass);

	return TestStepResult();
	}

