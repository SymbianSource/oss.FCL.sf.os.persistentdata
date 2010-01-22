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
#include <e32math.h>
#include <hal.h>
#include <sqldb.h>
#include "t_sqliteperf.h"

///////////////////////////////////////////////////////////////////////////////////////

static RTest TheTest(_L("t_sqliteperf test"));
_LIT(KTestDir, "c:\\test\\");
_LIT(KTestDbName,   "c:\\test\\t_sqliteperf.db");
_LIT8(KTestDbName8, "c:\\test\\t_sqliteperf.db\x0");

_LIT(KSqlSrvName, "sqlsrv.exe");

static RSqlDatabase TheDb;

const TInt KInsertRecCnt = 1000;
const TInt KUpdateRecCnt = KInsertRecCnt / 10;
const TInt KDeleteRecCnt = KInsertRecCnt / 10;
const TInt KSelectRecCnt = KInsertRecCnt / 10;

static TInt TheUpdateRecIds[KUpdateRecCnt] = {0};
static TInt TheDeleteRecIds[KDeleteRecCnt] = {0};
static TInt TheSelectRecIds[KSelectRecCnt] = {0};

static TInt TheInsertRecId = 0;
static TInt TheUpdateRecId = 0;
static TInt TheDeleteRecId = 0;
static TInt TheSelectRecId = 0;

const char KMultiInsertSql[] = "INSERT INTO Tbl VALUES(:Id, 9234567890, 99.998, 'TEXT VALUE', x'AABBCCDDEEFF')";
const char KMultiUpdateSql[] = "UPDATE Tbl SET D=11.113 WHERE I IN";
const char KMultiDeleteSql[] = "DELETE FROM Tbl WHERE I IN";
const char KMultiSelectSql[] = "SELECT I64,D,T,B FROM Tbl WHERE I IN";

const char KSingleInsertSql[] = "INSERT INTO Tbl VALUES(%d, 9234567890, 99.998, 'TEXT VALUE', x'AABBCCDDEEFF')";
const char KSingleUpdateSql[] = "UPDATE Tbl SET D=11.113 WHERE I=";
const char KSingleDeleteSql[] = "DELETE FROM Tbl WHERE I=";
const char KSingleSelectSql[] = "SELECT I64,D,T,B FROM Tbl WHERE I=";

//This buffer is used for SQL statement formatting. 2000 bytes should be enough as a buffer max length.
static TBuf8<2000> TheSqlBuf;
//This buffer is used for printf related formatting. 500 characters should be enough.
static TBuf<500> ThePrintBuf;

#define UNUSED_VAR(a) (a) = (a)
#define UNUSED_PTR(a) a.Set(a)

///////////////////////////////////////////////////////////////////////////////////////

extern "C" void TestAbort(TInt aLine)
	{
	TheTest(0, aLine);
	}

extern "C" const char* TestDbName(void)
	{
	return (const char*)KTestDbName8().Ptr();
	}

extern "C" void PrintS(const char* aFmt, const char* aMsg)
	{
	TheSqlBuf.Format(TPtrC8((const TUint8*)aFmt), aMsg);
	ThePrintBuf.Copy(TheSqlBuf);
	TheTest.Printf(ThePrintBuf);
	}

extern "C" void PrintI(const char* aFmt, int a1)
	{
	TheSqlBuf.Format(TPtrC8((const TUint8*)aFmt), a1);
	ThePrintBuf.Copy(TheSqlBuf);
	TheTest.Printf(ThePrintBuf);
	}

extern "C" void PrintIII(const char* aFmt, int a1, int a2, int a3)
	{
	TheSqlBuf.Format(TPtrC8((const TUint8*)aFmt), a1, a2, a3);
	ThePrintBuf.Copy(TheSqlBuf);
	TheTest.Printf(ThePrintBuf);
	}

static void DeleteTestFiles()
	{
	TheDb.Close();
	(void)RSqlDatabase::Delete(KTestDbName);
	}

