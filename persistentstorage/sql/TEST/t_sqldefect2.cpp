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
#include <f32file.h>
#include <sqldb.h>

///////////////////////////////////////////////////////////////////////////////////////

static RFs TheFs;
static RTest TheTest(_L("t_sqldefect2 test"));
static RSqlDatabase TheDb1;
static RSqlDatabase TheDb2;

_LIT(KTestDir, "c:\\test\\");
_LIT(KTestDatabase1, "c:\\test\\t_sqldefect2.db");
_LIT(KTestDatabaseJournal1, "c:\\test\\t_sqldefect2.db-journal");


///////////////////////////////////////////////////////////////////////////////////////

//Deletes all created test files.
void DestroyTestEnv()
	{
    TheDb2.Close();
    TheDb1.Close();
	(void)RSqlDatabase::Delete(KTestDatabase1);
	TheFs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DestroyTestEnv();
		RDebug::Print(_L("*** Line %d\r\n"), aLine);
		TheTest(EFalse, aLine);
		}
	}
void Check2(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		DestroyTestEnv();
		RDebug::Print(_L("*** Line %d, Expected error: %d, got: %d\r\n"), aLine, aExpected, aValue);
		TheTest(EFalse, aLine);
		}
	}
#define TEST(arg) ::Check1((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check2(aValue, aExpected, __LINE__)

///////////////////////////////////////////////////////////////////////////////////////

//Creates file session instance and the test directory
void CreateTestEnv()
    {
	TInt err = TheFs.Connect();
	TEST2(err, KErrNone);

	err = TheFs.MkDir(KTestDir);
	TEST(err == KErrNone || err == KErrAlreadyExists);
	}

/**
@SYMTestCaseID          PDS-SQL-CT-4154
@SYMTestCaseDesc        Test for DEF143062: SQL, "CREATE INDEX" sql crashes SQL server.
                        The test creates a database with one empty table and establishes two connections
                        to that database. Then, while the first connection is at the middle of a read
                        transaction, the second connection attempts to create an index.
                        If the defect is not fixed, the SQL server will crash.
@SYMTestPriority        High
@SYMTestActions         DEF143062: SQL, "CREATE INDEX" sql crashes SQL server.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF143062
*/
void DEF143062()
    {
    (void)RSqlDatabase::Delete(KTestDatabase1);
    TInt err = TheDb1.Create(KTestDatabase1);
    TEST2(err, KErrNone);
    err = TheDb1.Exec(_L("CREATE TABLE T0(Thread INTEGER, LocalIndex INTEGER, Inserts INTEGER, Updates INTEGER, IndexMod8 INTEGER)"));
    TEST(err >= 0);
    
    err = TheDb2.Open(KTestDatabase1);
    TEST2(err, KErrNone);

    RSqlStatement stmt;
    err = stmt.Prepare(TheDb1, _L8("SELECT COUNT(Thread) FROM T0 WHERE Thread = 0"));
    TEST2(err, KErrNone);
    err = stmt.Next();
    TEST2(err, KSqlAtRow);
    
    err = TheDb2.Exec(_L8("CREATE INDEX T0INDEX ON T0(Thread,IndexMod8)"));//crashes the SQL server if the defect is not fixed 
    TEST2(err, KSqlErrLocked);
    
    stmt.Close();
    
    TheDb2.Close();
    TheDb1.Close();
    err = RSqlDatabase::Delete(KTestDatabase1);
    TEST2(err, KErrNone);
    }

/**
@SYMTestCaseID          PDS-SQL-CT-4155
@SYMTestCaseDesc        Test for DEF143061: SQL, SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT value is too big.
                                         The test verifies that after comitting a big transaction, the journal file size is made equal the 
                                          max journal file size limit of 64Kb.
@SYMTestPriority        High
@SYMTestActions         DEF143061: SQL, SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT value is too big..
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF143061
*/
void DEF143061()
    {
    (void)RSqlDatabase::Delete(KTestDatabase1);
    //"Auto" compaction is used in order to see how the journal file is immediatelly used.
    _LIT8(KConfig, "compaction=auto");
    TInt err = TheDb1.Create(KTestDatabase1, &KConfig);
    TEST2(err, KErrNone);
    err = TheDb1.Exec(_L("CREATE TABLE A(I INTEGER, B BLOB)"));
    TEST(err >= 0);

    const TInt KBlobSize = 100000;//bigger than the journal size limit
    HBufC8* buf = HBufC8::New(KBlobSize);
    TEST(buf != NULL);
    TPtr8 ptr = buf->Des();
    ptr.SetLength(KBlobSize);
        
    RSqlStatement stmt;
    err = stmt.Prepare(TheDb1, _L("INSERT INTO A VALUES(1, :Prm)"));
    TEST2(err, KErrNone);
    ptr.Fill(TChar('N'));
    err = stmt.BindBinary(0, ptr);
    TEST2(err, KErrNone);
    err = stmt.Exec();
    TEST2(err, 1);
    stmt.Close();
    
    //Try to update the BLOB in the record that was just inserted. This operation should create a big journal file.
    err = stmt.Prepare(TheDb1, _L("UPDATE A SET B=:Prm WHERE I=1"));
    TEST2(err, KErrNone);
    ptr.Fill(TChar('Y'));
    err = stmt.BindBinary(0, ptr);
    TEST2(err, KErrNone);
    err = stmt.Exec();
    TEST2(err, 1);
    stmt.Close();
    
    //Check the journal file size. It should be less than the 64Kb limit defined in sqlite_macro.mmh file.  
    TEntry entry;
    err = TheFs.Entry(KTestDatabaseJournal1, entry);
    TEST2(err, KErrNone);
    TInt64 fsize = entry.FileSize();
    TEST(fsize <= SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT);
    
    delete buf;
    TheDb1.Close();
    err = RSqlDatabase::Delete(KTestDatabase1);
    TEST2(err, KErrNone);
    }

