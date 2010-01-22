// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <bautils.h>
#include <sqldb.h>
#include <e32math.h>

#include "SqlBur.h"
#include "t_sqlbur.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("SQL Backup and Restore Test"));

_LIT(KPrivateDir, "C:\\private\\10281e17\\");

const TUid KClientUid = {0x21212122}; // the data owner's UID

_LIT(KBackupDir, "C:\\TEST\\");
_LIT(KBackupFile, "C:\\TEST\\Backup.bak");
_LIT(KBackupFile2Z, "Z:\\TEST\\t_sqlbur_backup_ver0.bak");
_LIT(KBackupFile2, "C:\\TEST\\t_sqlbur_backup_ver0.bak");

const TUint KBufferSize = 2048; // used for reading backup files for validation

static CActiveScheduler* TheScheduler = NULL;
static CSqlBurTestHarness* TheTestHarness = NULL;

/////////////////////////////////////

const TInt KMaxDbFileSize = 10 * 1024;//The max test db file size
const TInt KTestDbFileCnt = 2;

//Test db files
_LIT(KTestFileName1,"[21212122]AADB2.db");//Created outside this test app
_LIT(KTestFileName2,"[21212122]BBDB2.db");//Created outside this test app
_LIT(KTestDbFileName1,"C:[21212122]AADB2.db");
_LIT(KTestDbFileName2,"C:[21212122]BBDB2.db");

const TPtrC KTestFileNames[KTestDbFileCnt] = {KTestFileName1(), KTestFileName2()};

static TInt TheDbFileSizes[KTestDbFileCnt];//An array where the real db file size will be stored
static TUint8 TheDbFileData[KTestDbFileCnt][KMaxDbFileSize];//An array where the original db file content will be stored
static TUint8 TheBuf[KMaxDbFileSize];

/////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvDestroy()
	{
	delete TheTestHarness;
	TheTestHarness = NULL;		
	
	delete TheScheduler;
	TheScheduler = NULL;
	}

////////////////////////////
// Test macros and functions
////////////////////////////

void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		TestEnvDestroy();
		TheTest(EFalse, aLine);
		}
	}

