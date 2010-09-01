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
// TE_PerfTestCacheEnabledStep.cpp
// 
//

#include "TE_PerfTestCacheStepBase.h"
#include <centralrepository.h>
#include "t_cenrep_helper.h"
#include "srvreqs.h"

//--------------------------------
// class CPerfTestCacheStepBase
//--------------------------------

#if defined(__CENTREP_SERVER_CACHETEST__) && defined(__CENTREP_SERVER_PERFTEST__)
const TUid KUidCacheTestRepositorySmTxt		= { 0x00000002 };
const TUid KUidCacheTestRepositoryLrgTxt	= { 0xCCCCCC01 };
const TUid KUidCacheTestRepositorySmCre		= { 0x22222221 };
const TUid KUidCacheTestRepositoryLrgCre	= { 0xCCCCCC02 };
#endif

// CommonTestStepL
// This function accesses a small and a large repository various times 
// when cache is enabled and disabled.
void CPerfTestCacheStepBase::CommonTestStepL()
	{
	SetTestStepResult(EFail);

#if defined(__CENTREP_SERVER_CACHETEST__) && defined(__CENTREP_SERVER_PERFTEST__)
	TInt r = SetGetParameters(TIpcArgs(ERestartPerfTests));
	TEST(r==KErrNone);
	
 	CRepository* repository;
 	TInt i;
 	// Access small text rep 5 times in a row
	for(i=0; i<5; i++)
		{
		User::LeaveIfNull(repository = CRepository::NewL(KUidCacheTestRepositorySmTxt));
		delete repository;
		repository = NULL;
		User::After(200000); // delay 200ms before access
		}

 	// Access large text rep 5 times in a row
	for(i=0; i<5; i++)
		{
		User::LeaveIfNull(repository = CRepository::NewL(KUidCacheTestRepositoryLrgTxt));
		delete repository;
		repository = NULL;
		User::After(200000); // delay 200ms before access
		}

 	// Access small binary rep 20 times in a row
	for(i=0; i<20; i++)
		{
		User::LeaveIfNull(repository = CRepository::NewL(KUidCacheTestRepositorySmCre));
		delete repository;
		repository = NULL;
		User::After(200000); // delay 200ms before access
		}

 	// Access large binary rep 20 times in a row
	for(i=0; i<20; i++)
		{
		User::LeaveIfNull(repository = CRepository::NewL(KUidCacheTestRepositoryLrgCre));
		delete repository;
		repository = NULL;
		User::After(200000); // delay 200ms before access
		}
	SetTestStepResult(EPass);

#endif
	}
