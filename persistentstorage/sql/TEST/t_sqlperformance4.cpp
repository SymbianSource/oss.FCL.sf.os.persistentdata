// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include <hal.h>

///////////////////////////////////////////////////////////////////////////////////////

RTest			TheTest(_L("t_sqlperformance4 test"));
RSqlDatabase 	TheDbC;
RFs				TheFs;

_LIT(KCDriveDatabase, "c:[a000017f]t_sqlperformance4.db");

TFileName		TheDbFileName;
TBuf<256>  		TheCmd;
TDriveName 		TheDriveName;
TParse     		TheParse;

_LIT8( KMCSqlConfig, "cache_size=1024; page_size=1024; " );

_LIT(KMusicCreateTable, "CREATE TABLE Music("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"DbFlag INTEGER,"
    L"VolumeId INTEGER,"
    L"Title TEXT COLLATE NOCASE,"
    L"Artist INTEGER,"
    L"Art TEXT,"
    L"Deleted INTEGER DEFAULT 0,"
    L"Location TEXT,"
    L"AlbumTrack INTEGER,"
    L"PlayCount INTEGER DEFAULT 0,"
    L"TimeAdded TEXT,"
    L"TimePlayed TEXT DEFAULT '',"
    L"Duration INTEGER,"
    L"Sync INTEGER DEFAULT 0,"
    L"Modified INTEGER DEFAULT 0,"
    L"Album INTEGER,"
    L"Genre INTEGER,"
    L"Composer INTEGER,"
    L"ReleaseDate TEXT DEFAULT '',"
    L"Rating INTEGER,"
    L"Comment TEXT,"
    L"Copyright TEXT,"
    L"Url TEXT,"
    L"DRM INTEGER,"
    L"LastPlayPosition INTEGER DEFAULT 0,"
    L"SampleRate INTEGER,"
    L"BitRate INTEGER,"
    L"NumChannels INTEGER,"
    L"Codec INTEGER,"
    L"MimeType TEXT,"
    L"MTPDrmStatus INTEGER)");

_LIT(KAuxiliaryCreateTable, "CREATE TABLE Auxiliary("
    L"Id INTEGER,"
    L"Version TEXT,"
    L"TimeRefreshed TEXT,"
    L"TimeSynced TEXT,"
    L"Corrupt INTEGER DEFAULT 0,"
    L"SaveDeletedRecordCount INTEGER DEFAULT 0)");

_LIT(KAlbumCreateTable,"CREATE TABLE Album("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"Name TEXT COLLATE NOCASE,"
    L"SongCount INTEGER,"
    L"Artist INTEGER,"
    L"Art TEXT)");

_LIT(KArtistCreateTable,"CREATE TABLE Artist("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"Name TEXT COLLATE NOCASE,"
    L"SongCount INTEGER)");

_LIT(KComposerCreateTable,"CREATE TABLE Composer("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"Name TEXT COLLATE NOCASE,"
    L"SongCount INTEGER)");

_LIT(KGenreCreateTable,"CREATE TABLE Genre("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"Name TEXT COLLATE NOCASE,"
    L"SongCount INTEGER)");

_LIT(KPlaylistCreateTable, "CREATE TABLE Playlist("
    L"UniqueId INTEGER PRIMARY KEY,"
    L"VolumeId INTEGER,"
    L"DbFlag INTEGER,"
    L"Sync INTEGER,"
    L"Name TEXT COLLATE NOCASE,"
    L"Uri TEXT,"
    L"Time TEXT)");

_LIT(KPlaylistSongsCreateTable, "CREATE TABLE PlaylistSongs("
    L"UniqueId INTEGER PRIMARY KEY AUTOINCREMENT,"
    L"SongId INTEGER,"
    L"PlaylistId INTEGER,"
    L"Ordinal INTEGER)");

_LIT(KPlaylistSongInfoCreateTable, "CREATE TABLE PlaylistSongInfo("
    L"SongId INTEGER PRIMARY KEY,"
    L"VolumeId INTEGER,"
    L"DbFlag INTEGER,"
    L"Uri TEXT,"
    L"Title TEXT COLLATE NOCASE)");


_LIT(KBeginTransaction, "BEGIN TRANSACTION");
_LIT(KCommitTransaction, "COMMIT TRANSACTION");

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvDestroy()
	{
	TheDbC.Close();
	(void)RSqlDatabase::Delete(TheDbFileName);
	TheFs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		TestEnvDestroy();
		TheTest.Printf(_L("*** Line %d\r\n"), aLine);
		TheTest(EFalse, aLine);
		}
	}