void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TestEnvDestroy();
		RDebug::Print(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

//CSqlBurTestHarness - test implementation of the MSqlSrvBurInterface, implemented in the production code by the SQL server.
CSqlBurTestHarness *CSqlBurTestHarness::New()
	{
	CSqlBurTestHarness* self = new CSqlBurTestHarness;
	TEST(self != NULL);
	self->Construct();
	return self;
	}

CSqlBurTestHarness::CSqlBurTestHarness()
	{
	}

void CSqlBurTestHarness::Construct()
	{
	TInt err = iFs.Connect();
	TEST2(err, KErrNone);
	err = iFs.MkDir(KBackupDir);
	TEST(err == KErrNone || err == KErrAlreadyExists);
	err = iFs.CreatePrivatePath(EDriveC);
	TEST2(err, KErrNone);
	}

CSqlBurTestHarness::~CSqlBurTestHarness()
	{
	(void)iFs.Delete(KBackupFile);
	iFs.Close();
	}

//Called by the backup client ot get a list of database files to backup
//The array is owned by the caller
//The SQL server would have the job to get a list of databases owned by
//the given SID and to determine whether the backup flag is set
//All databases that satisfy this requirement will be added to the array
void CSqlBurTestHarness::GetBackUpListL(TSecureId /*aUid*/, RArray<TParse>& aFileList)
	{
	//TheTest.Printf(_L("Getting backup file list for SID=%x\r\n"),aUid);
	for(TInt i=0;i<KTestDbFileCnt;++i)
		{
		TParse parse;
		parse.Set(KTestFileNames[i], &KPrivateDir, NULL);
		aFileList.AppendL(parse);
		}
	}

//Notification that a backup is starting
TBool CSqlBurTestHarness::StartBackupL(const RArray<TParse>& /*aFileList*/) 
	{
	//TheTest.Printf(_L("Start \"backup\". %d files in the list.\r\n"), aFileList.Count());
	return ETrue;
	}

//Notification that a backup has ended
void CSqlBurTestHarness::EndBackup(const RArray<TParse>& /*aFileList*/)
	{
	//TheTest.Printf(_L("End \"backup\". %d files in the list.\r\n"), aFileList.Count());
	}

//Notification that a restore is starting
TBool CSqlBurTestHarness::StartRestoreL(TSecureId /*aUid*/) 
	{
	//TheTest.Printf(_L("Start \"restore\" for UID=%X\r\n"), aUid);
	return ETrue;
	}

//Notification that a restore has ended
void CSqlBurTestHarness::EndRestore(TSecureId /*aUid*/) 
	{
	//TheTest.Printf(_L("End \"restore\" for UID=%X\r\n"), aUid);
	}
	
//Returns the file system resource handle to the caller.
RFs& CSqlBurTestHarness::Fs()
	{
	return iFs;
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

TBool FileExists(RFs& aFs, const TDesC& aFileName)
	{
	TEntry entry;
	return aFs.Entry(aFileName, entry) == KErrNone;
	}

void TestEnvCreate()
	{
	TheScheduler = new CActiveScheduler;
	TEST(TheScheduler != NULL);
	
	CActiveScheduler::Install(TheScheduler);

	TheTestHarness = CSqlBurTestHarness::New();
	TEST(TheTestHarness != NULL);
	}

//Reads the content of the db files and stores the content to a global memory buffer.
//That buffer content will be sued later for  a verification of the restore process.
void StoreDbContentToBuf(RFs& aFs)
	{
	for(TInt i=0;i<KTestDbFileCnt;++i)
		{
		RFile dbFile;
		TInt err = dbFile.Open(aFs, KTestFileNames[i], EFileRead);
		TEST2(err, KErrNone);
		
		TInt fileSize = 0;
		err = dbFile.Size(fileSize);
		TEST2(err, KErrNone);
		TEST(fileSize > 0);

		TPtr8 bufptr(TheDbFileData[i], 0, KMaxDbFileSize);
		err = dbFile.Read(bufptr, fileSize);
		TEST2(err, KErrNone);
		TEST(fileSize == bufptr.Length());

		TheDbFileSizes[i] = fileSize;
		
		dbFile.Close();
		}
	}

//At the moment when this function is called, the db files content is already restored.
//The function will open the restored db files and compare their content against the content
//of the original db files (kept in a global memory buffer).
void CompareDbContentWithBuf(RFs& aFs)
	{
	for(TInt i=0;i<KTestDbFileCnt;++i)
		{
		TEST(TheDbFileSizes[i] > 0);
		
		RFile dbFile;
		TInt err = dbFile.Open(aFs, KTestFileNames[i], EFileRead);
		TEST2(err, KErrNone);
		
		TInt fileSize = 0;
		err = dbFile.Size(fileSize);
		TEST2(err, KErrNone);
		TEST(fileSize > 0);
		TEST(TheDbFileSizes[i] == fileSize);

		TPtr8 bufptr(TheBuf, 0, KMaxDbFileSize);
		err = dbFile.Read(bufptr, fileSize);
		TEST2(err, KErrNone);
		TEST(fileSize == bufptr.Length());

		err = Mem::Compare(TheBuf, fileSize, TheDbFileData[i], TheDbFileSizes[i]);
		TEST2(err, 0);

		dbFile.Close();
		}
	}

////////////////////////////////////////////////////////////////////////////////////////

//The backup client will return a series of data chunks representing
//one of more databases for the uid of the data owner.
//This data is stored in a file on the C drive for the purposes of the test
TInt TestBackupL(CSqlBackupClient &aBackupClient, RFs& aFs, TInt aDataChunkSize = KBufferSize)
	{
	RFile file;
	CleanupClosePushL(file);
	TInt err = file.Replace(aFs, KBackupFile, EFileWrite | EFileStream | EFileShareExclusive);
	User::LeaveIfError(err);
	aBackupClient.InitialiseGetProxyBackupDataL(KClientUid, EDriveC);
	
	TBuf8<KBufferSize> buf;
	TPtr8 ptr((TUint8*)buf.Ptr(), aDataChunkSize);
	TBool finishedFlag = EFalse;
	TInt count = 0;
	
	do
		{
		aBackupClient.GetBackupDataSectionL(ptr, finishedFlag);
		count += ptr.Length();
		err = file.Write(ptr);
		User::LeaveIfError(err);
		ptr.SetLength(0);
		}
	while(!finishedFlag);

	CleanupStack::PopAndDestroy(&file);
	
	if(count == 0)
		{
		User::Leave(KErrEof);
		}
	if(!FileExists(aFs, KBackupFile))
		{
		User::Leave(KErrNotFound);
		}
	TheTest.Printf(_L("Backup complete. %d bytes processed.\r\n"), count);
	return count;
	}

//This sends the data in chunks form back to the BUR client
//for nupacking and restoration of the original databases files
TInt TestRestoreL(CSqlBackupClient &aRestoreClient, RFs& aFs, TInt aDataChunkSize = KBufferSize)
	{
	RFile file;
	CleanupClosePushL(file);
	TInt err = file.Open(aFs, KBackupFile, EFileRead | EFileShareExclusive);
	User::LeaveIfError(err);
	aRestoreClient.InitialiseRestoreProxyBaseDataL(KClientUid, EDriveC);
	
	TBuf8<KBufferSize> buf;
	TPtr8 ptr((TUint8*)buf.Ptr(), aDataChunkSize);
	TBool finishedFlag = EFalse;
	
	TInt fileSize = 0;
	err = file.Size(fileSize);
	User::LeaveIfError(err);
	TInt count = fileSize;
	
	do
		{
		err = file.Read(ptr, aDataChunkSize);
		User::LeaveIfError(err);
		fileSize -= ptr.Length();
		finishedFlag = fileSize == 0;
		aRestoreClient.RestoreBaseDataSectionL(ptr, finishedFlag);
		ptr.SetLength(0);
		} 
	while(fileSize > 0);
	
	CleanupStack::PopAndDestroy(&file);
	
	aRestoreClient.RestoreComplete(EDriveC);
	
	if(!finishedFlag)
		{
		User::Leave(KErrEof);
		}
	for(TInt i=0;i<KTestDbFileCnt;++i)
		{
		if(!FileExists(aFs, KTestFileNames[i]))
			{
			User::Leave(KErrNotFound);
			}
		}
		
	TheTest.Printf(_L("Restore complete. %d bytes processed.\r\n"), count);
	return count;
	}

//Verifies the integrity of the backup file.
void TestArchiveIntegrityL(CSqlBackupClient &aBackupClient, RFs& aFs)
	{
	RFile bkpFile;
	CleanupClosePushL(bkpFile);
	
	TInt err = bkpFile.Open(aFs, KBackupFile, EFileRead | EFileShareExclusive);
	User::LeaveIfError(err);
	
	TBuf8<KBufferSize> buf;
	TPtr8 ptr((TUint8*)buf.Ptr(), buf.MaxLength());
	
	TInt bkpFileSize = 0;
	err = bkpFile.Size(bkpFileSize);
	User::LeaveIfError(err);
	
	while(bkpFileSize > 0)
		{
		// get the checksum
		err = bkpFile.Read(ptr, 16); // 8 UTF-16 characters
		User::LeaveIfError(err);
		if(ptr.Length() != 16)
			{
			User::Leave(KErrCorrupt);
			}
		TPtr ptr16((TUint16*) ptr.Ptr(), 8, 8);
		TLex lex(ptr16);
		TUint32 checksum;
		lex.SkipSpace();
		err = lex.Val(checksum, EHex);
		User::LeaveIfError(err);
		bkpFileSize -= 16;

		// get the old file size
		err = bkpFile.Read(ptr, 16); // 8 UTF-16 characters
		User::LeaveIfError(err);
		if(ptr.Length() != 16)
			{
			User::Leave(KErrCorrupt);
			}
		ptr16.Set((TUint16*) ptr.Ptr(), 8, 8);
		lex.Assign(ptr16);
		TUint32 oldFileSize;
		lex.SkipSpace();
		err = lex.Val(oldFileSize, EHex);
		User::LeaveIfError(err);
		bkpFileSize -= 16;

		// get the backup file header version
		err = bkpFile.Read(ptr, 8); // 4 UTF-16 characters
		User::LeaveIfError(err);
		ptr16.Set((TUint16*)ptr.Ptr(), 4, 4);
		lex.Assign(ptr16);
		TUint32 hdrVer;
		lex.SkipSpace();
		err = lex.Val(hdrVer, EHex);
		User::LeaveIfError(err);
		bkpFileSize -= 8;

		// get the file size
		err = bkpFile.Read(ptr, 32); // 16 UTF-16 characters
		User::LeaveIfError(err);
		if(ptr.Length() != 32)
			{
			User::Leave(KErrCorrupt);
			}
		ptr16.Set((TUint16*) ptr.Ptr(), 16, 16);
		lex.Assign(ptr16);
		TInt64 fileSize;
		lex.SkipSpace();
		err = lex.Val(fileSize, EHex);
		User::LeaveIfError(err);
		bkpFileSize -= 32;

		// get the filename size
		err = bkpFile.Read(ptr, 16); // 8 UTF-16 characters
		User::LeaveIfError(err);
		ptr16.Set((TUint16*)ptr.Ptr(), 8, 8);
		lex.Assign(ptr16);
		TUint32 fileNameSize;
		lex.SkipSpace();
		err = lex.Val(fileNameSize, EHex);
		User::LeaveIfError(err);
		bkpFileSize -= 16;

		// get the filename
		err = bkpFile.Read(ptr, fileNameSize * 2); // fileName UTF-16 characters
		User::LeaveIfError(err);
		if(ptr.Length() != (fileNameSize * 2))
			{
			User::Leave(KErrCorrupt);
			}
		ptr16.Set((TUint16*) ptr.Ptr(), fileNameSize, fileNameSize);
		lex.Assign(ptr16);
		TParse tp;
		tp.Set(ptr16, NULL, NULL);
		TPtrC dbFileName = tp.Name();
		bkpFileSize -= fileNameSize * 2;
	
		// open a local file - replaces any previous one
		RFile64 dbFile;
		CleanupClosePushL(dbFile);
		err = dbFile.Replace(aFs, dbFileName, EFileWrite | EFileShareExclusive);
		User::LeaveIfError(err);
	
		// copy all the data (file size bytes)
		TInt bytesLeftToRead = fileSize;

		while(bytesLeftToRead > 0)
			{
			TInt readSize = bytesLeftToRead > KBufferSize ? KBufferSize : bytesLeftToRead;
			err = bkpFile.Read(ptr, readSize);
			User::LeaveIfError(err);
			if(ptr.Length() != readSize)
				{
				User::Leave(KErrCorrupt);	
				}
			bytesLeftToRead -= readSize;
			err = dbFile.Write(ptr, readSize);
			User::LeaveIfError(err);
			}

		bkpFileSize -= fileSize;
		
		// checksum the file
		TUint32 dbChecksum = aBackupClient.CheckSumL(dbFile) & 0xFFFFFFFF;
		
		if(checksum != dbChecksum)
			{
			User::Leave(KErrCorrupt);
			}
			
		// all done with this file
		CleanupStack::PopAndDestroy(&dbFile);
		err = aFs.Delete(dbFileName);
		User::LeaveIfError(err);
		}
	
	CleanupStack::PopAndDestroy(&bkpFile);
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4002
@SYMTestCaseDesc		Test for DEF113598 - "SQL, t_sqlbur unit test needs refactoring"
						The test backups 2 test db files, then verifies the backup file integrity,
						then restores the test db files content from the backup file.
						At the end, the test checks that the restored test db files content is the
						same as the content of the original test db file.
@SYMTestPriority		High
@SYMTestActions			Test for DEF113598 - "SQL, t_sqlbur unit test needs refactoring"
@SYMTestExpectedResults Test must not fail
@SYMDEF					DEF113598
*/	
void FunctionalTest()
	{
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4002 Backup: functional test "));
	
	CSqlBackupClient* backupClient = NULL;
	TRAPD(err, backupClient = CSqlBackupClient::NewL(TheTestHarness));
	TEST(backupClient != NULL);

	////////////////////////////////////////

	const TDriveNumber KDrive = EDriveC;
	
	//Virtual functions - with default implementation
	
	(void)backupClient->GetExpectedDataSize(KDrive);

	(void)backupClient->GetDataChecksum(KDrive);
	
	TBool finished = EFalse;
	TPtr8 ptr(0, 0, 0);
	TRAP(err, backupClient->GetSnapshotDataL(KDrive, ptr, finished));
	TEST2(err, KErrNotSupported);

	TRAP(err, backupClient->InitialiseGetBackupDataL(KDrive));
	TEST2(err, KErrNotSupported);

	TRAP(err, backupClient->InitialiseRestoreBaseDataL(KDrive));
	TEST2(err, KErrNotSupported);

	TRAP(err, backupClient->InitialiseRestoreIncrementDataL(KDrive));
	TEST2(err, KErrNotSupported);

	TPtrC8 ptr2(KNullDesC8);
	TRAP(err, backupClient->RestoreIncrementDataSectionL(ptr2, finished));
	TEST2(err, KErrNotSupported);

	TRAP(err, backupClient->AllSnapshotsSuppliedL());
	TEST2(err, KErrNone);

	TRAP(err, backupClient->ReceiveSnapshotDataL(KDrive, ptr2, finished));
	TEST2(err, KErrNotSupported);

	backupClient->TerminateMultiStageOperation();

	////////////////////////////////////////

	TInt bytesStored = 0;
	TRAP(err, bytesStored = TestBackupL(*backupClient, TheTestHarness->Fs()));
	TEST2(err, KErrNone);

	TheTest.Next(_L("Archive integrity test"));
	
	TRAP(err, TestArchiveIntegrityL(*backupClient, TheTestHarness->Fs()));
	TEST2(err, KErrNone);

	delete backupClient;

	TheTest.Next(_L("Restore: functional test"));

	CSqlBackupClient* restoreClient = NULL;
	TRAP(err, restoreClient = CSqlBackupClient::NewL(TheTestHarness));
	TEST(restoreClient != NULL);

	TInt bytesRestored = 0;
	TRAP(err, bytesRestored = TestRestoreL(*restoreClient, TheTestHarness->Fs()));
	TEST2(err, KErrNone);
	
	TEST(bytesRestored == bytesStored);

	delete restoreClient;

	CompareDbContentWithBuf(TheTestHarness->Fs());
	}
	
TInt DoBackupL()
	{
	CSqlBackupClient* backupClient = CSqlBackupClient::NewLC(TheTestHarness);
	TInt bytesStored = TestBackupL(*backupClient, TheTestHarness->Fs());
	CleanupStack::PopAndDestroy(backupClient);
	return bytesStored;
	}

TInt DoRestoreL()
	{
	CSqlBackupClient* restoreClient = CSqlBackupClient::NewLC(TheTestHarness);
	TInt bytesRestored = TestRestoreL(*restoreClient, TheTestHarness->Fs());
	CleanupStack::PopAndDestroy(restoreClient);
	return bytesRestored;
	}

/**
@SYMTestCaseID			SYSLIB-SQL-UT-4003
@SYMTestCaseDesc		Test for DEF113598 - "SQL, t_sqlbur unit test needs refactoring"
						Under simulated OOM condition, the test backups 2 test db files, 
						then restores the test db files content from the backup file.
						At the end, the test checks that the restored test db files content is the
						same as the content of the original test db file.
@SYMTestPriority		High
@SYMTestActions			Test for DEF113598 - "SQL, t_sqlbur unit test needs refactoring"
@SYMTestExpectedResults Test must not fail
@SYMDEF					DEF113598
*/	
void OomTest()
	{
	///////////////////////////////////////////////////////////////////////////////
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-UT-4003 Backup: OOM test "));
	TInt err = KErrNoMemory;
	TInt bytesStored = 0;
	TInt count = 0;
	
	for(count=1;err==KErrNoMemory;++count)
		{
		TInt startProcessHandleCount;
		TInt startThreadHandleCount;
		RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);
		
		User::__DbgMarkStart(RHeap::EUser);
		User::__DbgSetAllocFail(RHeap::EUser,RHeap::EFailNext, count);
		TRAP(err, bytesStored = DoBackupL());
		User::__DbgMarkEnd(RHeap::EUser, 0);
		
		TInt endProcessHandleCount;
		TInt endThreadHandleCount;
		RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);
		
		TEST(startProcessHandleCount == endProcessHandleCount);
		TEST(startThreadHandleCount == endThreadHandleCount);
		}
	TEST2(err, KErrNone);
	TheTest.Printf(_L("OOM backup test succeeded at heap failure rate of %d\r\n"), count);

	///////////////////////////////////////////////////////////////////////////////
	TheTest.Next(_L("Restore: OOM test"));
	err = KErrNoMemory;
	TInt bytesRestored = 0;
	
	for(count=1;err==KErrNoMemory;++count)
		{
		TInt startProcessHandleCount;
		TInt startThreadHandleCount;
		RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);
		
		User::__DbgMarkStart(RHeap::EUser);
		User::__DbgSetAllocFail(RHeap::EUser,RHeap::EFailNext, count);
		TRAP(err, bytesRestored = DoRestoreL());
		User::__DbgMarkEnd(RHeap::EUser, 0);
		
		TInt endProcessHandleCount;
		TInt endThreadHandleCount;
		RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);
		
		TEST(startProcessHandleCount == endProcessHandleCount);
		TEST(startThreadHandleCount == endThreadHandleCount);
		}
	TEST2(err, KErrNone);
	User::__DbgSetAllocFail(RHeap::EUser, RAllocator::ENone, 0);
	TheTest.Printf(_L("OOM restore test succeeded at heap failure rate of %d\r\n"), count);
	
	TEST(bytesStored == bytesRestored);

	CompareDbContentWithBuf(TheTestHarness->Fs());
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4143
@SYMTestCaseDesc		SQL Backup&Restore - data chunk size test.
						The test uses an integer array of 10 elements with randomly generated data chunk sizes.
						Then the test runs 10 backup iterations using each time different data chunk size.
						After each backup iteration the test performs a restore operation and checks that the 
						data has been backup&restored without errors.
