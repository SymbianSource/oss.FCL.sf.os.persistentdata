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

#include "TE_PerfTestServer.h"
#include "TE_PerfTestStep.h"
#include "TE_MemTestStep.h"
#include "TE_FindPerfTestStep.h"
#include "TE_PerfTestCacheEnabledStep.h"
#include "TE_PerfTestCacheDisabledStep.h"
#include "TE_PerfTestCompareStep.h"
#include "TE_DefectTestStep.h"
#include "TE_PerfTestClientOpenStep.h"
#include "TE_NotifyMemTestStep.h"

_LIT(KServerName, "CentRepPerfTest");

CPerfTestServer* CPerfTestServer::NewL()
	{
	CPerfTestServer* server = new (ELeave) CPerfTestServer();
	CleanupStack::PushL(server);
	// CServer base class call
	server->StartL(KServerName);
	CleanupStack::Pop(server);
	return server;
	}
	
LOCAL_C void MainL()
	{
	// Leave the hooks in for platform security
#if (defined __DATA_CAGING__)
	RProcess().DataCaging(RProcess::EDataCagingOn);
	RProcess().SecureApi(RProcess::ESecureApiOn);
#endif
	
	CActiveScheduler* sched=NULL;
	sched=new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(sched);
	CPerfTestServer* server = NULL;
	// Create the CTestServer derived server
	TRAPD(err,server = CPerfTestServer::NewL());
	if (KErrNone == err)
		{
		// Sync with the client and enter the active scheduler
		RProcess::Rendezvous(KErrNone);
		sched->Start();
		}
	delete server;
	delete sched;
	}

GLDEF_C TInt E32Main()
	{
	CTrapCleanup* cleanup = CTrapCleanup::New();
	if(cleanup == NULL)
		{
		return KErrNoMemory;
		}
	TRAPD(err,MainL());
	delete cleanup;
	return err;
    }

CTestStep* CPerfTestServer::CreateTestStep(const TDesC& aStepName)
	{
	CTestStep* testStep = NULL;

	if (aStepName == KGetPerfTestResults)
		{
		testStep = new CPerfTestStep();
		}
	else if (aStepName == KMemTestName)
		{
		testStep = new CMemTestStep();
		}
	else if (aStepName == KFindPerfTestName)
		{
		testStep = new CFindPerfTestStep();
		}
	else if (aStepName == KPerfTestCacheEnabledStep)
		{
		testStep = new CPerfTestCacheEnabledStep();
		}
	else if (aStepName == KPerfTestCacheDisabledStep)
		{
		testStep = new CPerfTestCacheDisabledStep();
		}
	else if (aStepName == KPerfTestCompareStep)
		{
		testStep = new CPerfTestCompareStep();
		}
	else if (aStepName == KPerfTestDefectStepDEF057491)
		{
		testStep = new CPerfTestDefectStep057491();
		}
	else if (aStepName == KPerfTestDefectStepDEF059633)
		{
		testStep = new CPerfTestDefectStep059633();
		}
	else if (aStepName == KPerfTestClientOpenName)
		{
		testStep = new CPerfTestClientOpenStep();
		}
	else if (aStepName == KNotifyMemTestStep)
		{
		testStep = new CNotifyMemTestStep();
		}

	return testStep;
	}
	
