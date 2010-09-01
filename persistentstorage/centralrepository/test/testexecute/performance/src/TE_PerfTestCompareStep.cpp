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

#include "TE_PerfTestCompareStep.h"
#include "t_cenrep_helper.h"
#include <centralrepository.h>
#include "srvreqs.h"
#include "srvdefs.h"
#include "cachemgr.h"

//--------------------------------
// class CPerfTestCompareStep
//--------------------------------

CPerfTestCompareStep::CPerfTestCompareStep() 
	{
	SetTestStepName(KPerfTestCompareStep);
	}

// doTestStepL
// Implement the pure virtual function.
// Comapre the results of previous timing tests
TVerdict CPerfTestCompareStep::doTestStepL()
	{
#ifndef __CENTREP_SERVER_CACHETEST__
	return EPass;
#else	
    SetTestStepResult(EFail);

	// To make sure future tests in this harness run under caching
	TInt r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));

	TPtrC resultSection1;
	_LIT(KSharedName1, "shared_name_1");
	TInt bRet = GetStringFromConfig(ConfigSection(), KSharedName1, resultSection1);
	TESTL(bRet==1);

	TPtrC resultSection2;
	_LIT(KSharedName2, "shared_name_2");
	bRet = GetStringFromConfig(ConfigSection(), KSharedName2, resultSection2);
	TESTL(bRet==1);
	
	TBuf<KMaxFileName> sharedString; 
		
	ReadSharedDataL(resultSection1, sharedString);
	
	ReadResults(iResults1, sharedString);
	sharedString.Zero();
	ReadSharedDataL(resultSection2, sharedString);
	ReadResults(iResults2, sharedString);
	
	TESTL(iResults1.Count()==iResults2.Count());
	
	for(TInt i=0; i<iResults1.Count(); i++)
		{
		TESTL(iResults1[i].iUseCount==iResults2[i].iUseCount);
#if defined __WINS__ || defined __WINSCW__
		// This test is deactivated for emulator builds.
      		// Emulator build runs produce inconsistent results when running this 
	      	// test on ONB machines, probably because of uncontrolled CPU and disk 
      		// activity on the host system caused by previous build and test commands
#else	
		// Expect an increase in performance
		TESTL(iResults1[i].iSumElapsedTicks<iResults2[i].iSumElapsedTicks);
#endif
		}
	
	SetTestStepResult(EPass);		
	
	return TestStepResult();
#endif //__CENTREP_SERVER_CACHETEST__	
	}

void CPerfTestCompareStep::ReadResults(RArray<TSummary>& aArray, TBuf<KMaxFileName>& aString) 
	{
	TLex analyse(aString);
	
	TInt items;
	TSummary summary;	

	for(TInt i=0; i<2; i++)
		{
		analyse.Val(items);
		analyse.SkipSpaceAndMark();
		for(TInt i=0;i<items;i++)
			{
			analyse.SkipCharacters();		
			TPtrC token = analyse.MarkedToken();
			summary.iName.Copy(token);
			analyse.SkipSpaceAndMark();
			analyse.Val(summary.iUseCount);
			analyse.SkipSpaceAndMark();
			analyse.Val(summary.iSumElapsedTicks,EDecimal);
			analyse.SkipSpaceAndMark();
			aArray.Append(summary);
			}
		}
	}
