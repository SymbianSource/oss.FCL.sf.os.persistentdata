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
#include "comparePMAStep.h"
#include "Te_centrep_BURSuiteDefs.h"
#include "Te_centrep_BURSuiteStepBase.h"

CcomparePMAStep::~CcomparePMAStep()
/**
 * Destructor
 */
	{
	}

CcomparePMAStep::CcomparePMAStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KcomparePMAStep);
	}

TVerdict CcomparePMAStep::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{
    SetTestStepResult(EFail);	

#if defined(SYMBIAN_INCLUDE_APP_CENTRIC)
	// Check on PMA repository to test that a repository which:
	// 1. Has not been modified and hence don't have repository  
	//    file on the PMA drive is correctly handled - This  
	//    should always contain just the ROM default values.
	// 2. Has been been modified (after backup) and hence has  
    //    repository file on the PMA drive is correctly handled  
    //    This should always contain the modified the PMA drive's 
	//    repository values (even after restore).
	INFO_PRINTF2(_L("Start PMA ComparePMAStep repos id: 0x%x"), KUidBURTestPMARepository);
	
	TBool PmaReposExist = EFalse;
	TBool bRet = GetBoolFromConfig(ConfigSection(), KIniPMAReposExist, PmaReposExist);
	TESTL(bRet);
    CRepository* repository(NULL);	
	TRAPD(r, repository = CRepository::NewL(KUidBURTestPMARepository));
    
    if(!PmaReposExist)
        {
        TESTL(r==KErrNotFound);
        }
    else
        {
        TESTL(r==KErrNone);
        CleanupStack::PushL(repository);
        // Int value
        // In modify step, we will modify an 'integer' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        TInt PmaExpectedResult ;
        bRet = GetIntFromConfig(ConfigSection(), KIniPMAExpectedGetResult, PmaExpectedResult);
        TBool PmaIsModified ;
        bRet = GetBoolFromConfig(ConfigSection(), KIniPMAIsModified, PmaIsModified);
        TInt PMAIntCurrentVal = 0;
        r=repository->Get(KPMAIntKey, PMAIntCurrentVal);
        TESTL(r==KErrNone);
        if(PmaIsModified)
            {
            TESTL(PMAIntCurrentVal == KPMAIntModifiedValue);
            }
        else
            {
            TESTL(PMAIntCurrentVal == KPMAIntOrigValue);
            }
        
        // Real value
        // In modify step, we will modify a 'real' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        TReal PMARealCurrentVal = 0;
        r=repository->Get(KPMARealKey, PMARealCurrentVal);
        TESTL(r==KErrNone); //ROM setting, should always exist
        if(PmaIsModified)
            {
            TESTL(PMARealCurrentVal == KPMARealModifiedValue);
            }
        else
            {
            TESTL(PMARealCurrentVal == KPMARealOrigValue);
            }
        
        // Bin value
        // In modify step, we will modify a 'string8(binary)' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        TBuf8 <20> PMABinCurrentVal;
        r=repository->Get(KPMABinKey, PMABinCurrentVal);
        TESTL(r==KErrNone); //ROM setting, should always exist
        if(PmaIsModified)
            {
            TESTL(PMABinCurrentVal == KPMABinModifiedValue);
            }
        else
            {
            TESTL(PMABinCurrentVal == KPMABinOrigValue);
            }
        
        // String value
        // In modify step, we will modify a 'string' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        TBuf <20> PMAStrCurrentVal;
        r=repository->Get(KPMAStrKey, PMAStrCurrentVal);
        TESTL(r==KErrNone); //ROM setting, should always exist
        if(PmaIsModified)
            {
            TESTL(PMAStrCurrentVal == KPMAStrModifiedValue);
            }
        else
            {
            TESTL(PMAStrCurrentVal == KPMAStrOrigValue);
            }
        
        //Deleted key
        // In modify step, we will delete a setting (after backup finished),
        // it must not be restored after restore is finished.
        // During other steps, this setting will not be deleted, hence it should still exist.
        r=repository->Get(KPMADeleteIntKey, PMAIntCurrentVal);
        if(PmaIsModified)
            {
            TESTL(r==KErrNotFound);
            }
        else
            {
            TESTL(r==KErrNone);
            TESTL(PMAIntCurrentVal==KPMADeleteIntValue);
            }
        
        //Created key
        // In modify step, we will create a new int setting (after backup finished),
        // it must still be exist after restore.
        // During other steps, this setting will not be created, hence it should not exist.
        r=repository->Get(KPMACreateIntKey, PMAIntCurrentVal);
        if(PmaIsModified)
            {
            TESTL(r==KErrNone);
            TESTL(PMAIntCurrentVal==KPMACreateIntValue);
            }
        else
            {
            TESTL(r==KErrNotFound);
            }
        
        
        
        //adsaddlkajdlkas
        r=repository->Get(KPMAInitIntKey, PMAIntCurrentVal);
        RDebug::Print(_L("r = %d, PmaExpectedResult = %d, PmaIsModified = %d"),r,PmaExpectedResult, PmaIsModified);
        RDebug::Print(_L("PMAIntCurrentVal = %d, KPMAInitIntOrigValue = %d, KPMAInitIntModifiedValue = %d"), PMAIntCurrentVal, KPMAInitIntOrigValue, KPMAInitIntModifiedValue);
        TESTL(r==PmaExpectedResult);
        if (r == KErrNone) 
            {
            if(PmaIsModified)
                {
                TESTL(PMAIntCurrentVal == KPMAInitIntModifiedValue);
                }
            else
                {
                TESTL(PMAIntCurrentVal == KPMAInitIntOrigValue);
                }
            }
        
        // Real value
        // In modify step, we will modify a 'real' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        r=repository->Get(KPMAInitRealKey, PMARealCurrentVal);
        RDebug::Print(_L("PMARealCurrentVal = %f, KPMAInitRealOrigValue = %f, KPMAInitRealModifiedValue = %f"), PMARealCurrentVal, KPMAInitRealOrigValue, KPMAInitRealModifiedValue);
        TESTL(r==PmaExpectedResult);
        if (r == KErrNone) 
            {
            if(PmaIsModified)
                {
                TESTL(PMARealCurrentVal == KPMAInitRealModifiedValue);
                }
            else
                {
                TESTL(PMARealCurrentVal == KPMAInitRealOrigValue);
                }
            }
        
        // Bin value
        // In modify step, we will modify a 'string8(binary)' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        r=repository->Get(KPMAInitBinKey, PMABinCurrentVal);
        RDebug::Printf("PMABinCurrentVal = %s, KPMAInitBinOrigValue = %s, KPMAInitBinModifiedValue = %s", &PMABinCurrentVal, &KPMAInitBinOrigValue, &KPMAInitBinModifiedValue);
        TESTL(r==PmaExpectedResult);
        if (r == KErrNone) 
            {
            if(PmaIsModified)
                {
                TESTL(PMABinCurrentVal == KPMAInitBinModifiedValue);
                }
            else
                {
                TESTL(PMABinCurrentVal == KPMAInitBinOrigValue);
                }
            }
        
        // String value
        // In modify step, we will modify a 'string' setting (after backup finished),
        // it must still contain the modified setting after restore.
        // During other steps, this setting will not be modified, hence it should 
        // still have the ROM value.
        r=repository->Get(KPMAInitStrKey, PMAStrCurrentVal);
        RDebug::Print(_L("PMAStrCurrentVal = %S, KPMAInitStrOrigValue = %S, KPMAInitStrModifiedValue = %S"), &PMAStrCurrentVal, &KPMAInitStrOrigValue, &KPMAInitStrModifiedValue);
        TESTL(r==PmaExpectedResult);
        if (r == KErrNone) 
            {
            if(PmaIsModified)
                {
                TESTL(PMAStrCurrentVal == KPMAInitStrModifiedValue);
                }
            else
                {
                TESTL(PMAStrCurrentVal == KPMAInitStrOrigValue);
                }
            }
        CleanupStack::PopAndDestroy(repository);
        }
#endif //defined(SYMBIAN_INCLUDE_APP_CENTRIC)

    SetTestStepResult(EPass);	

	return TestStepResult();
	}
