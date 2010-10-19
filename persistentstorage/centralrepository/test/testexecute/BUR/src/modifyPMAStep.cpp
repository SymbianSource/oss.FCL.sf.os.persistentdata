// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
*/
#include "modifyPMAStep.h"
#include "Te_centrep_BURSuiteDefs.h"

CmodifyPMAStep::~CmodifyPMAStep()
/**
 * Destructor
 */
	{
	}

CmodifyPMAStep::CmodifyPMAStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KmodifyPMAStep);
	}

TVerdict CmodifyPMAStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
    SetTestStepResult(EFail);	
	
#if defined(SYMBIAN_INCLUDE_APP_CENTRIC)
    CRepository* repository;
    TInt r;
	
    INFO_PRINTF2(_L("Start PMA ModifyPMAStep repos id: 0x%x"), KUidBURTestPMARepository);
	repository = CRepository::NewLC(KUidBURTestPMARepository);
    // Modify current entries
    r = repository->Set(KPMAIntKey, KPMAIntModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMARealKey, KPMARealModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMABinKey, KPMABinModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMAStrKey, KPMAStrModifiedValue);
    TESTL(r==KErrNone);
        

    // Delete keys
    r = repository->Delete(KPMADeleteIntKey);
    TESTL(r==KErrNone);
    
    r = repository->Create(KPMACreateIntKey, KPMACreateIntValue);
    TESTL(r==KErrNone);
    
    
    // Modify current entries
    r = repository->Set(KPMAInitIntKey, KPMAInitIntModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMAInitRealKey, KPMAInitRealModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMAInitBinKey, KPMAInitBinModifiedValue);
    TESTL(r==KErrNone);
    
    r = repository->Set(KPMAInitStrKey, KPMAInitStrModifiedValue);
    TESTL(r==KErrNone);

    CleanupStack::PopAndDestroy(repository);
#endif //defined(SYMBIAN_INCLUDE_APP_CENTRIC)

    SetTestStepResult(EPass);
    
	return TestStepResult();
	}
