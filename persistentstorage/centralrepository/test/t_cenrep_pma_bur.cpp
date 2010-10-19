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
#include "backup.h"
#include "t_cenrep_helper.h"

RTest TheTest(_L("t_cenrep_pma_bur.exe"));

enum FileSet
	{
	EOriginals,
	EChanged,
	ERemoved,
	EDEF058823L
	};

CRepositoryBackupClient* backupClient;

CActiveScheduler* globalAS;

const TUid KUidPmaBackupTestRepository1 = { 0xf1000201 };
const TUid KUidPmaBackupTestRepository2 = { 0xf1000202 };
const TUid KUidPmaBackupTestRepository3 = { 0xf1000203 };
const TUid KUidPmaBackupTestRepository4 = { 0xf1000204 };
const TUid KUidPmaBackupTestRepository5 = { 0xf1000205 };

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions

LOCAL_C void DeleteFilesL()
	{
	_LIT( KOldInstallFiles, "c:\\private\\102081E4\\*.*" );
	_LIT( KOldPersistFiles, "c:\\private\\102081E4\\persists\\*.*" );
	_LIT( KOldPMAFiles,     "c:\\private\\102081E4\\persists\\protected\\*.*" );

	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
	CFileMan* fm = CFileMan::NewL( fs );
	CleanupStack::PushL( fm );
	
	TInt r = KErrNone;

	r = fm->Attribs( KOldInstallFiles, KEntryAttArchive, KEntryAttReadOnly, TTime( 0 ), CFileMan::ERecurse );
    if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
        User::Leave(r);
	r = fm->Delete( KOldInstallFiles );
	if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
		User::Leave(r);
	
	r = fm->Attribs( KOldPersistFiles, KEntryAttArchive, KEntryAttReadOnly, TTime( 0 ), CFileMan::ERecurse );
	if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
	    User::Leave(r);
	r = fm->Delete( KOldPersistFiles );
	if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
	    User::Leave(r);
    
	r = fm->Attribs( KOldPMAFiles, KEntryAttArchive, KEntryAttReadOnly, TTime( 0 ), CFileMan::ERecurse );
	if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
	        User::Leave(r);
    r = fm->Delete( KOldPMAFiles );
	if ( r != KErrNone && r != KErrNotFound && r != KErrPathNotFound )
		User::Leave(r);

	CleanupStack::PopAndDestroy( 2 ); //fs and fm
	}

LOCAL_C void Check( TInt aValue, TInt aLine )
	{
	if ( !aValue )
		{
		TRAPD(err, DeleteFilesL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** DeleteFilesL also failed with error %d expecting KErrNone\r\n"), err );
            }
		TheTest( EFalse, aLine );
		}
	}

LOCAL_C void Check( TInt aValue, TInt aExpected, TInt aLine )
	{
	if ( aValue != aExpected )
		{
		RDebug::Print( _L( "*** Expected error: %d, got: %d\r\n"), aExpected, aValue );
		TRAPD(err, DeleteFilesL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** DeleteFilesL also failed with error %d expecting KErrNone\r\n"), err );
            }
		TheTest( EFalse, aLine );
		}
	}

#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

static void CloseTServerResources(TAny*)
    {
    TServerResources::Close();
    }

LOCAL_C void InstallFileSetL()
	{
	//There shouldn't be PMA keyspace in PMA drive that doesn't have its ROM equivalent
	_LIT(KPmaCreReposInPMADriveSrc,    "z:\\private\\10202BE9\\f1000202.pma");
	_LIT(KPmaCreReposInPMADriveTgt,    "c:\\private\\102081E4\\persists\\protected\\f1000202.cre");
	_LIT(KPmaCreReposInPersistDirSrc,  "z:\\private\\10202BE9\\f1000203.crp");
	_LIT(KPmaCreReposInPersistDirTgt,  "c:\\private\\102081E4\\persists\\f1000203.cre");
	_LIT(KPmaTxtReposInInstallDirSrc,  "z:\\private\\10202BE9\\f1000204.txi");
	_LIT(KPmaTxtReposInInstallDirTgt,  "c:\\private\\102081E4\\f1000204.txt");
	_LIT(KPmaCreReposInInstallDirSrc,  "z:\\private\\10202BE9\\f1000205.cri");
	_LIT(KPmaCreReposInInstallDirTgt,  "c:\\private\\102081E4\\f1000205.cre");
	
	DeleteFilesL();
	// When the contents of the repository directories change, the cached iOwnerIdLookUpTable becomes invalid
    TServerResources::iOwnerIdLookUpTable.Reset();
	    
	RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);
    
    CFileMan* fm = CFileMan::NewL( rfs );
    CleanupStack::PushL( fm );
    
    CopyTestFilesL(*fm, KPmaCreReposInPMADriveSrc, KPmaCreReposInPMADriveTgt);
    CopyTestFilesL(*fm, KPmaCreReposInPersistDirSrc, KPmaCreReposInPersistDirTgt);
    CopyTestFilesL(*fm, KPmaTxtReposInInstallDirSrc, KPmaTxtReposInInstallDirTgt);
    CopyTestFilesL(*fm, KPmaCreReposInInstallDirSrc, KPmaCreReposInInstallDirTgt);
    
    
    CleanupStack::PopAndDestroy(2, &rfs);
	}

