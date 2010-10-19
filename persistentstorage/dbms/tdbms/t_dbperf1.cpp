// Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <e32math.h>
#include <bautils.h>
#include <hal.h>
#include <d32dbms.h>
#include "t_dbcmdlineutil.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("t_dbperf1"));
RFs   TheFs;
RDbs  TheDbs;

TFileName TheNonSecureDbName;
TFileName TheNonSecureDbName2;
_LIT(KFillDbScript, "z:\\test\\t_dbperf1.sql");
_LIT(KUpdateSql, "UPDATE IDENTITYTABLE SET CM_FIRSTNAME='%S%d',CM_LASTNAME='%S%d',CM_COMPANYNAME='%S%d' WHERE PARENT_CMID=%d");
_LIT(KSelectSql, "SELECT CM_FIRSTNAME, CM_LASTNAME, CM_COMPANYNAME FROM IDENTITYTABLE WHERE PARENT_CMID > 50");
_LIT(KDeleteSql, "DELETE FROM IDENTITYTABLE WHERE PARENT_CMID > 50");
_LIT(KFirstName, "FirstName-");
_LIT(KLastName, "LastName-");
_LIT(KCompanyName, "CompanyName-");

const TInt KTestTecordCount = 1000;

enum TTestType 
	{
	EClientDbTest,		
	EServerDbTest
	};
	
enum TCompactDbOption
	{
	ENoCompaction,
	ECompactAfterCommit,
	ECompactAfterEvery10Rec,
	ECompactAfterEveryUpd,
	ECompactAtEnd
	};
	
TCmdLineParams TheCmdLineParams;
RFile TheLogFile; 
TBuf<250> TheLogLine;
TBuf8<250> TheLogLine8;
TBuf<200> TheTestTitle;

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvDestroy()
	{
	TheDbs.Close();
	TheFs.Delete(TheNonSecureDbName2);
	TheFs.Delete(TheNonSecureDbName);
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		(void)TheLogFile.Flush();
		TheLogFile.Close();
		}
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
		TheTest.Printf(_L("*** Line %d. Expression evaluated to false\r\n"), aLine);
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

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvInit()
    {
	TInt err = TheFs.Connect();
	TEST2(err, KErrNone);

	err = TheFs.MkDir(TheNonSecureDbName);
	TEST(err == KErrNone || err == KErrAlreadyExists);

	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		err = TheLogFile.Replace(TheFs, TheCmdLineParams.iLogFileName, EFileRead | EFileWrite);
		TEST2(err, KErrNone);
		LogConfig(TheLogFile, TheCmdLineParams);
		}
	
	err = TheDbs.Connect();
	TEST2(err, KErrNone);
	}

//Reads a SQL file and returns the file content as HBUFC string.
//The caller is responsible for destroying the returned HBufC object.
HBufC* ReadSqlScript(const TDesC& aSqlFileName)
	{
	RFile file;
	TEST2(file.Open(TheFs, aSqlFileName, EFileRead), KErrNone);
	
	TInt size = 0;
	TEST2(file.Size(size), KErrNone);
	
	HBufC8* sql = HBufC8::New(size);
	TEST(sql != NULL);
	
	TPtr8 ptr = sql->Des();
	TEST2(file.Read(ptr, size), KErrNone);

	file.Close();
	
	HBufC* sql2 = HBufC::New(size);
	TEST(sql2 != NULL);
	sql2->Des().Copy(sql->Des());
	delete sql;
	
	return sql2;
	}
	
//Searches for the next ';' appearance in aSqlScript string and returns a TPtrC object holding
//the SQL statement, terminated with that ';'.
TPtrC GetNextStmt(TPtrC& aSqlScript)
	{
	_LIT(KTerminator, ";\xD");
	TPtrC res(NULL, 0);
	TInt pos = aSqlScript.FindF(KTerminator());
	if(pos >= 0)
		{
		res.Set(aSqlScript.Left(pos));
		pos += 1;
		aSqlScript.Set(aSqlScript.Mid(pos));
		}
	return res;
	}