///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
static void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DeleteTestFiles();
		TheTest(EFalse, aLine);
		}
	}
static void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		DeleteTestFiles();
		RDebug::Print(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

static TInt KillProcess(const TDesC& aProcessName)
	{
	TFullName name;
	//RDebug::Print(_L("Find and kill \"%S\" process.\n"), &aProcessName);
	TBuf<64> pattern(aProcessName);
	TInt length = pattern.Length();
	pattern += _L("*");
	TFindProcess procFinder(pattern);

	while (procFinder.Next(name) == KErrNone)
		{
		if (name.Length() > length)
			{//If found name is a string containing aProcessName string.
			TChar c(name[length]);
			if (c.IsAlphaDigit() ||
				c == TChar('_') ||
				c == TChar('-'))
				{
				// If the found name is other valid application name
				// starting with aProcessName string.
				//RDebug::Print(_L(":: Process name: \"%S\".\n"), &name);
				continue;
				}
			}
		RProcess proc;
		if (proc.Open(name) == KErrNone)
			{
			proc.Kill(0);
			//RDebug::Print(_L("\"%S\" process killed.\n"), &name);
			}
		proc.Close();
		}
	return KErrNone;
	}

static TInt TheCounterFreq = -10000000;
const TInt KMicroSecIn1Sec = 1000000;

//Prints aFastCount parameter (converted to us)
static TInt FcDiff2Us(TUint32 aFastCount)
	{
	double v = ((double)aFastCount * KMicroSecIn1Sec) / (double)TheCounterFreq;
	TInt v2 = (TInt)v;
	return v2;
	}

static void GetFastCounterFrequency()
	{
	TEST2(HAL::Get(HAL::EFastCounterFrequency, TheCounterFreq), KErrNone);
	TheTest.Printf(_L("Counter frequency=%d\r\n"), TheCounterFreq);
	}

extern "C" unsigned int FastCounterValue(void)
	{
	return User::FastCounter();
	}

//The performance test case results are stored in the ThePerfTestResult array.
static TUint32 ThePerfTestResult[EPerfTestModeCnt][EPerfTestTypeCnt];

extern "C" void StorePerfTestResult(TPerfTestMode aMode, TPerfTestType aType, unsigned int aResult)
	{
	ThePerfTestResult[aMode][aType] = aResult;
	}

static void PrintPerfTestResults()
	{
	TInt r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestMultiInsert]);
	TInt r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestMultiInsert]);
	TInt r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestMultiInsert]);
	TInt r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	TInt r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("###                 SQL            SQLITE-SQL     SQLITE-DEFAULT\r\n"));
	TheTest.Printf(_L("###Mutli Insert:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestMultiUpdate]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestMultiUpdate]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestMultiUpdate]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("###Mutli Update:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestMultiDelete]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestMultiDelete]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestMultiDelete]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("###Mutli Delete:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestMultiSelect]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestMultiSelect]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestMultiSelect]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("###Mutli Select:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestSingleInsert]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestSingleInsert]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestSingleInsert]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("##Single Insert:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestSingleUpdate]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestSingleUpdate]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestSingleUpdate]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("##Single Update:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestSingleDelete]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestSingleDelete]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestSingleDelete]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("##Single Delete:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);

	r1 = FcDiff2Us(ThePerfTestResult[EPerfTestSqlMode]          [EPerfTestSingleSelect]);
	r2 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteSqlMode]    [EPerfTestSingleSelect]);
	r3 = FcDiff2Us(ThePerfTestResult[EPerfTestSqliteDefaultMode][EPerfTestSingleSelect]);
	r4 = (TInt)((Abs(r2 - r1) * 100.0) / r1);
	r5 = (TInt)((Abs(r3 - r1) * 100.0) / r1);
	TheTest.Printf(_L("##Single Select:  %8dus     %8dus     %8dus   %8d%%   %8d%%\r\n"), r1, r2, r3, r2 > r1 ? -r4 : +r4, r3 > r1 ? -r5 : +r5);
	}

