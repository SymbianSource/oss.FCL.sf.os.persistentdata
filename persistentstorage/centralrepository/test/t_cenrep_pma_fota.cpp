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
#include "cachemgr.h"
#include "sessnotf.h"
#include "t_cenrep_helper.h"

RTest TheTest(_L("t_cenrep_pma_fota.exe"));
_LIT(KCachedVersionFile,"C:\\private\\10202be9\\romversion\\romversion_info.txt");
_LIT(KModifiedSwVersion, "z:\\private\\10202be9\\sw_modified.txt");
_LIT(KPMADriveCre,"C:\\private\\10202be9\\persists\\protected\\f1000602.cre");

const TUid KUidRepository = { 0xf1000601 };

const TUint32 KModifiedIntKey = 1;
const TUint32 KModifiedRealKey = 2;
const TUint32 KModifiedStringKey = 3;
const TUint32 KDeletedIntKey = 4;
const TUint32 KNewIntKey = 5;

const TInt KModifiedIntValue = 123;
const TReal KModifiedRealValue = 20.23;
_LIT(KModifiedStringValue, "modified string");

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
LOCAL_C void Check(TInt aValue, TInt aLine)
    {
    if (!aValue)
        {
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print(_L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"),err);
            }
        TheTest(EFalse, aLine);
        }
    }

LOCAL_C void Check(TInt aValue, TInt aExpected, TInt aLine)
    {
    if (aValue != aExpected)
        {
        RDebug::Print(_L( "*** Expected error: %d, got: %d\r\n"), aExpected,aValue);
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print(_L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"),err);
            }
        TheTest(EFalse, aLine);
        }
    }

#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

LOCAL_C void CopyTestFilesL()
    {
    _LIT( KPMAInstallRepSource, "z:\\private\\10202BE9\\f10006*.txi" );
    _LIT( KPMAInstallRepTarget, "c:\\private\\10202BE9\\*.txt" );
    _LIT( KPMAModifiedRepSource, "z:\\private\\10202BE9\\f10006*.pma" );
    _LIT( KPMAModifiedRepTarget, "c:\\private\\10202BE9\\persists\\protected\\*.cre" );

    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);
    CFileMan* fm = CFileMan::NewL(rfs);
    CleanupStack::PushL(fm);

    CopyTestFilesL(*fm, KPMAInstallRepSource, KPMAInstallRepTarget);
    CopyTestFilesL(*fm, KPMAModifiedRepSource, KPMAModifiedRepTarget);

    CleanupStack::PopAndDestroy(2, &rfs);
    }

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
static void CloseTServerResources(TAny*)
    {
    TServerResources::Close();
    }

LOCAL_C void CleanUp()
    {
    // If cache manager is initialized and used before, we flush it
    if (TServerResources::iCacheManager)
        {
        TServerResources::iCacheManager->FlushCache();
        }
    TServerResources::iObserver->CloseiOpenRepositories();
    TServerResources::iObserver->Reset();
    TServerResources::iOwnerIdLookUpTable.Reset();
    User::After(KGeneralDelay);
    }

LOCAL_C void VerifyPMAUnchangedByFOTAL()
    {
    //Check the merge content for correctness.
    CSessionNotifier* notifier = new (ELeave) CSessionNotifier;
    CleanupStack::PushL(notifier);

    CServerRepository* repository = new (ELeave) CServerRepository();
    CleanupStack::PushL(repository);

    repository->OpenL(KUidRepository, *notifier);

    //Get the user added setting, it should still be there.
    TInt intValue = 0;
    TInt r = repository->Get(KNewIntKey, intValue);
    TEST(r == KErrNone);
    //Get the user deleted setting, it should still be missing.
    r = repository->Get(KDeletedIntKey, intValue);
    TEST(r == KErrNotFound);

    //Get user modified settings. It should still contain the user modification
    r = repository->Get(KModifiedIntKey, intValue);
    TEST(r == KErrNone);
    TEST(intValue == KModifiedIntValue);

    TReal realValue = 0.0;
    r = repository->Get(KModifiedRealKey, realValue);
    TEST(r == KErrNone);
    TEST(realValue == KModifiedRealValue);

    TBuf8<50> stringValue;
    r = repository->Get(KModifiedStringKey, stringValue);
    TEST(r == KErrNone);
    TPtr16 str16((TUint16*) stringValue.Ptr(), stringValue.Length() / 2,stringValue.Length() / 2);
    TEST(str16.Compare(KModifiedStringValue) == 0);

    repository->Close();
    CleanupStack::PopAndDestroy(2);//repository, notifier
    }