//Prints aTime parameter (converted to ms)
void PrintStats(TUint32 aStartTicks, TUint32 aEndTicks)
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		TEST2(HAL::Get(HAL::EFastCounterFrequency, freq), KErrNone);
		}
	TInt64 diffTicks = (TInt64)aEndTicks - (TInt64)aStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	TheTest.Printf(_L("%S: %d us\r\n"), &TheTestTitle, us);
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		TheLogLine.Format(_L("%S¬%d¬us\r\n"), &TheTestTitle, us);
		TheLogLine8.Copy(TheLogLine);
		(void)TheLogFile.Write(TheLogLine8);
		}
	}
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////     DBMS SERVER performance tests
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Creates aDb database schema.
void CreateDbSchemaL(RDbDatabase& aDb)
	{
	//"Contacts" table
	CDbColSet* colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_id"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_type"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_preftemplaterefid"), EDbColInt32));
	colSet->AddL(TDbCol(_L("cm_uidstring"), EDbColText, 244));
	colSet->AddL(TDbCol(_L("cm_last_modified"), EDbColDateTime));
	colSet->AddL(TDbCol(_L("cm_contactcreationdate"), EDbColDateTime));
	colSet->AddL(TDbCol(_L("cm_attributes"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_replicationcount"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_header"), EDbColLongText));
	colSet->AddL(TDbCol(_L("cm_textblob"), EDbColLongText));
	colSet->AddL(TDbCol(_L("cm_searchabletext"), EDbColLongText));
	TInt err = aDb.CreateTable(_L("contacts"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"identitytable" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("parent_cmid"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_firstname"), EDbColText, 255));
	colSet->AddL(TDbCol(_L("cm_lastname"), EDbColText, 255));
	colSet->AddL(TDbCol(_L("cm_companyname"), EDbColText, 255));
	colSet->AddL(TDbCol(_L("cm_type"), EDbColInt32));
	colSet->AddL(TDbCol(_L("cm_attributes"), EDbColInt32));
	colSet->AddL(TDbCol(_L("cm_hintfield"), EDbColUint8));
	colSet->AddL(TDbCol(_L("cm_exthintfield"), EDbColUint16));
	colSet->AddL(TDbCol(_L("cm_firstnmprn"), EDbColText, 255));
	colSet->AddL(TDbCol(_L("cm_lastnmprn"), EDbColText, 255));
	colSet->AddL(TDbCol(_L("cm_companynmprn"), EDbColText, 255));
	err = aDb.CreateTable(_L("identitytable"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"emailtable" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("emailparent_cmid"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("email_fieldid"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("emailaddress"), EDbColText, 255));
	err = aDb.CreateTable(_L("emailtable"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"phone" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_id"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_phonematching"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_extendedphonematching"), EDbColInt32, TDbCol::ENotNull));
	err = aDb.CreateTable(_L("phone"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"groups" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_id"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_members"), EDbColInt32, TDbCol::ENotNull));
	err = aDb.CreateTable(_L("groups"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"groups2" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_id"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_groupmembers"), EDbColLongText));
	err = aDb.CreateTable(_L("groups2"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"sync" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_id"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_lastsyncdate"), EDbColDateTime));
	err = aDb.CreateTable(_L("sync"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//"preferences" table
	colSet = CDbColSet::NewLC();
	colSet->AddL(TDbCol(_L("cm_preffileid"), EDbColInt16));
	colSet->AddL(TDbCol(_L("cm_preftemplateid"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_prefowncardid"), EDbColInt32));
	colSet->AddL(TDbCol(_L("cm_prefcardtemplateprefid"), EDbColInt32));
	colSet->AddL(TDbCol(_L("cm_preffilever"), EDbColInt32, TDbCol::ENotNull));
	colSet->AddL(TDbCol(_L("cm_prefcardtemplateid"), EDbColLongText));
	colSet->AddL(TDbCol(_L("cm_prefgroupidlist"), EDbColLongText));
	colSet->AddL(TDbCol(_L("cm_creationdate"), EDbColDateTime));
	colSet->AddL(TDbCol(_L("cm_machineuid"), EDbColInt64));
	colSet->AddL(TDbCol(_L("cm_prefsortorder"), EDbColLongText));
	err = aDb.CreateTable(_L("preferences"), *colSet);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); colSet = NULL;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//"Contacts" table keys
	CDbKey* key = CDbKey::NewLC();
	key->AddL(_L("cm_id"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("contacts"), _L("contacts"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_preftemplaterefid"));
	err = aDb.CreateIndex(_L("cm_preftemplaterefid"), _L("contacts"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"identitytable" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("parent_cmid"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("identitytable"), _L("identitytable"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"emailtable" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("email_fieldid"));
	key->AddL(_L("emailparent_cmid"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("emailtable"), _L("emailtable"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("emailparent_cmid"));
	err = aDb.CreateIndex(_L("emailparent_cmid"), _L("emailtable"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"phone" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("cm_phonematching"));
	key->AddL(_L("cm_id"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("phone"), _L("phone"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_id"));
	err = aDb.CreateIndex(_L("cm_id"), _L("phone"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"groups" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("cm_members"));
	key->AddL(_L("cm_id"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("groups"), _L("groups"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_id"));
	err = aDb.CreateIndex(_L("cm_id"), _L("groups"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_members"));
	err = aDb.CreateIndex(_L("cm_members"), _L("groups"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"groups2" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("cm_id"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("groups2"), _L("groups2"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"sync" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("cm_id"));
	key->MakeUnique();
	err = aDb.CreateIndex(_L("sync"), _L("sync"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	//"preferences" table keys
	key = CDbKey::NewLC();
	key->AddL(_L("cm_preftemplateid"));
	err = aDb.CreateIndex(_L("cm_preftemplateid"), _L("preferences"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_prefowncardid"));
	err = aDb.CreateIndex(_L("cm_prefowncardid"), _L("preferences"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	key = CDbKey::NewLC();
	key->AddL(_L("cm_prefcardtemplateprefid"));
	err = aDb.CreateIndex(_L("cm_prefcardtemplateprefid"), _L("preferences"), *key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(); key = NULL;
	}

class TDbHelper
	{
public:	
	static void CreateL(const TDesC& aDbName)
		{
		RDbNamedDatabase db;
		TInt err = db.Replace(TheFs, aDbName);
		TEST2(err, KErrNone);
		CreateDbSchemaL(db);
		db.Close();
		}
	static RDbDatabase Open(TTestType aTestType, const TDesC& aDbName)
		{
		RDbNamedDatabase db;
		TInt err = aTestType == EClientDbTest ? db.Open(TheFs, aDbName) : db.Open(TheDbs, aDbName);
		TEST2(err, KErrNone);
		return db;
		}
	};
	
//Executes SQL script
void ExecuteSqlScript(RDbDatabase& aDb, const TDesC& aScriptFileName, TCompactDbOption aCompactDbOpt)
	{
	HBufC* fullDbScript = ReadSqlScript(aScriptFileName);
	TUint32 start = User::FastCounter();
	TPtrC ptr(fullDbScript->Des());
	TInt stmtCnt = 0;
	TInt err = aDb.Begin();
	TEST2(err, KErrNone);
	TPtrC sql(GetNextStmt(ptr));
	while(sql.Length() > 0)
		{
		++stmtCnt;
		TInt rc = aDb.Execute(sql);
		if(rc == KErrNoMemory)
			{
			TheTest.Printf(_L("###ERROR 'Out of memory'! The test cannot be completed!\r\n"));
			return;	
			}
		TEST2(rc, 1);
		if((stmtCnt % 200) == 0) //~16 transactions (~3270 statements total)
			{
			err = aDb.Commit();	
			TEST2(err, KErrNone);
			if(aCompactDbOpt == ECompactAfterCommit)
				{
				err = aDb.Compact();
				TEST2(err, KErrNone);
				}
			err = aDb.Begin();
			TEST2(err, KErrNone);
			}
		sql.Set(GetNextStmt(ptr));
		}
	err = aDb.Commit();	
	TEST2(err, KErrNone);
	if(aCompactDbOpt == ECompactAfterCommit || aCompactDbOpt == ECompactAtEnd)
		{
		err = aDb.Compact();
		TEST2(err, KErrNone);
		}
	TUint32 end = User::FastCounter();
	PrintStats(start, end);
	delete fullDbScript;
	}

///////////////////////////////////////////////////////////////////////////////////////

//"INSERT" test function
void InsertTest(TTestType aTestType, const TDesC& aDbFileName, TCompactDbOption aCompactDbOpt)
	{
	RDbDatabase db = TDbHelper::Open(aTestType, aDbFileName);
	ExecuteSqlScript(db, KFillDbScript, aCompactDbOpt);
	RDbTable tbl;
	TInt err = tbl.Open(db, _L("CONTACTS"));
	TEST2(err, KErrNone);
	TInt recCnt = 0;
	TRAP(err, recCnt = tbl.CountL());
	TEST2(err, KErrNone);
	TEST2(recCnt, 1001);
	tbl.Close();
	db.Close();	
	}

//"UPDATE" test function
void UpdateTest(TTestType aTestType, const TDesC& aDbFileName, const TDesC& aUpdateSql, TCompactDbOption aCompactDbOpt)
	{
	RDbDatabase db = TDbHelper::Open(aTestType, aDbFileName);

	TUint32 start = User::FastCounter();
	for(TInt id=1,updateOpCnt=0;id<=KTestTecordCount;++id,++updateOpCnt)
		{
		TBuf<200> sql;
		sql.Format(aUpdateSql, &KFirstName, id, &KLastName, id, &KCompanyName, id, id);
		TInt rc = db.Execute(sql);
		TEST2(rc, 1);
		if(aCompactDbOpt == ECompactAfterEveryUpd || (aCompactDbOpt == ECompactAfterEvery10Rec && (updateOpCnt % 10) == 0))
			{
			TInt err = db.Compact();
			TEST2(err, KErrNone);
			}
		}
	if(aCompactDbOpt == ECompactAfterEvery10Rec || aCompactDbOpt == ECompactAtEnd)
		{
		TInt err = db.Compact();
		TEST2(err, KErrNone);
		}
	TUint32 end = User::FastCounter();
	PrintStats(start, end);
	db.Close();
	}

//"SELECT" test function
void SelectTestL(TTestType aTestType, const TDesC& aDbFileName, const TDesC& aSelectSql)
	{
	RDbDatabase db = TDbHelper::Open(aTestType, aDbFileName);
	TDbQuery query(aSelectSql);
	RDbView view;
	TInt err = view.Prepare(db, query, RDbRowSet::EReadOnly);
	TEST2(err, KErrNone);

	TUint32 start = User::FastCounter();
	err = view.EvaluateAll();
	TEST2(err, KErrNone);
	while(view.NextL())
		{
		view.GetL();
		TPtrC colVal;
		TBuf<20> res;
		colVal.Set(view.ColDes(1));
		res.Copy(colVal);
		TEST(colVal.Length() > 0);
		colVal.Set(view.ColDes(2));
		TEST(colVal.Length() > 0);
		res.Copy(colVal);
		colVal.Set(view.ColDes(3));
		TEST(colVal.Length() > 0);
		res.Copy(colVal);
		}
	TEST(view.AtEnd());
	view.Close();
	TUint32 end = User::FastCounter();
	PrintStats(start, end);
	db.Close();	
	}

//"DELETE" test function
void DeleteTest(TTestType aTestType, const TDesC& aDbFileName, const TDesC& aDeleteSql, TCompactDbOption aCompactDbOpt)
	{
	RDbDatabase db = TDbHelper::Open(aTestType, aDbFileName);
	TUint32 start = User::FastCounter();
	TInt rc = db.Execute(aDeleteSql);
	TEST(rc > 900);
	if(aCompactDbOpt == ECompactAtEnd)
		{
		TInt err = db.Compact();
		TEST2(err, KErrNone);
		}
	TUint32 end = User::FastCounter();
	PrintStats(start, end);
	db.Close();	
	}

/**
@SYMTestCaseID			PDS-DBMS-CT-4008
@SYMTestCaseDesc		DBMS server performance tests.
						Three test types used: "insert records", "update records" and "select records".
						The tests are executed on non-secure databases.
						Each test executed using UTF16 encoded SQl statements.
						The test results are used for a comparison against the SQL server test results 
						(T_SqlPerformance test).
@SYMTestPriority		High
@SYMTestActions			DBMS server performance tests.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ5792
                        REQ5793
*/	
void DoTestsL()
	{
	TheTest.Start(_L(" @SYMTestCaseID:PDS-DBMS-CT-4008 DBMS performance tests"));
	
	CFileMan* fm = CFileMan::NewL(TheFs);
	CleanupStack::PushL(fm);

	//==================================== CLIENT ==========================================================
	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, client, no db compaction"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EClientDbTest, TheNonSecureDbName, ENoCompaction);
	
	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, client, db compaction at end"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EClientDbTest, TheNonSecureDbName, ECompactAtEnd);
	
	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, client, db compaction after commit"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EClientDbTest, TheNonSecureDbName, ECompactAfterCommit);

	TInt err = fm->Copy(TheNonSecureDbName, TheNonSecureDbName2);
	TEST2(err, KErrNone);
	
	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, client, no db compaction"));
	UpdateTest(EClientDbTest, TheNonSecureDbName, KUpdateSql(), ENoCompaction);
	err = fm->Copy(TheNonSecureDbName2, TheNonSecureDbName);
	TEST2(err, KErrNone);
	
	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, client, db compaction at end"));
	UpdateTest(EClientDbTest, TheNonSecureDbName, KUpdateSql(), ECompactAtEnd);
	err = fm->Copy(TheNonSecureDbName2, TheNonSecureDbName);
	TEST2(err, KErrNone);

	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, client, db compaction after every 10 records"));
	UpdateTest(EClientDbTest, TheNonSecureDbName, KUpdateSql(), ECompactAfterEvery10Rec);
	
	TheTestTitle.Copy(_L("DBMS, select, UTF16 SQL strings, non-secure db, client"));
	SelectTestL(EClientDbTest, TheNonSecureDbName, KSelectSql());

	TheTestTitle.Copy(_L("DBMS, delete, UTF16 SQL strings, non-secure db, client"));
	DeleteTest(EClientDbTest, TheNonSecureDbName, KDeleteSql(), ECompactAtEnd);

	//==================================== SERVER ==========================================================

	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, server, no db compaction"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EServerDbTest, TheNonSecureDbName, ENoCompaction);
	
	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, server, db compaction at end"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EServerDbTest, TheNonSecureDbName, ECompactAtEnd);

	TheTestTitle.Copy(_L("DBMS, insert, UTF16 SQL strings, non-secure db, server, db compaction after commit"));
	TDbHelper::CreateL(TheNonSecureDbName);
	InsertTest(EServerDbTest, TheNonSecureDbName, ECompactAfterCommit);

	err = fm->Copy(TheNonSecureDbName, TheNonSecureDbName2);
	TEST2(err, KErrNone);

	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, server, no db compaction"));
	UpdateTest(EServerDbTest, TheNonSecureDbName, KUpdateSql(), ENoCompaction);
	err = fm->Copy(TheNonSecureDbName2, TheNonSecureDbName);
	TEST2(err, KErrNone);
	
	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, server, db compaction at end"));
	UpdateTest(EServerDbTest, TheNonSecureDbName, KUpdateSql(), ECompactAtEnd);
	err = fm->Copy(TheNonSecureDbName2, TheNonSecureDbName);
	TEST2(err, KErrNone);

	TheTestTitle.Copy(_L("DBMS, update, UTF16 SQL strings, non-secure db, server, db compaction after every 10 records"));
	UpdateTest(EServerDbTest, TheNonSecureDbName, KUpdateSql(), ECompactAfterEvery10Rec);
	
	TheTestTitle.Copy(_L("DBMS, select, UTF16 SQL strings, non-secure db, server"));
	SelectTestL(EServerDbTest, TheNonSecureDbName, KSelectSql());

	TheTestTitle.Copy(_L("DBMS, delete, UTF16 SQL strings, non-secure db, server"));
	DeleteTest(EServerDbTest, TheNonSecureDbName, KDeleteSql(), ECompactAtEnd);
	
	//======================================================================================================
	CleanupStack::PopAndDestroy(fm);
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();

	GetCmdLineParams(TheTest, _L("t_dbperf1"), TheCmdLineParams);
	_LIT(KNonSecureDbName, "c:\\test\\t_dbperf1_1.db");
	PrepareDbName(KNonSecureDbName, TheCmdLineParams.iDriveName, TheNonSecureDbName);
	_LIT(KNonSecureDbName2, "c:\\test\\t_dbperf1_2.db");
	PrepareDbName(KNonSecureDbName2, TheCmdLineParams.iDriveName, TheNonSecureDbName2);
	TheTest.Printf(_L("==Databases: %S, %S\r\n"), &TheNonSecureDbName, &TheNonSecureDbName2); 
	
	__UHEAP_MARK;
	
	TestEnvInit();
	TRAPD(err, DoTestsL());
	TestEnvDestroy();
	TEST2(err, KErrNone);
	
	__UHEAP_MARKEND;

	User::Heap().Check();
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	return KErrNone;
	}
