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
#include <bautils.h>
#include "SqlSrvMain.h"
#include "SqlSrvStartup.h"
#include "SqlSrvUtil.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("t_sqlstartup test"));

static TInt TheProcessHandleCount = 0;
static TInt TheThreadHandleCount = 0;
static TInt TheAllocatedCellsCount = 0;

#ifdef _DEBUG
static const TInt KBurstRate = 20;
#endif

///////////////////////////////////////////////////////////////////////////////////////

void DeleteTestFiles()
	{
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DeleteTestFiles();
		RDebug::Print(_L("*** Expresssion evaluated to false\r\n"));
		TheTest(EFalse, aLine);
		}
	}
void Check(TInt aValue, TInt aExpected, TInt aLine)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void MarkHandles()
    {
    RThread().HandleCount(TheProcessHandleCount, TheThreadHandleCount);
    }

static void MarkAllocatedCells()
    {
    TheAllocatedCellsCount = User::CountAllocCells();
    }

static void CheckAllocatedCells()
    {
    TInt allocatedCellsCount = User::CountAllocCells();
    TEST2(allocatedCellsCount, TheAllocatedCellsCount);
    }

static void CheckHandles()
    {
    TInt endProcessHandleCount;
    TInt endThreadHandleCount;
    
    RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);

    TEST2(TheProcessHandleCount, endProcessHandleCount);
    TEST2(TheThreadHandleCount, endThreadHandleCount);
    }

static void OomPreStep(TInt
#ifdef _DEBUG        
    aFailingAllocationNo
#endif
                      )
    {
    MarkHandles();
    MarkAllocatedCells();
    __UHEAP_MARK;
    __UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, aFailingAllocationNo, KBurstRate);
    }