/**
 @SYMTestCaseID          PDS-CENTRALREPOSITORY-CT-4122
 @SYMTestCaseDesc        FOTx update test on PMA keyspaces
 @SYMTestPriority        High
 @SYMTestActions         Copy a PMA cre file to PMA drive that contains different value from the ROM keyspace.
 Call CheckROMReflashL to simulate a reboot of cenrep.
 Check that all the setting in the PMA cre file is not modified at all to the ROM value.
 @SYMTestExpectedResults After calling CheckROMReflashL setting values in the PMA drive's cre file should not changed
 @SYMREQ                 REQ42876
 */
LOCAL_C void PMARepFOTxL()
    {
    TheTest.Next(_L( " @SYMTestCaseID:PDS-CENTRALREPOSITORY-CT-4122 PMA FOTx update test" ));//Force a rom update for this repository.

    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));

    CFileMan* fm = CFileMan::NewL(TServerResources::iFs);
    CleanupStack::PushL(fm);

    //Initial cleanup
    //Copy of modified stored rom version info
    User::LeaveIfError(fm->Copy(KModifiedSwVersion, KCachedVersionFile,CFileMan::EOverWrite));
    User::LeaveIfError(fm->Attribs(KCachedVersionFile, 0, KEntryAttReadOnly,TTime(0)));
    CleanupStack::PopAndDestroy();//fm
    
    User::After(KGeneralDelay);
    // flush the cache manager contents.
    CleanUp();

    CServerRepository::CheckROMReflashL();

    VerifyPMAUnchangedByFOTAL();
    
    CleanupStack::Pop(); // CloseTServerResources
    
    TServerResources::Close();
    }

/**
 @SYMTestCaseID          PDS-CENTRALREPOSITORY-CT-4123
 @SYMTestCaseDesc        FOTx test on PMA keyspaces with no basis in install directory or rom
 @SYMTestPriority        High
 @SYMTestActions         Copy a repository into the PMA drive that has no rom or SWI equivalent.
 Call CheckROMReflashL to simulate a reboot of cenrep.
 Verify that the persisted repository is NOT removed.
 @SYMTestExpectedResults Persisted repository is not removed during rom update.
 @SYMREQ                 REQ42876
 */
LOCAL_C void NoRomNoInstallPMARepFOTxL()
    {
    TheTest.Next(_L( " @SYMTestCaseID:PDS-CENTRALREPOSITORY-CT-4123 PMA FOTx delete test" ));
    
    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
    
    CFileMan* fm = CFileMan::NewL(TServerResources::iFs);
    CleanupStack::PushL(fm);

    //Copy of modified stored rom version info
    User::LeaveIfError(fm->Copy(KModifiedSwVersion, KCachedVersionFile,CFileMan::EOverWrite));
    User::LeaveIfError(fm->Attribs(KCachedVersionFile, 0, KEntryAttReadOnly,TTime(0)));
    User::After(KGeneralDelay);
    CleanupStack::PopAndDestroy();//fm

    CServerRepository::CheckROMReflashL();
    TEntry entry;

    TEST(TServerResources::iFs.Entry(KPMADriveCre, entry) == KErrNone);
    
    CleanupStack::Pop(); //CloseTServerResources
    
    TServerResources::Close();

    }

LOCAL_C void DoTestsL()
    {
    PMARepFOTxL();
    NoRomNoInstallPMARepFOTxL();
    }

LOCAL_C void MainL()
    {
    CopyTestFilesL();

    // create and install the active scheduler we need
    CActiveScheduler* s = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(s);
    CActiveScheduler::Install(s);

    DoTestsL();

    CleanupCDriveL();

    // Cleanup the scheduler
    CleanupStack::PopAndDestroy(s);
    }

TInt E32Main()
    {
    TheTest.Title ();
    TheTest.Start(_L( "PMA Fota Unit Tests" ));
    
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