void Check2(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		TestEnvDestroy();
		TheTest.Printf(_L("*** Line %d, Expected error: %d, got: %d\r\n"), aLine, aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check1((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)


TInt GetDuration(TUint32 aStartTicks, TUint32 aEndTicks)
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		HAL::Get(HAL::EFastCounterFrequency, freq);
		}
	TInt64 diffTicks = (TInt64)aEndTicks - (TInt64)aStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	
	return ((diffTicks * KMicroSecIn1Sec) / freq);
	}

///////////////////////////////////////////////////////////////////////////////////////

void CreateDatabaseL(const TDesC& aDbName)
	{			
	// create the database now
	RSqlSecurityPolicy securityPolicy;
	CleanupClosePushL(securityPolicy);
	
	TSecurityPolicy policy(TSecurityPolicy::EAlwaysPass);
	securityPolicy.Create(policy);
	
	TSecurityPolicy schemaPolicy(TSecurityPolicy::EAlwaysPass);
	TSecurityPolicy readPolicy(TSecurityPolicy::EAlwaysPass);
	TSecurityPolicy writePolicy(TSecurityPolicy::EAlwaysPass);
	
	User::LeaveIfError(securityPolicy.SetDbPolicy(RSqlSecurityPolicy::ESchemaPolicy, schemaPolicy));
	User::LeaveIfError(securityPolicy.SetDbPolicy(RSqlSecurityPolicy::EReadPolicy, readPolicy));
	User::LeaveIfError(securityPolicy.SetDbPolicy(RSqlSecurityPolicy::EWritePolicy, writePolicy));
 
	TheTest.Printf(_L("Creating Database %S\n"),  &aDbName);
		
	TInt err = TheDbC.Create(aDbName, securityPolicy, &KMCSqlConfig);
	
	TBuf<64> tmp;
	tmp.Copy(KMCSqlConfig);
	tmp.Append(_L("\n"));
	
	TheTest.Printf(tmp);
	
	if (KErrAlreadyExists == err)
		{
		
		// the file already exists
		// make sure we delete the file
        User::LeaveIfError(TheDbC.Delete(aDbName));

        // try again
        err = TheDbC.Create(aDbName, securityPolicy, &KMCSqlConfig);

		}
	
	User::LeaveIfError(err);
	
	//Create tables	
	User::LeaveIfError(TheDbC.Exec(KMusicCreateTable));
	User::LeaveIfError(TheDbC.Exec(KAuxiliaryCreateTable));
	User::LeaveIfError(TheDbC.Exec(KAlbumCreateTable));
	User::LeaveIfError(TheDbC.Exec(KArtistCreateTable));
	User::LeaveIfError(TheDbC.Exec(KComposerCreateTable));
	User::LeaveIfError(TheDbC.Exec(KGenreCreateTable));
	User::LeaveIfError(TheDbC.Exec(KPlaylistCreateTable));
	User::LeaveIfError(TheDbC.Exec(KPlaylistSongInfoCreateTable));
	User::LeaveIfError(TheDbC.Exec(KPlaylistSongsCreateTable));
	
	TheDbC.Close();
	
	CleanupStack::PopAndDestroy(&securityPolicy);
	}

void TestEnvInit()
    {
    
	TInt err = TheFs.Connect();
	TEST2(err, KErrNone);	
	
	//Create database files
	TRAP(err,CreateDatabaseL(TheDbFileName));
	TEST2(err, KErrNone);
	
	}
	
		
///////////////////////////////////////////////////////////////////////////////////////
/**
@SYMTestCaseID			PDS-SQL-UT-4151
@SYMTestCaseDesc		Measures the performance of inserting multiple records
						into the Music Player MPX database.  This test is based on 
						a real Music Player Harvesting use case
@SYMTestPriority		Medium
@SYMTestActions			Reads SQL transactions from a file and executes them.  
						Records the time for executing each statement
@SYMTestExpectedResults All statements should be executed without error and 
						performance measurements logged
@SYMDEF					DEF142306
*/
void RunTest()
	{
	//Open the file with the sql statements 
	_LIT(KSqlFileName,"z:\\test\\t_sqlperformance4.sql");
	RFile sqlFile;
	TInt err = sqlFile.Open(TheFs, KSqlFileName, EFileRead); 
	TEST2(err, KErrNone);
	
	TInt fileLen = 0;
	err = sqlFile.Size(fileLen); 
	TEST2(err, KErrNone);
	
	HBufC8* sqlBuf = HBufC8::New(fileLen); 
	TEST(sqlBuf != NULL);
	TPtr8 sql = sqlBuf->Des();
	err = sqlFile.Read(sql);
	
	sqlFile.Close();
	TEST2(err, KErrNone);
	TEST2(sql.Length(), fileLen);
	
	//Open main database
	err = TheDbC.Open(TheDbFileName);
	TEST2(err, KErrNone);
	
	TheTest.Printf(_L("Beginning INSERTS...\n"));
	
	const TInt KRecordCount = 6544;
	TInt recordCount = 0;
	TInt insertCnt = 0;
	TInt updateCnt = 0;
	TInt selectCnt = 0;
	TInt trnCnt = 0;
	TInt totalTime = 0;

	TInt insertTrnCnt = 0;
	TInt updateTrnCnt = 0;
	TInt selectTrnCnt = 0;
	
	for(;sql.Length()>0;)
		{
		TInt eolPos = sql.Locate(TChar('\n'));
		if(eolPos < 0)
			{
			break;//No more SQL statements
			}
		TPtrC8 sqlStmt8(sql.Ptr(), eolPos - 1);//"eolPos - 1" - to cut off the '\r' character
		TPtrC8 ptr = sql.Mid(eolPos + 1);//"eolPos + 1" - first character after '\n'
		sql.Set(const_cast <TUint8*> (ptr.Ptr()), ptr.Length(), ptr.Length());
		++recordCount;
		
		//Convert to 16 bit query string
		TBuf<1024> query;
		query.Copy(sqlStmt8);
		
		//Execute the statement
		TInt start = User::FastCounter();
		err = TheDbC.Exec(query);
		TInt end = User::FastCounter();
		
		TEST(err >= 0);
		
		//Get the execution time for that statement
		TInt duration = GetDuration(start, end);
		totalTime += duration;
		
		if(query == KBeginTransaction)
			{		
			TheTest.Printf(_L("Execute Statement - BEGIN: %d us\n"), duration);
			}
		
		else if(query == KCommitTransaction)
			{
			++trnCnt;
			TheTest.Printf(_L("Execute Statement - COMMIT: %d us, Trn#%d, \"INSERT\" count: %d, \"UPDATE\" count: %d, \"SELECT\" count: %d\n"), 
					duration, trnCnt, insertTrnCnt, updateTrnCnt, selectTrnCnt);
			insertTrnCnt = updateTrnCnt = selectTrnCnt = 0;
			}

		else
			{	
			TPtrC queryType(query.Ptr(), 6);
			TheTest.Printf(_L("Execute Statement - %S: %d us\n"),&queryType, duration);
			if(queryType.FindF(_L("INSERT")) >= 0)
				{
				++insertCnt;
				++insertTrnCnt;
				}
			else if(queryType.FindF(_L("UPDATE")) >= 0)
				{
				++updateCnt;
				++updateTrnCnt;
				}
			else if(queryType.FindF(_L("SELECT")) >= 0)
				{
				++selectCnt;
				++selectTrnCnt;
				}
			}
		}
	delete sqlBuf;
	
	TheDbC.Close();
	
	TheTest.Printf(_L("Total time to process Songs: %d us\n"), totalTime);
	TheTest.Printf(_L("Transactions count: %d, \"INSERT\" count: %d, \"UPDATE\" count: %d, \"SELECT\" count: %d\n"), 
			               trnCnt, insertCnt, updateCnt, selectCnt);
	TEST2(recordCount, KRecordCount);
	}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void DoTests()
	{
	TheTest.Start(_L("@SYMTestCaseID:PDS-SQL-UT-4151; SQL Music Player Db Performance Test"));
	
	RunTest();
	}

//Usage: "t_sqlperformance4 [<drive letter>:]"

TInt E32Main()
	{
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);

	__UHEAP_MARK;

	User::CommandLine(TheCmd);
	TheCmd.TrimAll();
	if(TheCmd.Length() > 0)
		{
		TheDriveName.Copy(TheCmd);
		}
	TheParse.Set(TheDriveName, &KCDriveDatabase, 0);
	const TDesC& dbFilePath = TheParse.FullName();
	TheDbFileName.Copy(dbFilePath);
	TheTest.Printf(_L("==Database file name: %S\r\n"), &TheDbFileName);
	
	TestEnvInit();
	
	DoTests();
	
	TestEnvDestroy();
	
	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
