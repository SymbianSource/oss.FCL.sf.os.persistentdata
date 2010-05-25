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
//
#include <e32test.h>
#include <bautils.h>
#include <sqldb.h>

///////////////////////////////////////////////////////////////////////////////////////

RSqlDatabase TheDb;
RTest TheTest(_L("t_sqlcorrupt test"));

_LIT(KTestDir, "c:\\test\\");

_LIT(KDbName, "c:[08770000]t_sqlcorrupt.db");
_LIT(KFullDbName, "c:\\private\\10281E17\\[08770000]t_sqlcorrupt.db");

_LIT(KDbName2, "c:[08770000]t_sqlcorrupt2.db");
_LIT(KFullDbName2, "c:\\private\\10281E17\\[08770000]t_sqlcorrupt2.db");

RFs TheFs;

///////////////////////////////////////////////////////////////////////////////////////

void DestroyTestEnv()
	{
	TheDb.Close();
	(void)RSqlDatabase::Delete(KDbName2);
	(void)RSqlDatabase::Delete(KDbName);
	TheFs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check1(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DestroyTestEnv();
		RDebug::Print(_L("*** Boolean expression evaluated to false. Line %d\r\n"), aLine);
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

enum TDbEncoding
	{
	EDbEncUtf16,
	EDbEncUtf8,
	};

void DoCorruptedSecureDbTest(TDbEncoding aEncoding)
	{
	(void)RSqlDatabase::Delete(KDbName);
	
	RSqlSecurityPolicy policy;
	TInt err = policy.Create(TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	
	err = policy.SetDbPolicy(RSqlSecurityPolicy::ESchemaPolicy, TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	err = policy.SetDbPolicy(RSqlSecurityPolicy::EWritePolicy, TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	err = policy.SetDbPolicy(RSqlSecurityPolicy::EReadPolicy, TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	
	err = policy.SetPolicy(RSqlSecurityPolicy::ETable, _L("A"), RSqlSecurityPolicy::EWritePolicy, TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	err = policy.SetPolicy(RSqlSecurityPolicy::ETable, _L("A"), RSqlSecurityPolicy::EReadPolicy, TSecurityPolicy::EAlwaysPass);
	TEST2(err, KErrNone);
	
	if(aEncoding == EDbEncUtf16)
		{
		err = TheDb.Create(KDbName, policy);
		}
	else
		{
		_LIT8(KConfig, "encoding = \"UTF-8\"");
		err = TheDb.Create(KDbName, policy, &KConfig);
		}
	TEST2(err, KErrNone);
	err = TheDb.Exec(_L("CREATE TABLE A(I INTEGER); INSERT INTO A VALUES(10)"));
	TEST(err >= 0);
	TheDb.Close();
	policy.Close();

	CFileMan* fm = NULL;
	TRAP(err, fm = CFileMan::NewL(TheFs));
	TEST2(err, KErrNone);
	
	//Make a copy of the database
	err = fm->Copy(KFullDbName, KFullDbName2);
	TEST2(err, KErrNone);
	//Get the database file size and calculate the iterations count.
	TEntry entry;
	err = TheFs.Entry(KFullDbName, entry);
	TEST2(err, KErrNone);
	const TInt KCorruptBlockLen = 19;
	const TInt KIterationCnt = entry.iSize / KCorruptBlockLen;
	//
	TBuf8<KCorruptBlockLen> invalidData;
	invalidData.SetLength(KCorruptBlockLen);
	invalidData.Fill(TChar(0xCC));
	//
	for(TInt i=0;i<KIterationCnt;++i)
		{
		TheTest.Printf(_L("% 4d\r"), i + 1);
		//Corrupt the database
		err = fm->Copy(KFullDbName2, KFullDbName);
		TEST2(err, KErrNone);
		RFile file;
		err = file.Open(TheFs, KFullDbName, EFileRead | EFileWrite);
		TEST2(err, KErrNone);
		err = file.Write(i * KCorruptBlockLen, invalidData);
		TEST2(err, KErrNone);
		file.Close();
		//Try to open the database and read the record
		TBool testPassed = EFalse;
		err = TheDb.Open(KDbName);
		if(err == KErrNone)
			{
			RSqlStatement stmt;
			err = stmt.Prepare(TheDb, _L("SELECT I FROM A"));
			if(err == KErrNone)
				{
				err = stmt.Next();
				if(err == KSqlAtRow)
					{
					TInt val = stmt.ColumnInt(0);
					if(val == 10)
						{
						testPassed = ETrue;
						err = KErrNone;
						}
					else
						{
						err = KErrGeneral;
						}
					}
				stmt.Close();
				}
			}

		TheDb.Close();
		(void)RSqlDatabase::Delete(KDbName);
		TheTest.Printf(_L("Iteration % 4d, err=%d\r\n"), i + 1, err);
		if(!testPassed)
			{
			TEST(err != KErrNone);
			}
		}//end of - for(TInt i=0;i<KIterationCnt;++i)

	delete fm;
	TheTest.Printf(_L("\r\n"));
	}

/**
@SYMTestCaseID          PDS-SQL-CT-4202
@SYMTestCaseDesc        Invalid UTF16 encoded secure database test.
@SYMTestPriority        High
@SYMTestActions         The test creates 16-bit encoded secure database with one table and one record.
						Then the test simulates a database corruption by writing 19 bytes with random values
						from "pos" to "pos + 19", where "pos" is a valid db file position, incremented by 19
						at the end of each test iteration.
@SYMTestExpectedResults Test must not fail
*/  
void CorruptedSecureDbTest16()
	{
	DoCorruptedSecureDbTest(EDbEncUtf16);
	}

/**
@SYMTestCaseID          PDS-SQL-CT-4202
@SYMTestCaseDesc        Invalid UTF8 encoded secure database test.
@SYMTestPriority        High
@SYMTestActions         The test creates 8-bit encoded secure database with one table and one record.
						Then the test simulates a database corruption by writing 19 bytes with random values
						from "pos" to "pos + 19", where "pos" is a valid db file position, incremented by 19
						at the end of each test iteration.
@SYMTestExpectedResults Test must not fail
*/  
void CorruptedSecureDbTest8()
	{
	DoCorruptedSecureDbTest(EDbEncUtf8);
	}

void CreateTestEnv()
    {
    TInt err = TheFs.Connect();
    TEST2(err, KErrNone);

    err = TheFs.MkDir(KTestDir);
    TEST(err == KErrNone || err == KErrAlreadyExists);

    err = TheFs.CreatePrivatePath(EDriveC);
    TEST(err == KErrNone || err == KErrAlreadyExists);
    }

void DoTestsL()
	{
	TheTest.Start(_L("@SYMTestCaseID:PDS-SQL-CT-4202 Corrupted UTF16 encoded secure database test"));
	CorruptedSecureDbTest16();
	
	TheTest.Next(_L("@SYMTestCaseID:PDS-SQL-CT-4203 Corrupted UTF8 encoded secure database test"));
	CorruptedSecureDbTest8();
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);
	
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