///////////////////////////////////////////////////////////////////////////////////////

static void CreateTestDir()
    {
    RFs fs;
	TInt err = fs.Connect();
	TEST2(err, KErrNone);

	err = fs.MkDir(KTestDir);
	TEST(err == KErrNone || err == KErrAlreadyExists);

	err = fs.CreatePrivatePath(EDriveC);
	TEST(err == KErrNone || err == KErrAlreadyExists);

	fs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////

static void CreateTestDatabase()
	{
	RSqlDatabase::Delete(KTestDbName);

	_LIT8(KConfigStr, "encoding=\"UTF-8\"");
	TInt err = TheDb.Create(KTestDbName, &KConfigStr);
	TEST2(err, KErrNone);

	err = TheDb.Exec(_L8("CREATE TABLE Tbl(I INTEGER PRIMARY KEY, I64 BIGINT, D DOUBLE, T TEXT, B BINARY)"));
	TEST2(err, 1);

	TheDb.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
////////////////////             SQL server tests           ///////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4010
@SYMTestCaseDesc		SQL server multi-insert performance test.
						The test inserts 1000 records in a single transaction and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server multi-insert performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerMultiInsertTest(const char aInsertSql[], TInt aInsertRecCnt)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4010 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	RSqlStatement stmt;
	err = stmt.Prepare(TheDb, TPtrC8((const TUint8*)aInsertSql));
	TEST2(err, KErrNone);

	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(_L8("BEGIN"));
	TEST(err >= 0);

	for(TInt i=0;i<aInsertRecCnt;++i)
		{
		err = stmt.BindInt(0, i + 1);
		TEST2(err, KErrNone);
		err = stmt.Exec();
		TEST2(err, 1);
		err = stmt.Reset();
		TEST2(err, KErrNone);
		}

	err = TheDb.Exec(_L8("COMMIT"));
	TEST(err >= 0);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestMultiInsert, FastCounterValue() - fc);

	stmt.Close();
	TheDb.Close();
	}

static void FormatSqlStmt(TDes8& aSqlBuf, const char aSql[], TInt aRecIds[], TInt aRecCnt)
	{
	aSqlBuf.Copy(TPtrC8((const TUint8*)aSql));
	aSqlBuf.Append(_L8("("));
	for(TInt i=0;i<aRecCnt;++i)
		{
		aSqlBuf.AppendNum((TInt64)aRecIds[i]);
		aSqlBuf.Append(_L8(","));
		}
	aSqlBuf.SetLength(aSqlBuf.Length() - 1);
	aSqlBuf.Append(_L8(")"));
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4011
@SYMTestCaseDesc		SQL server multi-update performance test.
						The test updates 100 randomly chosen records and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server multi-update performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerMultiUpdateTest(const char aUpdateSql[], TInt aUpdateRecIds[], TInt aUpdateRecCnt)
	{
    TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4011 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	FormatSqlStmt(TheSqlBuf, aUpdateSql, aUpdateRecIds, aUpdateRecCnt);

	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(TheSqlBuf);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestMultiUpdate, FastCounterValue() - fc);
	TEST2(err, aUpdateRecCnt);

	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4012
@SYMTestCaseDesc		SQL server multi-delete performance test.
						The test deletes 100 randomly chosen records and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server multi-delete performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerMultiDeleteTest(const char aDeleteSql[], TInt aDeleteRecIds[], TInt aDeleteRecCnt)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4012 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	FormatSqlStmt(TheSqlBuf, aDeleteSql, aDeleteRecIds, aDeleteRecCnt);

	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(TheSqlBuf);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestMultiDelete, FastCounterValue() - fc);
	TEST2(err, aDeleteRecCnt);

	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4013