@SYMTestActions			SQL Backup&Restore - data chunk size test.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		Medium
@SYMREQ					REQ12104
*/
void FunctionalTest2()
	{
	TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4143 Backup&Restore: functional test 2"));
	
	TTime now;
	now.UniversalTime();
	TInt64 seed = now.Int64();

	const TInt KArraySize = 10;
	TInt  dataChunks[10] = {2, 6, 0, 0, 0, 0, 0, 0, 0, 0};
	const TInt KMaxDataChunkSize = 50;
	
	for(TInt i=2;i<KArraySize;)
		{
		TInt dataChunkSize = Math::Rand(seed) % KMaxDataChunkSize;
		if((dataChunkSize % 2) == 0 && dataChunkSize != 0)	//The code works only with data chunks with even sizes!!!
			{
			dataChunks[i++] = dataChunkSize;
			}
		}
	
	for(TInt i=0;i<KArraySize;++i)
		{
		TheTest.Printf(_L(" === Iteration %d, chunk size %d\r\n"), i + 1, dataChunks[i]);
		CSqlBackupClient* backupClient = NULL;
		TRAPD(err, backupClient = CSqlBackupClient::NewL(TheTestHarness));
		TEST(backupClient != NULL);

		TInt bytesStored = 0;
		TRAP(err, bytesStored = TestBackupL(*backupClient, TheTestHarness->Fs(), dataChunks[i]));
		TEST2(err, KErrNone);

		TRAP(err, TestArchiveIntegrityL(*backupClient, TheTestHarness->Fs()));
		TEST2(err, KErrNone);

		delete backupClient;

		CSqlBackupClient* restoreClient = NULL;
		TRAP(err, restoreClient = CSqlBackupClient::NewL(TheTestHarness));
		TEST(restoreClient != NULL);

		TInt bytesRestored = 0;
		TRAP(err, bytesRestored = TestRestoreL(*restoreClient, TheTestHarness->Fs(), dataChunks[i]));
		TEST2(err, KErrNone);
		
		TEST(bytesRestored == bytesStored);

		delete restoreClient;

		CompareDbContentWithBuf(TheTestHarness->Fs());
		}
	}

