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
#include "srvreqs.h"
#include "TE_PerfTestCacheDisabledStep.h"

//---------------------------------
// class CPerfTestCacheDisabledStep
//---------------------------------

CPerfTestCacheDisabledStep::CPerfTestCacheDisabledStep() 
	{
	SetTestStepName(KPerfTestCacheDisabledStep);
	}

// doTestStepL
// Implement the pure virtual function.
// This accesses a small and a large repository various times when cache is 
// enabled and disabled.
TVerdict CPerfTestCacheDisabledStep::doTestStepL()
	{
#ifndef __CENTREP_SERVER_CACHETEST__
	return EPass;
#else	
    SetTestStepResult(EFail);	

	TInt r;
	
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TESTL(r==KErrNone);
	
	CommonTestStepL();
	
	return TestStepResult();	
#endif	
	}