static void OomPostStep()
    {
    __UHEAP_RESET;
    __UHEAP_MARKEND;
    CheckAllocatedCells();
    CheckHandles();
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CreateAndDestroySqlServerL()
    {
    CSqlServer* server = CSqlServer::NewLC();
    CleanupStack::PopAndDestroy(server);
    }

static CSqlServer* CreateSqlServerL()
    {
    CSqlServer* server = CSqlServer::NewLC();
    CleanupStack::Pop(server);
    return server;
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4159
@SYMTestCaseDesc        SQL server startup OOM test
@SYMTestPriority        High
@SYMTestActions         Runs the SQL server startup code in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144096
*/  
void SqlServerStartupOomTest()
    {
    TInt err = KErrNoMemory;
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        TRAP(err, CreateAndDestroySqlServerL());
        OomPostStep();
        }
    if(err != KErrNoMemory)
        {
        TEST2(err, KErrNone);   
        }
    TheTest.Printf(_L("\r\n===OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4160
@SYMTestCaseDesc        CSqlServer::GetBackUpListL() OOM test
@SYMTestPriority        High
@SYMTestActions         Calls CSqlServer::GetBackUpListL() in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144096
*/  
void GetBackupListOomTest()
    {
    CSqlServer* server = NULL;
    TRAPD(err, server = CreateSqlServerL());
    TEST2(err, KErrNone);
  
    TInt fileCnt = 0;
    err = KErrNoMemory;
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        const TUid KDbUd = {0x98765432};
        RArray<TParse> files;
        TRAP(err, server->GetBackUpListL(KDbUd, files));
        fileCnt = files.Count(); 
        files.Close();
        OomPostStep();
        }
    
    delete server;
    
    if(err != KErrNoMemory)
        {
        TEST2(err, KErrNone);   
        }
    TheTest.Printf(_L("\r\n===OOM test succeeded at heap failure rate of %d ===\r\nFile count: %d\r\n"), failingAllocationNo, fileCnt);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4161
@SYMTestCaseDesc        SQL server startup file I/O error simulation test
@SYMTestPriority        High
@SYMTestActions         Runs the SQL server startup code in a file I/O error simulation loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144096
*/  
void SqlServerStartupFileIoErrorTest()
    {
    RFs fs;
    TInt err = fs.Connect();
    TEST2(err, KErrNone);
    
    for(TInt fsError=KErrNotFound;fsError>=KErrBadName;--fsError)
        {
        TheTest.Printf(_L("===Simulated error: %d\r\nIteration: "), fsError);
        err = KErrNotFound;
        TInt cnt=1;
        while(err<KErrNone)
            {
            TheTest.Printf(_L("%d "), cnt);
            (void)fs.SetErrorCondition(fsError, cnt);
            TRAP(err, CreateAndDestroySqlServerL());
            (void)fs.SetErrorCondition(KErrNone);
            if(err != KErrNone)
                {
                ++cnt;
                }
            }
        TEST2(err, KErrNone);
        TheTest.Printf(_L("\r\n===File I/O error simulation test succeeded on iteration %d===\r\n"), cnt);
        }

    fs.Close();
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4162
@SYMTestCaseDesc        CSqlServer::GetBackUpListL() file I/O error simulation test
@SYMTestPriority        High
@SYMTestActions         Calls CSqlServer::GetBackUpListL() in a file I/O error simulation loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144096
*/  
void GetBackupListFileIoErrorTest()
    {
    CSqlServer* server = NULL;
    TRAPD(err, server = CreateSqlServerL());
    TEST2(err, KErrNone);

    for(TInt fsError=KErrNotFound;fsError>=KErrBadName;--fsError)
        {
        TheTest.Printf(_L("===Simulated error: %d\r\nIteration: "), fsError);
        err = KErrNotFound;
        TInt fileCnt = 0;
        TInt cnt=1;
        while(err<KErrNone)
            {
            TheTest.Printf(_L("%d "), cnt);
            (void)server->Fs().SetErrorCondition(fsError, cnt);
            const TUid KDbUd = {0x98765432};
            RArray<TParse> files;
            TRAP(err, server->GetBackUpListL(KDbUd, files));
            fileCnt = files.Count(); 
            files.Close();
            (void)server->Fs().SetErrorCondition(KErrNone);
            if(err != KErrNone)
                {
                ++cnt;
                }
            }
        TEST2(err, KErrNone);
        TheTest.Printf(_L("\r\n===File I/O error simulation test succeeded on iteration %d===\r\nFile count: %d\r\n"), cnt, fileCnt);
        }
        
    delete server;
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4163
@SYMTestCaseDesc        Test for DEF144196: SQL, server code coverage can be improved
@SYMTestPriority        High
@SYMTestActions         Tests the UTF conversion functions implemented in SqlSrvUtil.cpp.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144196
*/  
void UtfConversionTest()
    {
    /////////       UTF16ToUTF8()       ///////////////////////
    _LIT(KStr16, "abcd");
    _LIT8(KStr8, "abcd");
    TBuf8<KMaxFileName + 1> bufout;
    TBool rc = UTF16ToUTF8(KStr16, bufout);
    TEST(rc);
    TEST(bufout == KStr8);
    //Test where the input buffer contains non-convertible characters
    TBuf<2> name2;
    name2.SetLength(2);
    name2[0] = TChar(0xD800); 
    name2[1] = TChar(0xFC00); 
    rc = UTF16ToUTF8(name2, bufout);
    TEST(!rc);
    /////////       UTF16ToUTF8Z()       ///////////////////////
    _LIT8(KStr8Z, "abcd\x0");
    rc = UTF16ToUTF8Z(KStr16, bufout);
    TEST(rc);
    TEST(bufout == KStr8Z);
    //Test where the input buffer contains non-convertible characters
    rc = UTF16ToUTF8Z(name2, bufout);
    TEST(!rc);
    /////////       UTF16ZToUTF8Z()       ///////////////////////
    _LIT(KStr16Z, "abcd\x0");
    rc = UTF16ZToUTF8Z(KStr16Z, bufout);
    TEST(rc);
    TEST(bufout == KStr8Z);
    //Test where the input buffer contains non-convertible characters
    TBuf<3> name3;
    name3.SetLength(3);
    name3[0] = TChar(0xD800); 
    name3[1] = TChar(0xFC00); 
    name3[2] = TChar(0x0000); 
    rc = UTF16ZToUTF8Z(name3, bufout);
    TEST(!rc);
    }

void DoTests()
	{
    CActiveScheduler* scheduler = new CActiveScheduler;
    TEST(scheduler != NULL);
    CActiveScheduler::Install(scheduler);
	
    TheTest.Start(_L(" @SYMTestCaseID:PDS-SQL-UT-4159 SQL server startup OOM test"));
    SqlServerStartupOomTest();

    TheTest.Next (_L(" @SYMTestCaseID:PDS-SQL-UT-4160 CSqlServer::GetBackUpListL() OOM test"));
    GetBackupListOomTest();

    TheTest.Next (_L(" @SYMTestCaseID:PDS-SQL-UT-4161 SQL server startup file I/O error simulation test"));
    SqlServerStartupFileIoErrorTest();
    
    TheTest.Next (_L(" @SYMTestCaseID:PDS-SQL-UT-4162 CSqlServer::GetBackUpListL() file I/O error simulation test"));
    GetBackupListFileIoErrorTest();

    TheTest.Next (_L(" @SYMTestCaseID:PDS-SQL-UT-4163 SQL server, UTF conversion test"));
    UtfConversionTest();

    delete scheduler;
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	
	__UHEAP_MARK;
	
	DoTests();
	DeleteTestFiles();

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
