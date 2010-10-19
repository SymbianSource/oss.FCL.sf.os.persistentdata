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
#include <f32file.h>
#include <badesca.h>

#include "srvrepos_noc.h"
#include "srvres.h"
#include "t_cenrep_helper.h"

RTest TheTest(_L("t_cenrep_pma_array.exe"));

_LIT(KPMAKeyspace1Src, "z:\\private\\10202be9\\f1000001.pma");
_LIT(KPMAKeyspace2Src, "z:\\private\\10202be9\\f1000002.pma");
_LIT(KPMAKeyspace3Src, "z:\\private\\10202be9\\f1000003.pma");
_LIT(KPMAKeyspace4Src, "z:\\private\\10202be9\\f1000004.pma"); //Non-PMA keyspace in PMA drive
_LIT(KPMAKeyspace5Src, "z:\\private\\10202be9\\f1000006.txp");

_LIT(KPMAKeyspace1Dst, "c:\\private\\10202be9\\persists\\protected\\f1000001.cre");
_LIT(KPMAKeyspace2Dst, "c:\\private\\10202be9\\persists\\protected\\f1000002.cre");
_LIT(KPMAKeyspace3Dst, "c:\\private\\10202be9\\persists\\protected\\f1000003.cre");
_LIT(KPMAKeyspace4Dst, "c:\\private\\10202be9\\persists\\protected\\f1000004.cre");
_LIT(KPMAKeyspace5Dst, "c:\\private\\10202be9\\persists\\protected\\f1000006.txt");


const TUid KPMATestUid = {0xf1000005};
const TUid KNonPMATestUid = {0xf1000004};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions

LOCAL_C void Check( TInt aValue, TInt aExpected, TInt aLine )
    {
    if ( aValue != aExpected )
        {
        RDebug::Print( _L( "*** Expected error: %d, got: %d\r\n"), aExpected, aValue );
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"), err );
            }
        TheTest( EFalse, aLine );
        }
    }

#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
static void CloseTServerResources(TAny*)
    {
    TServerResources::Close();
    }

//////////////////////////////////////////////////////////////////////////////////////////////////
class CenrepPMAArrayTest :public CBase
{
public:
    void SetL(TUint32 aKey, TInt aValue);
    void GetL(TUint32 aKey, TInt& aValue);
    CenrepPMAArrayTest();
    ~CenrepPMAArrayTest();
    static CenrepPMAArrayTest* NewL(TUid aReposUid);
private:
    void ConstructL(TUid aReposUid);
public:
    CServerRepository* iServerRepo;
    CSessionNotifier* iSessionNotif;
};


CenrepPMAArrayTest::CenrepPMAArrayTest(){}

CenrepPMAArrayTest::~CenrepPMAArrayTest()
    {
    if (iServerRepo)
        {
        if (iSessionNotif)
            {
            iServerRepo->Close();
            }
        delete iServerRepo;
        }
    if (iSessionNotif)
        delete iSessionNotif;
    }

CenrepPMAArrayTest* CenrepPMAArrayTest::NewL(TUid aReposUid)
    {
    CenrepPMAArrayTest* self=new (ELeave)CenrepPMAArrayTest;
    CleanupStack::PushL(self);
    self->ConstructL(aReposUid);
    CleanupStack::Pop(self);
    return self;
    }

void CenrepPMAArrayTest::ConstructL(TUid aReposUid)
    {
    iServerRepo=new (ELeave)CServerRepository();
    iSessionNotif=new (ELeave) CSessionNotifier();

    iServerRepo->OpenL(aReposUid,*iSessionNotif);
    }

void CenrepPMAArrayTest::SetL(TUint32 aKey, TInt aValue)
    {
    iServerRepo->StartTransaction(EConcurrentReadWriteTransaction);
    iServerRepo->CleanupCancelTransactionPushL();
    iServerRepo->TransactionSetL(aKey, aValue);
    CleanupStack::Pop();
    TUint32 keyInfo;
    User::LeaveIfError(iServerRepo->CommitTransaction(keyInfo));
    }

void CenrepPMAArrayTest::GetL(TUint32 aKey, TInt& aValue)
    {
    User::LeaveIfError(iServerRepo->Get(aKey, aValue));
    }

