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

#include "t_cenrep_helper.h"
#include <centralrepository.h>
#include <e32test.h>  // RTest
#include <e32debug.h> // RDebug::Printf
#include <f32file.h>  // RFs

#include "../cenrepsrv/srvparams.h" //KServerUid3

_LIT(KSoftReset, "--SoftReset");

RTest TheTest(_L("t_cenrep_pma_rfs.exe"));

const TUid KUidPMARepROMOnly = { 0xf1000501 };
const TUid KUidPMARepInstallOnly = { 0xf1000502 };
const TUid KUidPMARepROMAndInstall = { 0xf1000503 };
const TUid KUidNonPMARep = { 0xf1000504 };

_LIT(KInstallOnlySource, "Z:\\private\\10202BE9\\f1000502.txi");
_LIT(KInstallOnlyTarget, "C:\\private\\10202BE9\\f1000502.txt");
            
_LIT(KInstallOnlyPmaSource, "Z:\\private\\10202BE9\\f1000502.pma");
_LIT(KInstallOnlyPmaTarget, "C:\\private\\10202BE9\\persists\\protected\\f1000502.cre");
            
_LIT(KRomAndInstallSource, "Z:\\private\\10202BE9\\f1000503.txi");
_LIT(KRomAndInstallTarget, "C:\\private\\10202BE9\\f1000503.txt");

typedef enum
    {
    EInstallOnly,
    ERomAndInstall,
    } TRepositoryFileState;

const TUint32 KInt1 = 1;
const TInt KInt1_UpdatedValue = 73;

const TUint32 KNewInt = 1000;
const TUint32 KNewInt2 = 0x0FFF; // outside range meta (in default meta)
const TUint32 KNewInt3 = 0x1000; // inside range meta

const TUint32 KReal1 = 2;

const TUint32 KString1 = 5;
_LIT(KString1_UpdatedValue, "another one");

const TUint32 KIntNonPMA = 1;
const TInt KIntNonPMA_InitialValue = 100;
const TInt KIntNonPMA_UpdatedValue = 102;


LOCAL_C void Check(TInt aValue, TInt aLine)
    {
    if(!aValue)
        {
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"), err );
            }
        TheTest(EFalse, aLine);
        }
    }
LOCAL_C void Check(TInt aValue, TInt aExpected, TInt aLine)
    {
    if(aValue != aExpected)
        {
        RDebug::Print(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"), err );
            }
        TheTest(EFalse, aLine);
        }
    }
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

// This function kills the C32exe.exe process. This commsdat process will
// interfere with the test if not killed. In a nutshell, some of the test cases 
// will kill and then wait for 2 seconds and restart the centrep server 
// with --SoftReset option. During that 2 seconds wait sometimes C32exe.exe 
// will use centrep API, thus starting the server normally without --SoftReset.
LOCAL_C void KillC32Exe()
    {
    _LIT( KC32ServerName, "c32exe");
    KillProcess(KC32ServerName); // Don't need to check the return code, it always return KErrNone anyway.
    User::After(KGeneralDelay);
    }

LOCAL_C void KillCentrepExe()
    {
    _LIT( KCentralRepositoryServerName, "Centralrepositorysrv");
    KillProcess(KCentralRepositoryServerName); // Don't need to check the return code, it always return KErrNone anyway.
    User::After(KGeneralDelay);
    }

//This function restores the state of the files required for this test
//Existing files are deleted and then the required files are copied
//back from the Z drive to the c drive
LOCAL_C void RestoreRFSTestFilesL(TRepositoryFileState aState)
    {
    //Delete all files from C:\\private\\10202BE9\\persists\\ dir
    //and C:\\private\\10202BE9\\ dir
    CleanupCDriveL();
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);

    CFileMan* fm = CFileMan::NewL(fs);
    CleanupStack::PushL(fm);
        
    switch(aState)
    {
        case EInstallOnly:
            CopyTestFilesL(*fm,KInstallOnlySource, KInstallOnlyTarget);
            //Since we can't open and modify a PMA keyspace in install directory
            // we have to copy a pre-persisted version to the PMA drive.
            CopyTestFilesL(*fm,KInstallOnlyPmaSource, KInstallOnlyPmaTarget);
            break;

        case ERomAndInstall:
            CopyTestFilesL(*fm,KRomAndInstallSource, KRomAndInstallTarget);
            break;

        default:
            break;
    }

    CleanupStack::PopAndDestroy(2);

    }

//
// Start the server process or thread
//
LOCAL_C TInt ReStartServerInSoftResetMode()
    {
    const TUidType serverUid(KNullUid, KNullUid, KServerUid3);

    //
    // EPOC and EKA2 is easy, we just create a new server process. Simultaneous
    // launching of two such processes should be detected when the second one
    // attempts to create the server object, failing with KErrAlreadyExists.
    //
    RProcess server;
    TInt r=server.Create(KServerImg,
                         KSoftReset,
                         serverUid);

    if (r != KErrNone)
        {
        return r;
        }

    TRequestStatus stat;
    server.Rendezvous(stat);

    if (stat != KRequestPending)
        {
        server.Kill(0);        // abort startup
        }
    else
        {
        server.Resume();    // logon OK - start the server
        }

    User::WaitForRequest(stat);        // wait for start or death
    // we can't use the 'exit reason' if the server panicked as this
    // is the panic 'reason' and may be '0' which cannot be distinguished
    // from KErrNone
    r = (server.ExitType() == EExitPanic) ? KErrGeneral : stat.Int();

    server.Close();
    return r;
    }


