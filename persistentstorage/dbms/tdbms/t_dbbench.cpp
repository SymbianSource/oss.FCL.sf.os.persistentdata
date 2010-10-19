// Copyright (c) 1998-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <d32dbms.h>
#include <s32file.h>
#include <e32test.h>
#include <e32math.h>
#include <e32svr.h>
#include <hal.h>
#include <d32dbmsconstants.h>
#include "t_dbcmdlineutil.h"

TCmdLineParams TheCmdLineParams;
TFileName TheDbFileName;
RFile TheLogFile; 
RTest TheTest(_L("t_dbbench"));
RDbNamedDatabase TheDatabase;
RDbView TheView;
RFs TheFs;

TBuf<250> TheLogLine;
TBuf8<250> TheLogLine8;

const TPtrC KTableName=_S("Test");
const TPtrC KColCluster=_S("Cluster");
const TPtrC KColXcluster=_S("xCluster");
const TPtrC KColRandom=_S("Random");
const TPtrC KColXrandom=_S("xRandom");
const TInt  KRecords=2000;

struct TTest
	{
	const TText* iName;
	const TText* iQuery;
	};

const TTest KQuery[]=
	{
	{_S("project"),_S("select cluster,xcluster,random,xrandom from test")},
	{_S("restrict 1"),_S("select * from test where cluster=0")},
	{_S("restrict 2"),_S("select * from test where xrandom=0")},
	{_S("restrict 3"),_S("select * from test where xcluster<500 and xrandom <500")},
	{_S("order 1"),_S("select * from test order by xrandom")},
	{_S("order 2"),_S("select * from test order by cluster")},
	{_S("all 1"),_S("select * from test where random<500 order by xrandom")},
	{_S("all 2"),_S("select * from test where xcluster<500 order by xrandom")},
	{_S("all 3"),_S("select * from test where xcluster<500 order by xcluster")},
	{_S("all 4"),_S("select * from test where xcluster<500 and xrandom<200 order by xcluster")}
	};

///////////////////////////////////////////////////////////////////////////////////////

void TestEnvDestroy()
	{
	TheView.Close();
	TheDatabase.Close();
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		(void)TheLogFile.Flush();
		TheLogFile.Close();
		}
	//T_BENCH.DB cannot be deleted here, because it is used by T_DBCOMP test!
	TheFs.Close();
	}

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

TInt FastCounterFrequency()
	{
	static TInt freq = 0;
	if(freq == 0)
		{
		TEST2(HAL::Get(HAL::EFastCounterFrequency, freq), KErrNone);
		}
	return freq;
	}

//Prints the test case title and execution time in microseconds
void PrintResult(const TDesC& aTitle, TUint32 aStartTicks, TUint32 aEndTicks, TInt aIterations = 0)
	{
	TInt freq = FastCounterFrequency();
	TInt64 diffTicks = (TInt64)aEndTicks - (TInt64)aStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	if(aIterations > 0)
		{
		us /= aIterations;
		}
	TheTest.Printf(_L("%S: %d us\r\n"), &aTitle, us);
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		TheLogLine.Format(_L("%S¬%d¬us\r\n"), &aTitle, us);
		TheLogLine8.Copy(TheLogLine);
		(void)TheLogFile.Write(TheLogLine8);
		}
	}

//Calculates time in microseconds
TInt CalcTime(TUint32 aStartTicks, TUint32 aEndTicks)
	{
	TInt freq = FastCounterFrequency();
	TInt64 diffTicks = (TInt64)aEndTicks - (TInt64)aStartTicks;
	if(diffTicks < 0)
		{
		diffTicks = KMaxTUint32 + diffTicks + 1;
		}
	const TInt KMicroSecIn1Sec = 1000000;
	TInt32 us = (diffTicks * KMicroSecIn1Sec) / freq;
	return us;
	}

///////////////////////////////////////////////////////////////////////////////////////