@SYMTestCaseDesc		SQL server multi-select performance test.
						The test selects 100 randomly chosen records and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server multi-select performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerMultiSelectTest(const char aSelectSql[], TInt aSelectRecIds[], TInt aSelectRecCnt)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4013 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	FormatSqlStmt(TheSqlBuf, aSelectSql, aSelectRecIds, aSelectRecCnt);

	RSqlStatement stmt;
	err = stmt.Prepare(TheDb, TheSqlBuf);
	TEST2(err, KErrNone);
	TInt recCnt = 0;
	TUint32 fc = FastCounterValue();
	while((err = stmt.Next()) == KSqlAtRow)
		{
		TInt64 i64 = stmt.ColumnInt64(0);
		UNUSED_VAR(i64);
		TReal d = stmt.ColumnReal(1);
		UNUSED_VAR(d);
		TPtrC t;
		err = stmt.ColumnText(2, t);
		TEST2(err, KErrNone);
		UNUSED_PTR(t);
		TPtrC8 b;
		err = stmt.ColumnBinary(3, b);
		TEST2(err, KErrNone);
		UNUSED_PTR(b);
		++recCnt;
		}
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestMultiSelect, FastCounterValue() - fc);
	TEST2(err, KSqlAtEnd);
	TEST2(recCnt, aSelectRecCnt);

	stmt.Close();
	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4014
@SYMTestCaseDesc		SQL server single-insert performance test.
						The test inserts one record and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server single-insert performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerSingleInsertTest(const char aSingleInsertSql[], TInt aInsertRecId)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4014 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	TheSqlBuf.Format(TPtrC8((const TUint8*)aSingleInsertSql), aInsertRecId);
	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(TheSqlBuf);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestSingleInsert, FastCounterValue() - fc);
	TEST2(err, 1);

	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4015
@SYMTestCaseDesc		SQL server single-update performance test.
						The test updates one randomly chosen record and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server single-update performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerSingleUpdateTest(const char aSingleUpdateSql[], TInt aUpdateRecId)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4015 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	TheSqlBuf.Copy(TPtrC8((const TUint8*)aSingleUpdateSql));
	TheSqlBuf.AppendNum((TInt64)aUpdateRecId);
	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(TheSqlBuf);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestSingleUpdate, FastCounterValue() - fc);
	TEST2(err, 1);

	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4016
@SYMTestCaseDesc		SQL server single-delete performance test.
						The test deletes one randomly chosen record and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server single-delete performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerSingleDeleteTest(const char aSingleDeleteSql[], TInt aDeleteRecId)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4016 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	TheSqlBuf.Copy(TPtrC8((const TUint8*)aSingleDeleteSql));
	TheSqlBuf.AppendNum((TInt64)aDeleteRecId);
	TUint32 fc = FastCounterValue();
	err = TheDb.Exec(TheSqlBuf);
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestSingleDelete, FastCounterValue() - fc);
	TEST2(err, 1);

	TheDb.Close();
	}