LOCAL_C void RestoreFactorySettingsTestL( TUid aRepUid )
    {
    TheTest.Next(_L(" RestoreFactorySettingsTestL "));
    TInt r;
    TInt i;
    TBuf<20> str;
    
    //Kill centrep server to pick new test files
    KillCentrepExe();

    TheTest.Printf(_L("Open repository to ensure server is running"));
    CRepository* repository = CRepository::NewLC(aRepUid);

    TheTest.Printf(_L("Add a Setting"));
    const TInt KIntValue = 1234;
    r = repository->Create(KNewInt, KIntValue);
    TEST2(r, KErrNone);
    r = repository->Create(KNewInt2, KIntValue);
    TEST2(r, KErrNone);
    r = repository->Create(KNewInt3, KIntValue);
    TEST2(r, KErrNone);

    TheTest.Printf(_L("Delete a PMA Setting"));
    r = repository->Delete(KReal1);
    TEST2(r, KErrNone);

    TheTest.Printf(_L("Modify PMA Setting"));
    r = repository->Set(KInt1, KInt1_UpdatedValue);
    TEST2(r, KErrNone);
    r = repository->Set(KString1, KString1_UpdatedValue);
    TEST2(r, KErrNone);

    // Close repository
    CleanupStack::PopAndDestroy(repository);
    
    // Open a Non-PMA rep to prove that the RFS actually did happened later
    repository = CRepository::NewLC(KUidNonPMARep);
    
    TheTest.Printf(_L("Modify a Non-PMA and RFS-enabled setting"));
    r = repository->Set(KIntNonPMA, KIntNonPMA_UpdatedValue);
    TEST2(r, KErrNone);
    CleanupStack::PopAndDestroy(repository);

    TheTest.Printf(_L("Kill the server process"));
    _LIT( KCentralRepositoryServerName, "Centralrepositorysrv");
    r = KillProcess(KCentralRepositoryServerName);
    TEST2(r,KErrNone);

    User::After(KGeneralDelay);

    TheTest.Printf(_L("Manually start central respository in softreset mode"));
    ReStartServerInSoftResetMode();

    TheTest.Printf(_L("Re-create the repository to ensure server is running"));
    repository = CRepository::NewLC(aRepUid);

    TheTest.Printf(_L("Get 'Added' value"));
    r = repository->Get(KNewInt, i);
    TEST2(r, KErrNone);
    r = repository->Get(KNewInt2, i);
    TEST2(r, KErrNone);
    r = repository->Get(KNewInt3, i);
    TEST2(r, KErrNone);

    TReal real;
    TheTest.Printf(_L("Get 'Deleted' value"));
    r = repository->Get(KReal1, real);
    TEST2(r, KErrNotFound);

    TheTest.Printf(_L("Get 'Modified' value"));
    r = repository->Get(KInt1, i);
    TEST2(r, KErrNone);
    TEST(i == KInt1_UpdatedValue);

    r = repository->Get(KString1, str);
    TEST2(r, KErrNone);
    TEST(str == KString1_UpdatedValue);

    // Close repository
    CleanupStack::PopAndDestroy(repository);
    
    repository = CRepository::NewLC(KUidNonPMARep);
    TheTest.Printf(_L("Check the Non-PMA and RFS-enabled setting"));
    //If RFS did happen, this value should now be reverted back to the initial ROM value
    r = repository->Get(KIntNonPMA, i);
    TEST2(r, KErrNone);
    TEST(i == KIntNonPMA_InitialValue);
    CleanupStack::PopAndDestroy(repository);
    }

/**
@SYMTestCaseID          PDS-CENTRALREPOSITORY-CT-4128
@SYMTestCaseDesc        Restore Factory Setting test on PMA keyspaces
@SYMTestPriority        High
@SYMTestActions         - Add a PMA setting.
                        - Delete a PMA setting.
                        - Modify a PMA Setting.
                        - Modify a Non-PMA and RFS-enabled setting.
                        - Kill and manually restart central respository server with --SoftReset option.
                        - Check the aftermath of the RFS.
@SYMTestExpectedResults - Added PMA setting should still be there.
                        - Delete PMA setting should still be missing.
                        - Modified PMA setting should have the modified value.
                        - Modified Non-PMA setting should have its value reverted. 
@SYMREQ                 REQ42876
*/
LOCAL_C void MainL()
    {
    TheTest.Next(_L(" @SYMTestCaseID:PDS-CENTRALREPOSITORY-CT-4128 RFS Test on PMA keyspaces"));
    RFs fs;
    TEntry entry;
    TInt err = fs.Connect();
    TEST2(err, KErrNone);
    CleanupClosePushL(fs);
    
    CleanupCDriveL();
    KillC32Exe(); //Need to kill C32Exe as it is interfering with the test.
    KillCentrepExe();
    
    RestoreFactorySettingsTestL(KUidPMARepROMOnly);
    
    RestoreRFSTestFilesL(EInstallOnly);
    RestoreFactorySettingsTestL(KUidPMARepInstallOnly);
    err = fs.Entry(KInstallOnlyTarget,entry);//The PMA file in install dir should have been deleted during RFS
    TEST2(err, KErrNotFound);
    
    RestoreRFSTestFilesL(ERomAndInstall);
    RestoreFactorySettingsTestL(KUidPMARepROMAndInstall);
    err = fs.Entry(KRomAndInstallTarget,entry);//The PMA file in install dir should have been deleted during RFS
    TEST2(err, KErrNotFound);
    
    CleanupCDriveL();
    
    CleanupStack::PopAndDestroy(); //fs
    }

TInt E32Main()
    {
    TheTest.Title ();
    TheTest.Start(_L("PMA Restore Factory Settings tests "));
    
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
