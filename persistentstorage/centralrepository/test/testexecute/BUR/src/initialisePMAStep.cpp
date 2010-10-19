// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @SYMTestSuiteName SYSLIB-CENTRALREPOSITORY-TE_CENTREP_BURSUITE
 @SYMScriptTestEnvironment this script requires Techview ROM for BURTestserver dependency. BURTestserver must be built for this test to run.
*/
#include "initialisePMAStep.h"
#include "Te_centrep_BURSuiteDefs.h"

CinitialisePMAStep::~CinitialisePMAStep()
/**
 * Destructor
 */
	{
	}

CinitialisePMAStep::CinitialisePMAStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KinitialisePMAStep);
	}

TVerdict CinitialisePMAStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
    SetTestStepResult(EFail);

 	
#if defined(SYMBIAN_INCLUDE_APP_CENTRIC)
	INFO_PRINTF2(_L("Start PMA InitialiseStep repos id: 0x%x"), KUidBURTestPMARepository);
	
    CRepository* repository;
    repository = CRepository::NewLC(KUidBURTestPMARepository);

    TInt r = repository->Create(KPMAInitIntKey, KPMAInitIntOrigValue);
    TESTL(r==KErrNone);
    
    r = repository->Create(KPMAInitRealKey, KPMAInitRealOrigValue);
    TESTL(r==KErrNone);
    
    r = repository->Create(KPMAInitBinKey, KPMAInitBinOrigValue);
    TESTL(r==KErrNone);
    
    r = repository->Create(KPMAInitStrKey, KPMAInitStrOrigValue);
    TESTL(r==KErrNone);
    
    CleanupStack::PopAndDestroy(repository);
        //}
#endif //defined(SYMBIAN_INCLUDE_APP_CENTRIC)

    SetTestStepResult(EPass);
	
	return TestStepResult();
	}