/**
@SYMTestCaseID			SYSLIB-SQLITE3-UT-4017
@SYMTestCaseDesc		SQL server single-select performance test.
						The test selects one randomly chosen record and stores
						the execution time for later use (comparison and printing).
@SYMTestPriority		High
@SYMTestActions			SQL server single-select performance test.
@SYMTestExpectedResults Test must not fail
@SYMREQ					REQ8782
*/
static void SqlServerSingleSelectTest(const char aSingleSelectSql[], TInt aSelectRecId)
	{
	TheTest.Next( _L(" @SYMTestCaseID:SYSLIB-SQLITE3-UT-4017 "));
	(void)KillProcess(KSqlSrvName);

	TInt err = TheDb.Open(KTestDbName);
	TEST2(err, KErrNone);

	TheSqlBuf.Copy(TPtrC8((const TUint8*)aSingleSelectSql));
	TheSqlBuf.AppendNum((TInt64)aSelectRecId);

	RSqlStatement stmt;
	err = stmt.Prepare(TheDb, TheSqlBuf);
	TEST2(err, KErrNone);
	TInt recCnt = 0;
	TUint32 fc = FastCounterValue();
	while((err = stmt.Next()) == KSqlAtRow)
		{
		TInt64 i64 = stmt.ColumnInt64(0);
		UNUSED_VAR(i64);
		TReal d = stmt.ColumnReal(1);
		UNUSED_VAR(d);
		TPtrC t;
		err = stmt.ColumnText(2, t);
		TEST2(err, KErrNone);
		UNUSED_PTR(t);
		TPtrC8 b;
		err = stmt.ColumnBinary(3, b);
		TEST2(err, KErrNone);
		UNUSED_PTR(b);
		++recCnt;
		}
	StorePerfTestResult(EPerfTestSqlMode, EPerfTestSingleSelect, FastCounterValue() - fc);
	TEST2(err, KSqlAtEnd);
	TEST2(recCnt, 1);

	stmt.Close();
	TheDb.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

static void DoSqlServerTests()
	{
	TheTest.Next(_L("SQL: Create the test database"));
	CreateTestDatabase();

	TBuf<100> msgbuf;

	msgbuf.Format(_L("SQL: insert %d records in a single transaction"), KInsertRecCnt);
	TheTest.Next(msgbuf);
	SqlServerMultiInsertTest(KMultiInsertSql, KInsertRecCnt);

	msgbuf.Format(_L("SQL: update %d records in a single transaction"), KUpdateRecCnt);
	TheTest.Next(msgbuf);
	SqlServerMultiUpdateTest(KMultiUpdateSql, TheUpdateRecIds, KUpdateRecCnt);

	msgbuf.Format(_L("SQL: delete %d records in a single transaction"), KDeleteRecCnt);
	TheTest.Next(msgbuf);
	SqlServerMultiDeleteTest(KMultiDeleteSql, TheDeleteRecIds, KDeleteRecCnt);

	msgbuf.Format(_L("SQL: select %d records"), KSelectRecCnt);
	TheTest.Next(msgbuf);
	SqlServerMultiSelectTest(KMultiSelectSql, TheSelectRecIds, KSelectRecCnt);

	TheTest.Next(_L("SQL: insert a single record"));
	SqlServerSingleInsertTest(KSingleInsertSql, TheInsertRecId);

	TheTest.Next(_L("SQL: update a single record"));
	SqlServerSingleUpdateTest(KSingleUpdateSql, TheUpdateRecId);

	TheTest.Next(_L("SQL: delete a single record"));
	SqlServerSingleDeleteTest(KSingleDeleteSql, TheDeleteRecId);

	TheTest.Next(_L("SQL: select a single record"));
	SqlServerSingleSelectTest(KSingleSelectSql, TheSelectRecId);

	(void)RSqlDatabase::Delete(KTestDbName);
	}

static void DoSqliteLibraryTests(TPerfTestMode aPerfTestMode)
	{
	TEST(aPerfTestMode > EPerfTestSqlMode && aPerfTestMode < EPerfTestModeCnt);

	SqliteInitialize(aPerfTestMode);

	TheTest.Next( _L("SQLite: Create the test database"));
	CreateTestDatabase();

	TBuf<200> msgbuf;
	_LIT(KSqliteConfigSql, "\"SQL\"");
	_LIT(KSqliteConfigDefault, "\"Default\"");

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4018: SQLite, configuration: %S: insert %d records in a single transaction"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault, KInsertRecCnt);
	TheTest.Next(msgbuf);
	SqliteMultiInsertTest(aPerfTestMode, KMultiInsertSql, KInsertRecCnt);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4019: SQLite, configuration: %S: update %d records in a single transaction"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault, KUpdateRecCnt);
	TheTest.Next(msgbuf);
	SqliteMultiUpdateTest(aPerfTestMode, KMultiUpdateSql, TheUpdateRecIds, KUpdateRecCnt);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4020: SQLite, configuration: %S: delete %d records in a single transaction"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault, KDeleteRecCnt);
	TheTest.Next(msgbuf);
	SqliteMultiDeleteTest(aPerfTestMode, KMultiDeleteSql, TheDeleteRecIds, KDeleteRecCnt);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4021: SQLite, configuration: %S: select %d records"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault, KSelectRecCnt);
	TheTest.Next(msgbuf);
	SqliteMultiSelectTest(aPerfTestMode, KMultiSelectSql, TheSelectRecIds, KSelectRecCnt);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4022: SQLite, configuration: %S: insert a single record"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault);
	TheTest.Next(msgbuf);
	SqliteSingleInsertTest(aPerfTestMode, KSingleInsertSql, TheInsertRecId);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4023: SQLite, configuration: %S: update a single record"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault);
	TheTest.Next(msgbuf);
	SqliteSingleUpdateTest(aPerfTestMode, KSingleUpdateSql, TheUpdateRecId);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4024: SQLite, configuration: %S: delete a single record"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault);
	TheTest.Next(msgbuf);
	SqliteSingleDeleteTest(aPerfTestMode, KSingleDeleteSql, TheDeleteRecId);

	msgbuf.Format(_L("@SYMTestCaseID:SYSLIB-SQLITE3-UT-4025: SQLite, configuration: %S: select a single record"),
							aPerfTestMode == EPerfTestSqliteSqlMode ? &KSqliteConfigSql : &KSqliteConfigDefault);
	TheTest.Next(msgbuf);
	SqliteSingleSelectTest(aPerfTestMode, KSingleSelectSql, TheSelectRecId);

	(void)RSqlDatabase::Delete(KTestDbName);

	SqliteFinalize(aPerfTestMode);
	}

static TBool IdIn(TInt aId, const TInt aData[], TInt aDataSize)
	{
	TEST(aId > 0);
	TEST(aDataSize >= 0);
	for(TInt i=0;i<aDataSize;++i)
		{
		TEST(aData[i] > 0);
		if(aData[i] == aId)
			{
			return ETrue;
			}
		}
	return EFalse;
	}

static void VerifyGeneratedRecIds()
	{
	TInt i;

	for(i=0;i<KUpdateRecCnt;++i)
		{
		TEST(TheUpdateRecIds[i] > 0 && TheUpdateRecIds[i] <= KInsertRecCnt);
		TEST(!IdIn(TheUpdateRecIds[i], TheUpdateRecIds, i));
		TEST(!IdIn(TheUpdateRecIds[i], TheDeleteRecIds, KDeleteRecCnt));
		TEST(!IdIn(TheUpdateRecIds[i], TheSelectRecIds, KSelectRecCnt));
		}

	for(i=0;i<KDeleteRecCnt;++i)
		{
		TEST(TheDeleteRecIds[i] > 0 && TheDeleteRecIds[i] <= KInsertRecCnt);
		TEST(!IdIn(TheDeleteRecIds[i], TheDeleteRecIds, i));
		TEST(!IdIn(TheDeleteRecIds[i], TheUpdateRecIds, KUpdateRecCnt));
		TEST(!IdIn(TheDeleteRecIds[i], TheSelectRecIds, KSelectRecCnt));
		}

	for(i=0;i<KSelectRecCnt;++i)
		{
		TEST(TheSelectRecIds[i] > 0 && TheSelectRecIds[i] <= KInsertRecCnt);
		TEST(!IdIn(TheSelectRecIds[i], TheSelectRecIds, i));
		TEST(!IdIn(TheSelectRecIds[i], TheUpdateRecIds, KUpdateRecCnt));
		TEST(!IdIn(TheSelectRecIds[i], TheDeleteRecIds, KDeleteRecCnt));
		}

	TEST(TheInsertRecId > 0);

	TEST(TheUpdateRecId > 0 && TheUpdateRecId <= KInsertRecCnt);
	TEST(!IdIn(TheUpdateRecId, TheUpdateRecIds, KUpdateRecCnt));
	TEST(!IdIn(TheUpdateRecId, TheDeleteRecIds, KDeleteRecCnt));
	TEST(!IdIn(TheUpdateRecId, TheSelectRecIds, KSelectRecCnt));

	TEST(TheDeleteRecId > 0 && TheDeleteRecId <= KInsertRecCnt);
	TEST(!IdIn(TheDeleteRecId, TheUpdateRecIds, KUpdateRecCnt));
	TEST(!IdIn(TheDeleteRecId, TheDeleteRecIds, KDeleteRecCnt));
	TEST(!IdIn(TheDeleteRecId, TheSelectRecIds, KSelectRecCnt));

	TEST(TheSelectRecId > 0 && TheSelectRecId <= KInsertRecCnt);
	TEST(!IdIn(TheSelectRecId, TheUpdateRecIds, KUpdateRecCnt));
	TEST(!IdIn(TheSelectRecId, TheDeleteRecIds, KDeleteRecCnt));
	TEST(!IdIn(TheSelectRecId, TheSelectRecIds, KSelectRecCnt));
	}

static void GenerateTestRecIds()
	{
	TTime now;
	now.UniversalTime();
	TInt64 seed = now.Int64();
	TInt i;

	//Update multi
	for(i=0;i<KUpdateRecCnt;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && !IdIn(id, TheUpdateRecIds, i))
			{
			TheUpdateRecIds[i++] = id;
			}
		}
	//Delete multi
	for(i=0;i<KDeleteRecCnt;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && !IdIn(id, TheDeleteRecIds, i) && !IdIn(id, TheUpdateRecIds, KUpdateRecCnt))
			{
			TheDeleteRecIds[i++] = id;
			}
		}
	//Select multi
	for(i=0;i<KSelectRecCnt;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && !IdIn(id, TheSelectRecIds, i) && !IdIn(id, TheUpdateRecIds, KUpdateRecCnt) &&
		             !IdIn(id, TheDeleteRecIds, KDeleteRecCnt))
			{
			TheSelectRecIds[i++] = id;
			}
		}
	//Insert one
	TheInsertRecId = KInsertRecCnt + 1;
	//Update one
	for(;;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && !IdIn(id, TheUpdateRecIds, KUpdateRecCnt) && !IdIn(id, TheDeleteRecIds, KDeleteRecCnt) &&
		             !IdIn(id, TheSelectRecIds, KSelectRecCnt))
			{
			TheUpdateRecId = id;
			break;
			}
		}
	//Delete one
	for(;;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && id != TheUpdateRecId && !IdIn(id, TheUpdateRecIds, KUpdateRecCnt) &&
		                                     !IdIn(id, TheDeleteRecIds, KDeleteRecCnt) && !IdIn(id, TheSelectRecIds, KSelectRecCnt))
			{
			TheDeleteRecId = id;
			break;
			}
		}
	//Select one
	for(;;)
		{
		TInt id = Math::Rand(seed) % KInsertRecCnt;
		if(id > 0 && id != TheUpdateRecId && id != TheDeleteRecId && !IdIn(id, TheUpdateRecIds, KUpdateRecCnt) &&
		                                                             !IdIn(id, TheDeleteRecIds, KDeleteRecCnt) &&
		                                                             !IdIn(id, TheSelectRecIds, KSelectRecCnt))
			{
			TheSelectRecId = id;
			break;
			}
		}
	}

static void DoTests()
	{
	TheTest.Start(_L("Get fast counter frequency"));
	GetFastCounterFrequency();

	TheTest.Next( _L("Generate test record ids"));
	GenerateTestRecIds();

	TheTest.Next( _L("Verify generated test record ids"));
	VerifyGeneratedRecIds();

	DoSqlServerTests();
	(void)KillProcess(KSqlSrvName);

	DoSqliteLibraryTests(EPerfTestSqliteDefaultMode);
	
	DoSqliteLibraryTests(EPerfTestSqliteSqlMode);

	PrintPerfTestResults();
	}

TInt E32Main()
	{
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();

	__UHEAP_MARK;

	CreateTestDir();
	DeleteTestFiles();
	DoTests();
	DeleteTestFiles();

	__UHEAP_MARKEND;

	TheTest.End();
	TheTest.Close();

	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