/**
@SYMTestCaseID          PDS-CENTRALREPOSITORY-CT-4120
@SYMTestCaseDesc        PMA repositories array test on pre-loaded PMA drive 
@SYMTestPriority        High
@SYMTestActions         - Cleanup PMA drive and copy 5 test repositories to the 
                          PMA drive. 1 of them is .txt and should be ignored on 
                          initialisation and another 1 is a non-PMA keyspace 
                          which is counted as a not supported keyspace if found in the 
                          PMA drive.
                        - Initialise server resource
                        - Check the preloaded repositories count in the array
                        - Modify a PMA repository that only exist in ROM (not persisted yet), 
                          check the count should increase by 1 after successful modification
                        - Modify the same PMA repository array again, the count 
                          should not be increased again
@SYMTestExpectedResults - On initialise count should be 4 (including the Non-PMA keyspace in PMA drive)
                        - On modification of  a PMA repository that only exist in ROM (not persisted yet),
                          the array should be 5 after successful modification
                        - On the 2nd modification of the same repository, the 
                          array should still be 5
@SYMREQ                 REQ42876
*/
LOCAL_C void ArrayPreLoadedAndSetTestL()
    {
    TheTest.Next( _L( " @SYMTestCaseID:PDS-CENTRALREPOSITORY-CT-4120 PMA Repositories Array test on pre-loaded PMA drive" ) );
    CleanupCDriveL();
    
    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);
    
    CFileMan* fm = CFileMan::NewL( rfs );
    CleanupStack::PushL( fm );
    
    //copying test files to PMA drive for the server startup simulation
    CopyTestFilesL(*fm, KPMAKeyspace1Src, KPMAKeyspace1Dst);
    CopyTestFilesL(*fm, KPMAKeyspace2Src, KPMAKeyspace2Dst);
    CopyTestFilesL(*fm, KPMAKeyspace3Src, KPMAKeyspace3Dst);
    CopyTestFilesL(*fm, KPMAKeyspace4Src, KPMAKeyspace4Dst); //Non-PMA keyspace
    CopyTestFilesL(*fm, KPMAKeyspace5Src, KPMAKeyspace5Dst); //Txt keyspace
    
    CleanupStack::PopAndDestroy(2, &rfs);
    
    
    TUint32 pmaIntKey = {0x01};
    TInt currentVal = 0;
    TInt pmaCount = 0;
    
    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
    
    pmaCount = TServerResources::iPMADriveRepositories.Count();
    TEST2(pmaCount, 4); //Initial load, there should be 4 repositories in PMA array    
    
    CenrepPMAArrayTest* theTest=CenrepPMAArrayTest::NewL(KPMATestUid);
    CleanupStack::PushL(theTest);
    
    // Part 1. Persist a repository
    theTest->SetL(pmaIntKey, 25); //Setting a value will trigger persist on the ROM repository to PMA Drive
    theTest->GetL(pmaIntKey, currentVal); //Double check that the value has indeed changed
    TEST2(currentVal, 25);
    pmaCount = TServerResources::iPMADriveRepositories.Count();
    TEST2(pmaCount, 5); //The array count should be 5 now 
    
    // Part 2. Persist the same repository
    theTest->SetL(pmaIntKey, 30); //Calling set on the same repository 
    theTest->GetL(pmaIntKey, currentVal); //Double check that the value has indeed changed
    TEST2(currentVal, 30);
    pmaCount = TServerResources::iPMADriveRepositories.Count();
    TEST2(pmaCount, 5); //The array count should stay the same
    
    CleanupStack::PopAndDestroy(theTest);
    
    TRAPD(err, theTest = CenrepPMAArrayTest::NewL(KNonPMATestUid)); 
    TEST2(err, KErrNotSupported);
    
    pmaCount = TServerResources::iPMADriveRepositories.Count();
    //The array count should be less by 1 to 4 because the non-PMA 
    // keyspace should be deleted and removed from the array.
    TEST2(pmaCount, 4);
    
    CleanupStack::Pop(); // CloseTServerResources
    
    TServerResources::Close();
    
    }

/**
@SYMTestCaseID          PDS-CENTRALREPOSITORY-CT-4121
@SYMTestCaseDesc        PMA repositories array test on empty PMA drive 
@SYMTestPriority        High
@SYMTestActions         - Cleanup PMA drive
                        - Initialise server resource
                        - Check the preloaded repositories count in the array
                        - Modify a ROM-only PMA repository, check the count should increase by 1
                        - Modify the same PMA repository array again, the count should not be increased again.
@SYMTestExpectedResults - On initialise count should be 0
                        - On modification of a ROM-only PMA repository, the array should be 1
@SYMREQ                 REQ42876
*/
LOCAL_C void ArraySetTestL()
    {
    TheTest.Next( _L( " @SYMTestCaseID:PDS-CENTRALREPOSITORY-CT-4121 PMA Repositories Array test on empty PMA drive" ) );
    CleanupCDriveL();
    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
    
    TInt pmaCount = TServerResources::iPMADriveRepositories.Count();
    TEST2(pmaCount, 0);
    
    CenrepPMAArrayTest* theTest=CenrepPMAArrayTest::NewL(KPMATestUid);
    CleanupStack::PushL(theTest);
    
    TUint32 pmaIntKey = {0x01};
    theTest->SetL(pmaIntKey, 40);

    pmaCount = TServerResources::iPMADriveRepositories.Count();
    
    TEST2(pmaCount, 1);
    
    CleanupStack::PopAndDestroy(theTest);
    CleanupStack::Pop(); // CloseTServerResources
    
    TServerResources::Close();
    
    }

LOCAL_C void MainL()
    {
    CActiveScheduler* scheduler = new(ELeave)CActiveScheduler;
    CActiveScheduler::Install(scheduler);
    
    CleanupStack::PushL(scheduler);

    //Array was preloaded during startup
    ArrayPreLoadedAndSetTestL();
    
    //Array was not preloaded during startup
    ArraySetTestL();
    
    CleanupCDriveL();

    CleanupStack::PopAndDestroy(scheduler);
    }


TInt E32Main()
    {
    TheTest.Title ();
    TheTest.Start( _L( "PMA Array Tests" ) );
    
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TheTest(cleanup != NULL);
    
    __UHEAP_MARK;
        
    TRAPD(err, MainL());
    TEST2(err, KErrNone);
    
    __UHEAP_MARKEND;
    
    TheTest.End ();
    TheTest.Close ();
    
    delete cleanup;
        
    User::Heap().Check();
    return KErrNone;
    }