LOCAL_C void CleanupActiveScheduler(TAny* aShc)
	{
	CActiveScheduler::Replace( globalAS );
	delete aShc;
	}

LOCAL_C void BackupRepositoryL( )
	{
	backupClient->CompleteOwnerIdLookupTableL();
	TEST2(TServerResources::FindOwnerIdLookupMapping(KUidPmaBackupTestRepository1.iUid), KErrNotFound);
	TEST2(TServerResources::FindOwnerIdLookupMapping(KUidPmaBackupTestRepository2.iUid), KErrNotFound);
	TEST2(TServerResources::FindOwnerIdLookupMapping(KUidPmaBackupTestRepository3.iUid), KErrNotFound);
	TEST2(TServerResources::FindOwnerIdLookupMapping(KUidPmaBackupTestRepository4.iUid), KErrNotFound);
	TEST2(TServerResources::FindOwnerIdLookupMapping(KUidPmaBackupTestRepository5.iUid), KErrNotFound);
	}

/**
@SYMTestCaseID SYSLIB-CENTRALREPOSITORY-UT-4174
@SYMTestCaseDesc Verify iOwnerIdLookupTable does not contain PMA keyspaces before backup.
@SYMTestPriority High
@SYMTestActions  Copy PMA keyspaces to PMA drive, persists and install directory.
                 Call CompleteOwnerIdLookupTableL().
                 Call FindOwnerIdLookupMapping to verify iOwnerIdLookTable does not contain PMA keyspaces.
@SYMTestExpectedResults iOwnerIdLookTable list does not contain PMA keyspaces.
@SYMREQ 42876
*/
LOCAL_C void OwnerIdLookupTableTestsL()
	{
    TheTest.Next( _L( " @SYMTestCaseID:PDS-CENTRALREPOSITORY-UT-4174 OwnerIdLookupTable test on PMA repository " ) );

	// create and install the active scheduler we need
	CActiveScheduler* s = new(ELeave) CActiveScheduler;
	
	TCleanupItem tc(CleanupActiveScheduler, s);
	CleanupStack::PushL(tc);
	
	CActiveScheduler::Replace( s );

	backupClient = CRepositoryBackupClient::NewLC( TServerResources::iFs );
	TEST( backupClient != 0 );

	// These tests don't test Backup&Restore functionality over Secure Backup Server so cache management
	// is not possible. For that reason, cache is disabled manually.
	TServerResources::iCacheManager->DisableCache();

	// Install known files
	InstallFileSetL();
	BackupRepositoryL( );

	DeleteFilesL();
	CleanupStack::PopAndDestroy( backupClient );
	// Cleanup the scheduler
	CleanupStack::PopAndDestroy( s );
	}

LOCAL_C void DoTestsL()
	{
	TServerResources::InitialiseL();
	
	CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
	TheTest.Next( _L( "OwnerIdLookupTable tests" ) );
    OwnerIdLookupTableTestsL();
    
    CleanupStack::Pop(1); //TServerResources
	TServerResources::Close();
	}


LOCAL_C void MainL()
	{
	// create and install the active scheduler we need for the cache manager in TServerResources::InitialiseL
	globalAS=new(ELeave) CActiveScheduler;
	CleanupStack::PushL(globalAS);
	CActiveScheduler::Install(globalAS);

	DoTestsL();
	DeleteFilesL();

	CleanupStack::PopAndDestroy(globalAS);

	}

TInt E32Main()
	{
    TheTest.Title ();
    TheTest.Start( _L( "PMA Backup and restore tests" ) );
    
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