/**
@SYMTestCaseID			PDS-SQL-UT-4144
@SYMTestCaseDesc		SQL Backup&Restore - legacy backup file format header test.
						The 64-bit file system related changes made in the SQL server required some 
						changes to be made in the format of the backup file header.
						The test checks that a backup file created with the previous format of the file header
						can be restored without errors by the updated Backup&Restore implementation.
@SYMTestActions			SQL Backup&Restore - legacy backup file format header test.
@SYMTestExpectedResults Test must not fail
@SYMTestPriority		Medium
@SYMREQ					REQ12104
*/
void LegacyFileFormatTest()
	{
	TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4144 Backup&Restore: legacy file format test"));

	//KBackupFile2 is a database backup file with header version 0.
	(void)TheTestHarness->Fs().Delete(KBackupFile2);
	TInt rc = BaflUtils::CopyFile(TheTestHarness->Fs(), KBackupFile2Z, KBackupFile2);
	TEST2(rc, KErrNone);
	(void)TheTestHarness->Fs().SetAtt(KBackupFile2, 0, KEntryAttReadOnly);

	//Restore the databases from KBackupFile2.
	CSqlBackupClient* restoreClient = NULL;
	TRAP(rc, restoreClient = CSqlBackupClient::NewL(TheTestHarness));
	TEST(restoreClient != NULL);

	RFile file;
	rc = file.Open(TheTestHarness->Fs(), KBackupFile2, EFileRead | EFileShareExclusive);
	TEST2(rc, KErrNone);
	
	TRAP(rc, restoreClient->InitialiseRestoreProxyBaseDataL(KClientUid, EDriveC));
	TEST2(rc, KErrNone);
	
	TBuf8<KBufferSize> buf;
	TPtr8 ptr((TUint8*)buf.Ptr(), buf.MaxSize());
	TBool finishedFlag = EFalse;
	
	TInt fileSize = 0;
	rc = file.Size(fileSize);
	TEST2(rc, KErrNone);
	
	do
		{
		rc = file.Read(ptr);
		TEST2(rc, KErrNone);
		fileSize -= ptr.Size();
		finishedFlag = fileSize == 0;
		TRAP(rc, restoreClient->RestoreBaseDataSectionL(ptr, finishedFlag));
		ptr.SetLength(0);
		} 
	while(fileSize > 0);
	
	file.Close();	
	
	restoreClient->RestoreComplete(EDriveC);
	
	TEST(finishedFlag);
		
	delete restoreClient;

	//At this point we have two restored databases: KTestDbFileName1 and KTestDbFileName2.
	//The content of the restored file cannot be compared directly, because t_sqlattach uses the same test databases
	//and modifies them. The original database content was stored without executing t_sqlattach.
	//Hence a simple test is made: open the restored database, check if the database content can be accessed.
	
	RSqlDatabase db;
	rc = db.Open(KTestDbFileName1);
	TEST2(rc, KErrNone);
	//The database contains this table: "TABLE C(A1 INTEGER, B2 BLOB)". 
	rc = db.Exec(_L("INSERT INTO C VALUES(100, 200)"));
	TEST2(rc, 1);
	RSqlStatement stmt;
	rc = stmt.Prepare(db, _L("SELECT * FROM C"));
	TEST2(rc, KErrNone);
	while((rc = stmt.Next()) == KSqlAtRow)
		{
		}
	stmt.Close();
	TEST2(rc, KSqlAtEnd);
	db.Close();

	rc = db.Open(KTestDbFileName2);
	TEST2(rc, KErrNone);
	//The database contains this table: "TABLE A1(F1 INTEGER , F2 INTEGER, B1 BLOB)"
	rc = db.Exec(_L("INSERT INTO A1 VALUES(100, 200, NULL)"));
	TEST2(rc, 1);
	rc = stmt.Prepare(db, _L("SELECT * FROM A1"));
	TEST2(rc, KErrNone);
	while((rc = stmt.Next()) == KSqlAtRow)
		{
		}
	stmt.Close();
	TEST2(rc, KSqlAtEnd);
	db.Close();

	(void)TheTestHarness->Fs().Delete(KBackupFile2);
	}
			
void DoMain()
	{
	TestEnvCreate();

	TheTest.Start(_L("Store db content to memory buffer"));
	StoreDbContentToBuf(TheTestHarness->Fs());
		
	FunctionalTest();

	OomTest();

	FunctionalTest2();

	LegacyFileFormatTest();

	TestEnvDestroy();
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	TEST(tc != NULL);
	
	__UHEAP_MARK;
	
	DoMain();
	
	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;
	
	User::Heap().Check();
	return KErrNone;
	}
