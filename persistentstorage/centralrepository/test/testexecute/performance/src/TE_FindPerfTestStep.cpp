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

#include "TE_FindPerfTestStep.h"
#include "TE_PerfTestUtilities.h"
#include <centralrepository.h>
#include "centralrepositoryinternal.h"
#include <hal.h>
#include <hal_data.h>

using namespace NCentralRepositoryConstants;

const TUid KUidFindPerfTestRepositorySmTxt = { 0x00000105 };

CFindPerfTestStep::CFindPerfTestStep()
	{
	SetTestStepName(KFindPerfTestName);
	}
	
// doTestStepL
// This test fetches the memory data collected by
// CentRep server, processes it and reports the result.
TVerdict CFindPerfTestStep::doTestStepL()
	{
	SetTestStepResult(EFail);
	
 	CRepository* repository = NULL;
	TRAPD(res, repository = CRepository::NewL(KUidFindPerfTestRepositorySmTxt));
	TESTL(res == KErrNone);
	TESTL(repository != NULL);
	CleanupStack::PushL(repository);

	//if the KCentRepFindBufSize has been increased to a value more then 16 then test 
	//repository "KUidFindPerfTestRepositorySmTxt" needs to be updated otherwise this 
	//test will start failing. In this case the repository should be updated to have
	//more entries.
	TESTL(KCentRepFindBufSize<=16);

	RArray<TUint32> foundIds;
	TUint32 accumulatedTicks = 0;
	for(TInt j=1; j<=KCentRepFindBufSize; j++)
		{
		accumulatedTicks = 0;
		for(TInt i=0; i<100; i++)
			{
			TInt r = KErrNone;
			TUint32 startTick = User::FastCounter();
			r = repository->FindEqL(0, 0, j, foundIds);
			TUint32 endTick = User::FastCounter();

			//check for errors
			TESTL(r == KErrNone);
			//check that number of found ids is as expected
			TESTL(foundIds.Count() == j);
						
			accumulatedTicks += endTick - startTick;
			}
		
		INFO_PRINTF3(_L("Total time spent to find settings of count %d is %f[ms]"), j, FastCountToMillisecondsInReal(accumulatedTicks));
		}

	//now run it one more time for settings of count more than KCentRepFindBufSize
	for(TInt j=1; j<=KCentRepFindBufSize; j++)
		{
		accumulatedTicks = 0;
		for(TInt i=0; i<100; i++)
			{
			TInt r = KErrNone;
			TUint32 startTick = User::FastCounter();
			r = repository->FindNeqL(0, 0, j, foundIds);
			TUint32 endTick = User::FastCounter();
			
			//check for errors
			TESTL(r == KErrNone);
			//check that number of found ids is as expected
			TESTL(foundIds.Count() > KCentRepFindBufSize);
			
			accumulatedTicks += endTick - startTick;
			}
		
		INFO_PRINTF3(_L("Total time spent to find settings of count %d is %f[ms]"), foundIds.Count(), FastCountToMillisecondsInReal(accumulatedTicks));
		}
		
	CleanupStack::PopAndDestroy(repository);
	repository = NULL;

	SetTestStepResult(EPass);
	
	return TestStepResult();
	}