/**
@SYMTestCaseID          PDS-SQL-CT-4156
@SYMTestCaseDesc        Test for DEF143150: SQL, strftime() returns incorrect result.
                        The test takes the current universal time (using TTime) 
                        and the current time retrieved from the SQL  server.
                        The test compares the times and expects the difference to be no more than
                        1 second. 
@SYMTestPriority        High
@SYMTestActions         DEF143150: SQL, strftime() returns incorrect result
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF143150
*/
void DEF143150()
    {
    (void)RSqlDatabase::Delete(KTestDatabase1);
    TInt err = TheDb1.Create(KTestDatabase1);
    TEST2(err, KErrNone);

    //Home date & time
    TBuf<50> dtstr1;
    TTime time;
    time.UniversalTime();
    TDateTime dt = time.DateTime();
    
    RSqlStatement stmt;
    err = stmt.Prepare(TheDb1, _L("SELECT strftime('%Y-%m-%d,%H:%M:%S','now')"));
    TEST2(err, KErrNone);
    err = stmt.Next();
    TEST2(err, KSqlAtRow);
    
    //SQLite date & time
    TBuf<50> dtstr2;
    err = stmt.ColumnText(0, dtstr2);
    TEST2(err, KErrNone);
    stmt.Close();

    TheDb1.Close();
    err = RSqlDatabase::Delete(KTestDatabase1);
    TEST2(err, KErrNone);
    
    dtstr1.Format(_L("%04d-%02d-%02d,%02d:%02d:%02d"), dt.Year(), dt.Month() + 1, dt.Day() + 1, dt.Hour(), dt.Minute(), dt.Second());
    TheTest.Printf(_L("Universal date&time=\"%S\"\n"), &dtstr1);
    TheTest.Printf(_L("SQLite    date&time=\"%S\"\n"), &dtstr2);
    
    //Comapare and fail if dates are not equal (+- 1 second)
    TLex lex;
    lex = dtstr2.Mid(0, 4);
    TInt sqlyear;
    err = lex.Val(sqlyear);
    TEST2(err, KErrNone);
    
    lex = dtstr2.Mid(5, 2);
    TInt sqlmonth;
    err = lex.Val(sqlmonth);
    TEST2(err, KErrNone);
    
    lex = dtstr2.Mid(8, 2);
    TInt sqlday;
    err = lex.Val(sqlday);
    TEST2(err, KErrNone);
    
    lex = dtstr2.Mid(11, 2);
    TInt sqlhour;
    err = lex.Val(sqlhour);
    TEST2(err, KErrNone);
    
    lex = dtstr2.Mid(14, 2);
    TInt sqlminute;
    err = lex.Val(sqlminute);
    TEST2(err, KErrNone);
    
    lex = dtstr2.Mid(17, 2);
    TInt sqlsecond;
    err = lex.Val(sqlsecond);
    TEST2(err, KErrNone);
    
    TDateTime sqldt(sqlyear, (TMonth)(sqlmonth - 1), sqlday - 1, sqlhour, sqlminute, sqlsecond, 0);
    TTime sqltime(sqldt);
    TTimeIntervalSeconds diff;
    err = sqltime.SecondsFrom(time, diff);
    TEST2(err, KErrNone);
    TEST(diff.Int() <= 1);
    }

void DoTestsL()
	{
	TheTest.Start(_L(" @SYMTestCaseID:SYSLIB-SQL-CT-4154 DEF143062: SQL, \"CREATE INDEX\" sql crashes SQL server"));
	DEF143062();

    TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-CT-4155 DEF143061: SQL, SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT value is too big"));
    DEF143061();

    TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-SQL-CT-4156 DEF143150: SQL, strftime() returns incorrect result"));
    DEF143150();
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	
	__UHEAP_MARK;
	
	CreateTestEnv();
	TRAPD(err, DoTestsL());
	DestroyTestEnv();
	TEST2(err, KErrNone);

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