/**
Create the database: keep the code 050 compatible

@SYMTestCaseID          SYSLIB-DBMS-CT-0577
@SYMTestCaseDesc        Benchmark Tests. Creation of a local Database test
@SYMTestPriority        Medium
@SYMTestActions        	Attempt to test RDbNamedDatabase::CreateTable(),RDbNamedDatabase::CreateIndex(),
						RDbNamedDatabase::Compact(),RDbView::Prepare() functions
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
void CreateDatabaseL()
	{
	TInt err = TheDatabase.Replace(TheFs, TheDbFileName);
	TEST2(err, KErrNone);
	
	CDbColSet* set = CDbColSet::NewLC();
	TDbCol col(KColCluster,EDbColInt32);
	col.iAttributes=col.ENotNull;
	set->AddL(col);
	col.iName=KColXcluster;
	set->AddL(col);
	col.iName=KColRandom;
	set->AddL(col);
	col.iName=KColXrandom;
	set->AddL(col);
	
	err = TheDatabase.CreateTable(KTableName, *set);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(set);
	
	TUint32 ticksStart = User::FastCounter();
	err = TheView.Prepare(TheDatabase,_L("select * from test"),TheView.EInsertOnly);
	TEST2(err, KErrNone);
	TheDatabase.Begin();
	TInt jj=0;
	for (TInt ii=0;ii<KRecords;++ii)
		{
		TheView.InsertL();
		jj=(jj+23);
		if (jj>=KRecords)
			jj-=KRecords;
		TheView.SetColL(1,ii);
		TheView.SetColL(2,ii);
		TheView.SetColL(3,jj);
		TheView.SetColL(4,jj);
		TheView.PutL();
		}
	err = TheDatabase.Commit();
	TEST2(err, KErrNone);
	TheView.Close();
	TUint32 ticksEnd = User::FastCounter();
	PrintResult(_L("Build table"), ticksStart, ticksEnd);
	
	ticksStart = User::FastCounter();
	CDbKey* key = CDbKey::NewLC();
	key->AddL(KColXcluster);
	key->MakeUnique();
	err = TheDatabase.CreateIndex(KColXcluster,KTableName,*key);
	TEST2(err, KErrNone);
	ticksEnd = User::FastCounter();
	PrintResult(_L("Cluster index"), ticksStart, ticksEnd);
	
	ticksStart = User::FastCounter();
	key->Clear();
	key->AddL(KColXrandom);
	err = TheDatabase.CreateIndex(KColXrandom,KTableName,*key);
	TEST2(err, KErrNone);
	CleanupStack::PopAndDestroy(key);
	ticksEnd = User::FastCounter();
	PrintResult(_L("Random index"), ticksStart, ticksEnd);
	
	ticksStart = User::FastCounter();
	err = TheDatabase.Compact();
	ticksEnd = User::FastCounter();
	PrintResult(_L("Compact"), ticksStart, ticksEnd);
	TEST2(err, KErrNone);
	}

void Evaluate(const TDesC& aTitle, const TDesC& aSql)
	{
	TInt m = 1;
	for(;;)
		{
		TUint32 ticksStart = User::FastCounter();
		for(TInt i=0; i<m; ++i)
			{
			TInt err = TheView.Prepare(TheDatabase,aSql,KDbUnlimitedWindow,TheView.EReadOnly);
			TEST2(err, KErrNone);
			err = TheView.EvaluateAll();
			TEST2(err, KErrNone);
			TheView.Close();
			}
		TUint32 ticksEnd = User::FastCounter();
		TInt us = CalcTime(ticksStart, ticksEnd);
		if(us > 100000)
			{
			PrintResult(aTitle, ticksStart, ticksEnd, m);
			return;
			}
		m *= 4;
		}
	}

/**
@SYMTestCaseID          SYSLIB-DBMS-CT-0578
@SYMTestCaseDesc        Benchmark Test.Querying a local Database Test
@SYMTestPriority        Medium
@SYMTestActions        	Evaluate SELECT queries on the created database
@SYMTestExpectedResults Test must not fail
@SYMREQ                 REQ0000
*/
void Queries()
	{
	for(TUint ii=0;ii<sizeof(KQuery)/sizeof(KQuery[0]);++ii)
		{
		Evaluate(TPtrC(KQuery[ii].iName), TPtrC(KQuery[ii].iQuery));
		}
	}

void BenchTestL()
	{
	TheTest.Start(_L("@SYMTestCaseID:SYSLIB-DBMS-CT-0577 RDbNamedDatabase performance test"));
	CreateDatabaseL();
	
	TheTest.Next(_L(" @SYMTestCaseID:SYSLIB-DBMS-CT-0578 SQL queries performance test"));
	Queries();
	
	TheDatabase.Close();
	}

void TestEnvInit()
    {
	TInt err = TheFs.Connect();
	TEST2(err, KErrNone);
	
	err = TheFs.MkDirAll(TheDbFileName);
	TEST(err == KErrNone || err == KErrAlreadyExists);
	
	if(TheCmdLineParams.iLogFileName.Length() > 0)
		{
		err = TheLogFile.Replace(TheFs, TheCmdLineParams.iLogFileName, EFileRead | EFileWrite);
		TEST2(err, KErrNone);
		LogConfig(TheLogFile, TheCmdLineParams);
		}
    }

TInt E32Main()
    {
	TheTest.Title();

	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);

	GetCmdLineParams(TheTest, _L("t_dbbench"), TheCmdLineParams);
	_LIT(KDbName, "c:\\dbms-tst\\t_bench.db");
	PrepareDbName(KDbName, TheCmdLineParams.iDriveName, TheDbFileName);
	TheTest.Printf(_L("==Database: %S\r\n"), &TheDbFileName); 

	__UHEAP_MARK;
	
	TestEnvInit();
	TRAPD(err, BenchTestL());
	TEST2(err, KErrNone);
	TestEnvDestroy();
	
	__UHEAP_MARKEND;

	User::Heap().Check();
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;
	
	return KErrNone;
    }
