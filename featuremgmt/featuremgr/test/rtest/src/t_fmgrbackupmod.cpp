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

#include <e32test.h>
#include <e32debug.h>
#include <bautils.h>
#include <featurecontrol.h>
#include "t_fmgrbursim.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("t_fmgrbur test"));

const TUint threadTimeout = 1500000;    // thread timeout = 1.5 second 

static RSemaphore MainThreadCrS;
static TInt featMgrIsResponsive = 0;

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine, TBool aPrintThreadName = EFalse)
    {
    if(!aValue)
        {
        //DeleteTestFiles();
        if(aPrintThreadName)
            {
            RThread th;
            TName name = th.Name();
            RDebug::Print(_L("*** Thread %S, Line %d\r\n"), &name, aLine);
            }
        else
            {
            RDebug::Print(_L("*** Line %d\r\n"), aLine);
            }
        TheTest(EFalse, aLine);
        }
    }

void Check2(TInt aValue, TInt aExpected, TInt aLine, TBool aPrintThreadName = EFalse)
    {
    if(aValue != aExpected)
        {
        //DeleteTestFiles();
        if(aPrintThreadName)
            {
            RThread th;
            TName name = th.Name();
            RDebug::Print(_L("*** Thread %S, Line %d Expected error: %d, got: %d\r\n"), &name, aLine, aExpected, aValue);
            }
        else
            {
            RDebug::Print(_L("*** Line %d, Expected error: %d, got: %d\r\n"), aLine, aExpected, aValue);
            }
        TheTest(EFalse, aLine);
        }
    }
#define TEST(arg) ::Check1((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)
#define TTEST(arg) ::Check1((arg), __LINE__, ETrue)
#define TTEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__, ETrue)

// ------------------------- ------------------------- 
// setup and cleanup functions

TInt TestModifyThreadL(void*)
    {
        __UHEAP_MARK;
        
        CTrapCleanup* tc = CTrapCleanup::New();
        // Modify a feature. We don't care about its result. 
        //  It is fine as long as it returns.
        RFeatureControl rfc;
        TEST2( rfc.Connect(), KErrNone );
        rfc.EnableFeature( TUid::Uid(0x00000001) );
        rfc.Close();
        featMgrIsResponsive = 1;
        RDebug::Print(_L("+++:TestModifyThread: Modification completed\r\n"));
        MainThreadCrS.Signal();
        delete tc;
        
        __UHEAP_MARKEND;
        
        return KErrNone;
    }
/**
@SYMTestCaseID          PDS-FEATMGR-CT-XXXX
@SYMTestCaseDesc        Querying a feature during backup operation. Verify that a 
                        response is returned from the server during backup.
@SYMTestPriority        High
@SYMTestActions         Start simulating backup operation
                        Create a thread that will query a feature
                        Verify that a response is received in not more than 1.5 second.
                        Otherwise the test fail.          
@SYMTestExpectedResults Test must not fail
@SYMREQ                 
*/  
void TestBackupModificationResponseL()
    {
        _LIT(KThreadName, "MdfTh");
        featMgrIsResponsive = 0;
        
        CFeatMgrBURSim* simulate = CFeatMgrBURSim::NewLC();
        RThread modifyThread;
        TRequestStatus modifyStatus;
        
        CleanupClosePushL( modifyThread );
        
        
        // Simulate a backup
        RDebug::Print(_L("Simulating Backup of BUR\r\n"));
        simulate->Simulate_StartBackupL();

        TEST2( modifyThread.Create(KThreadName, &TestModifyThreadL, 0x2000, 0x1000, 0x10000, NULL, EOwnerProcess), KErrNone );
        modifyThread.Logon(modifyStatus);
        TEST2( modifyStatus.Int(), KRequestPending );
        modifyThread.Resume();
        // Wait for 1.5 second for the query thread to finish. 
        RDebug::Print(_L("+++:MainThread: Wait for modification completion...\r\n"));
        MainThreadCrS.Wait(threadTimeout);
        // The modification request should NOT be responsive within the 1.5 second frame.
        // It should only be responsive after back up ended.
        TEST2 (featMgrIsResponsive, 0);
        simulate->Simulate_EndBackupL();
        
        MainThreadCrS.Wait(threadTimeout);
        
        // The modification request now must be responsive within the 1.5 second frame.
        TEST2 (featMgrIsResponsive, 1);
        
        CleanupStack::PopAndDestroy(&modifyThread);
        CleanupStack::PopAndDestroy(simulate);
    }

///////////////////////////////////////////////////////////////////////////////////////

void DoTestsL()
	{
    MainThreadCrS.CreateLocal(0);
    
    TheTest.Start(_L(" @SYMTestCaseID:PDS-FEATMGR-CT-XXXX Backup Modification Response"));
    TestBackupModificationResponseL();
    
    MainThreadCrS.Close();

	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	
	__UHEAP_MARK;
	
	TRAPD(err, DoTestsL());
	TEST2(err, KErrNone);

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;
	
	User::Heap().Check();
	return KErrNone;
	}
