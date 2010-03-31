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
#include "SqlSrvDatabase.h"
#include "SqlSrvFileData.h"

///////////////////////////////////////////////////////////////////////////////////////

RTest TheTest(_L("t_sqloom5 test"));
RFs TheFs;

_LIT(KTestDir, "c:\\test\\");
_LIT(KDbFile, "c:\\test\\t_sqloom5.db");
_LIT(KDbFile2, "c:[10281E17]t_sqloom5.db");
_LIT(KDbFile3, "c:[10281E17]t_sqloom5_2.db");
_LIT(KDbFile4, "c:[10281E17]t_sqloom5_3.db");

extern CSqlServer* TheServer;

static TInt TheProcessHandleCount = 0;
static TInt TheThreadHandleCount = 0;
static TInt TheAllocatedCellsCount = 0;

#ifdef _DEBUG
static const TInt KBurstRate = 20;
#endif

///////////////////////////////////////////////////////////////////////////////////////

void DestroyTestEnv()
	{
    (void)TheFs.Delete(KDbFile4);
    (void)TheFs.Delete(KDbFile3);
    (void)TheFs.Delete(KDbFile2);
    (void)TheFs.Delete(KDbFile);
	TheFs.Close();
	}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
void Check(TInt aValue, TInt aLine)
	{
	if(!aValue)
		{
		DestroyTestEnv();
		RDebug::Print(_L("*** Expresssion evaluated to false\r\n"));
		TheTest(EFalse, aLine);
		}
	}
void Check(TInt aValue, TInt aExpected, TInt aLine)
	{
	if(aValue != aExpected)
		{
		DestroyTestEnv();
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

void CreateTestEnv()
    {
    TInt err = TheFs.Connect();
    TEST2(err, KErrNone);

    err = TheFs.MkDir(KTestDir);
    TEST(err == KErrNone || err == KErrAlreadyExists);

    err = TheFs.CreatePrivatePath(EDriveC);
    TEST(err == KErrNone || err == KErrAlreadyExists);
    }

static CSqlServer* CreateSqlServerL()
    {
    CSqlServer* server = CSqlServer::NewLC();
    CleanupStack::Pop(server);
    return server;
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4167
@SYMTestCaseDesc        CSqlSrvDatabase::CreateL() OOM test.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::CreateL() in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577
*/  
void CreateDatabaseOomTest()
    {
    (void)TheFs.Delete(KDbFile);
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
    
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        TSqlSrvFileData& fdata = TheServer->FileData();
        RMessage2 msg;
        TRAP(err, fdata.SetL(msg, KDbFile().Length(), 0, (const TDesC8*)&KDbFile));
        if(err == KErrNone)
            {
            CSqlSrvDatabase* db = NULL;
            TRAP(err, db = CSqlSrvDatabase::CreateL(fdata));
            delete db;
            }
        OomPostStep();
        }

    delete TheServer;
    TheServer = NULL;
    
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::CreateL() OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4168
@SYMTestCaseDesc        CSqlSrvDatabase::OpenL() OOM test - non-secure database.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::OpenL() in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577
*/  
void OpenDatabaseOomTest()
    {
    //The database is created by the previous test: CreateDatabaseOomTest().
    
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
    
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        TSqlSrvFileData& fdata = TheServer->FileData();
        RMessage2 msg;
        TRAP(err, fdata.SetL(msg, KDbFile().Length(), 0, (const TDesC8*)&KDbFile));
        if(err == KErrNone)
            {
            CSqlSrvDatabase* db = NULL;
            TRAP(err, db = CSqlSrvDatabase::OpenL(fdata));
            delete db;
            }
        
        OomPostStep();
        }
    
    delete TheServer;
    TheServer = NULL;
    
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::OpenL() [non-secure db] OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4169
@SYMTestCaseDesc        CSqlSrvDatabase::CreateSecureL() OOM test.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::CreateSecureL() in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577
*/  
void CreateSecureDatabaseOomTest()
    {
    (void)TheFs.Delete(KDbFile2);
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
        
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        TSqlSrvFileData& fdata = TheServer->FileData();
        RMessage2 msg;
        TRAP(err, fdata.SetL(msg, KDbFile2().Length(), 0, (const TDesC8*)&KDbFile2));
        if(err == KErrNone)
            {
            TSecurityPolicy defaultPolicy(TSecurityPolicy::EAlwaysPass);
            CSqlSecurityPolicy* policy = NULL;
            TRAP(err, policy = CSqlSecurityPolicy::NewL(defaultPolicy));
            if(err == KErrNone)
                {
                CSqlSrvDatabase* db = NULL;
                TRAP(err, db = CSqlSrvDatabase::CreateSecureL(fdata, policy));
                delete db;
                }
            }
        OomPostStep();
        }

    delete TheServer;
    TheServer = NULL;
    
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::CreateSecureL() OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4170
@SYMTestCaseDesc        CSqlSrvDatabase::OpenL() OOM test - secure database.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::OpenL() in an OOM loop.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577
*/  
void OpenSecureDatabaseOomTest()
    {
    //The database is created by the previous test: CreateSecureDatabaseOomTest().
    
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
    
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        TSqlSrvFileData& fdata = TheServer->FileData();
        RMessage2 msg;
        TRAP(err, fdata.SetL(msg, KDbFile2().Length(), 0, (const TDesC8*)&KDbFile2));
        if(err == KErrNone)
            {
            CSqlSrvDatabase* db = NULL;
            TRAP(err, db = CSqlSrvDatabase::OpenL(fdata));
            delete db;
            }
        
        OomPostStep();
        }
    
    delete TheServer;
    TheServer = NULL;
    
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::OpenL() [secure db] OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/////////////////////////////////////////////////////////////

const TInt KDbConnCount = 7; 
#ifdef _DEBUG
const TInt KDbAttachedCount = 10;
#endif
TBuf<10> TheAttachedDbName;

//Declares KDbConnCount "CSqlSrvDatabase*" variables.  
#define TEST_DECLARE_DB_VARS() \
    CSqlSrvDatabase* db[KDbConnCount]; \
    Mem::FillZ(db, sizeof(db));

//Declares all KDbConnCount "CSqlSrvDatabase*" objects.  
#define TEST_DELETE_DB() \
    for(TInt i=0;i<KDbConnCount;++i) \
        { \
        delete db[i]; \
        }

//Creates CSqlSrvDatabase object where the database file is dbFile (the second macro parameter).
//The CSqlSrvDatabase pointer db[N - 1] (N is the first macro parameter) will be set to point to 
//the created CSqlSrvDatabase object. 
//N is the number of the database to be opened, between 1 and KDbConnCount.
#define TEST_OPEN_DB(N, dbFile) \
    __ASSERT_DEBUG(N > 0 && N <= KDbConnCount, User::Invariant()); \
    TRAP(err, fdata.SetL(msg, dbFile.Length(), 0, (const TDesC8*)&dbFile)); \
    if(err != KErrNone) \
        { \
        goto Cleanup; \
        } \
    db[N - 1] = NULL; \
    TRAP(err, db[N - 1] = CSqlSrvDatabase::OpenL(fdata)); \
    if(err != KErrNone) \
        { \
        goto Cleanup; \
        }

//Attaches the "dbFile" database to the database number specified by the first macro parameter.
//The attached database name is "A<M>", where M is the third macro parameter.
//N is the number of the database connection, between 1 and KDbConnCount.
//M is the number of the database to be attached, between 1 and KDbAttachedCount.
#define TEST_ATTACH_DB(N, dbFile, M) \
    __ASSERT_DEBUG(N > 0 && N <= KDbConnCount, User::Invariant()); \
    __ASSERT_DEBUG(M > 0 && M <= KDbAttachedCount, User::Invariant()); \
    TRAP(err, fdata.SetL(msg, dbFile.Length(), 0, (const TDesC8*)&dbFile)); \
    if(err != KErrNone) \
        { \
        goto Cleanup; \
        } \
    TheAttachedDbName.Copy(_L("A")); \
    TheAttachedDbName.AppendNum(M); \
    TRAP(err, db[N - 1]->AttachDbL(fdata, TheAttachedDbName)); \
    if(err != KErrNone) \
        { \
        goto Cleanup; \
        }

//Detaches database "A<M>" (M is the second macro parameter) from the database identified
//by the number N - the first macro parameter.
//N is the number of the database connection, between 1 and KDbConnCount.
//M is the number of the database to be detached, between 1 and KDbAttachedCount.
#define TEST_DETACH_DB(N, M) \
    __ASSERT_DEBUG(N > 0 && N <= KDbConnCount, User::Invariant()); \
    __ASSERT_DEBUG(M > 0 && M <= KDbAttachedCount, User::Invariant()); \
    if(db[N - 1]) \
        { \
        TheAttachedDbName.Copy(_L("A")); \
        TheAttachedDbName.AppendNum(M); \
        TRAP_IGNORE(db[N - 1]->DetachDbL(TheAttachedDbName)); \
        }

/////////////////////////////////////////////////////////////

void CreateSecureTestDb(const TDesC& aDbFile)
    {
    (void)TheFs.Delete(aDbFile);
    
    TSecurityPolicy defaultPolicy(TSecurityPolicy::EAlwaysPass);
    CSqlSecurityPolicy* policy = NULL;
    TRAPD(err, policy = CSqlSecurityPolicy::NewL(defaultPolicy));
    TEST2(err, KErrNone);
    
    TSqlSrvFileData& fdata = TheServer->FileData();
    RMessage2 msg;
    TRAP(err, fdata.SetL(msg, aDbFile.Length(), 0, (const TDesC8*)&aDbFile));
    
    CSqlSrvDatabase* db = NULL;
    TRAP(err, db = CSqlSrvDatabase::CreateSecureL(fdata, policy));
    delete db;
    TEST2(err, KErrNone);
    }
    
/**
@SYMTestCaseID          PDS-SQL-UT-4171
@SYMTestCaseDesc        CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM test.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::OpenL() and CSqlSrvDatabase::AttachDbL() in an OOM test.
                        The test is a complex one - 7 (KDbConnCount constant) databases opened 
                        (secure and non-secure), 10 (KDbAttachedCount constant) databases
                        attached (secure and non-secure).
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577, DEF144603
*/  
void OpenAttachDatabaseOomTest()
    {
    //Part of the databases are created by the previous tests.
        
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);

    CreateSecureTestDb(KDbFile3);
    CreateSecureTestDb(KDbFile4);
    
    //The following 2 declarations are used by the macros in the OOM loop
    RMessage2 msg;
    TSqlSrvFileData& fdata = TheServer->FileData();
    
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        //Declare KDbConnCount "CSqlSrvDatabase*" variables.
        TEST_DECLARE_DB_VARS();
        //Create CSqlSrvDatabase object, the database name is the second parameter of the macro. 
        //The related db[N - 1] variable will be set to point to the created object.
        TEST_OPEN_DB(1, KDbFile2());
        TEST_OPEN_DB(2, KDbFile2());
        TEST_OPEN_DB(3, KDbFile3());
        TEST_OPEN_DB(4, KDbFile3());
        TEST_OPEN_DB(5, KDbFile3());
        TEST_OPEN_DB(6, KDbFile());
        TEST_OPEN_DB(7, KDbFile());
        //Attach to the database with the number specified as first macro parameter, the database file, specified
        //as second macro parameter. The logical name of the attached database is "A<M>", where M is the third macro parameter.
        TEST_ATTACH_DB(1, KDbFile(), 1);
        TEST_ATTACH_DB(2, KDbFile(), 2);
        TEST_ATTACH_DB(2, KDbFile(), 3);
        TEST_ATTACH_DB(5, KDbFile4(), 4);
        TEST_ATTACH_DB(2, KDbFile4(), 5);
        TEST_ATTACH_DB(2, KDbFile4(), 6);
        TEST_ATTACH_DB(5, KDbFile4(), 7);
        TEST_ATTACH_DB(5, KDbFile4(), 8);
        TEST_ATTACH_DB(1, KDbFile4(), 9);
        TEST_ATTACH_DB(1, KDbFile(), 10);
Cleanup:
        __UHEAP_SETBURSTFAIL(RAllocator::ENone, 0, 0);
        //Detach from the database with the number specified as first macro parameter, the database         
        //with name "A<M>", where M is the second macro parameter.
        TEST_DETACH_DB(1, 9);
        TEST_DETACH_DB(1, 1);
        TEST_DETACH_DB(1, 10);
        TEST_DETACH_DB(2, 2);
        TEST_DETACH_DB(2, 3);
        TEST_DETACH_DB(2, 5);
        TEST_DETACH_DB(2, 6);
        TEST_DETACH_DB(5, 4);
        TEST_DETACH_DB(5, 7);
        TEST_DETACH_DB(5, 8);
        //Delete all created CSqlSrvDatabase objects.
        TEST_DELETE_DB();        
        
        OomPostStep();
        }
    
    delete TheServer;
    TheServer = NULL;
    
    (void)TheFs.Delete(KDbFile4);
    (void)TheFs.Delete(KDbFile3);
    
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM test succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4172
@SYMTestCaseDesc        CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM test.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::OpenL() and CSqlSrvDatabase::AttachDbL() in an OOM test.
                        Two secure databases are created and then, in an OOM loop, the test executes this sequence of
                        commands: open first database, attach the second database, detach the attached database,
                        close the first database. 
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577, PDEF44845
*/  
void OpenAttachDatabaseOomTest2()
    {
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
    
    CreateSecureTestDb(KDbFile3);
    CreateSecureTestDb(KDbFile4);
    
    TInt failingAllocationNo = 0;
    TheTest.Printf(_L("Iteration:\r\n"));
    
    RMessage2 msg;
    TSqlSrvFileData& fdata = TheServer->FileData();
    
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        TRAP(err, fdata.SetL(msg, KDbFile3().Length(), 0, (const TDesC8*)&KDbFile3));
        if(err == KErrNone)
            {
            CSqlSrvDatabase* db = NULL;
            TRAP(err, db = CSqlSrvDatabase::OpenL(fdata));
            if(err == KErrNone)
                {
                TRAP(err, fdata.SetL(msg, KDbFile4().Length(), 0, (const TDesC8*)&KDbFile4));
                if(err == KErrNone)
                    {
                    TRAP(err, db->AttachDbL(fdata, _L("db2")));
                    if(err == KErrNone)
                        {
                        TRAP(err, db->DetachDbL(_L("db2")));
                        }
                    }
                delete db;
                }
            }
        OomPostStep();
        }
    
    (void)TheFs.Delete(KDbFile4);
    (void)TheFs.Delete(KDbFile3);
    
    delete TheServer;
    TheServer = NULL;
     
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM test 2 succeeded at heap failure rate of %d ===\r\n"), failingAllocationNo);
    }

/**
@SYMTestCaseID          PDS-SQL-UT-4173
@SYMTestCaseDesc        CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::CreateSecureL() OOM test.
@SYMTestPriority        High
@SYMTestActions         The test runs CSqlSrvDatabase::OpenL() and CSqlSrvDatabase::CreateSecureL() in an OOM test.
                        The test creates a secure database then executes CSqlSrvDatabase::OpenL() in an OOM loop.
                        After that the database is deleted and the test executes CSqlSrvDatabase::CreateSecureL() in an OOM loop.
                        The purpose of the test is to check that the CSqlSrver maps are properly updated when
                        the database is closed.
@SYMTestExpectedResults Test must not fail
@SYMDEF                 DEF144577, PDEF44845
*/  
void OpenCreateDatabaseOomTest()
    {
    TheServer = NULL; 
    TRAPD(err, TheServer = CreateSqlServerL());
    TEST2(err, KErrNone);
    
    (void)TheFs.Delete(KDbFile2);
    CreateSecureTestDb(KDbFile2);
    
    TheTest.Printf(_L("Iteration:\r\n"));

    //Open the database
    TInt failingAllocationNo = 0;
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo);
        OomPreStep(failingAllocationNo);
        
        RMessage2 msg;
        TSqlSrvFileData& fdata = TheServer->FileData();
        TRAP(err, fdata.SetL(msg, KDbFile2().Length(), 0, (const TDesC8*)&KDbFile2));
        if(err == KErrNone)
            {
            CSqlSrvDatabase* db = NULL;
            TRAP(err, db = CSqlSrvDatabase::OpenL(fdata));
            if(err == KErrNone)
                {
                delete db;
                }
            else
                {
                TEST(!db);
                }
            }
        OomPostStep();
        }
    TEST2(err, KErrNone);   
    err = TheFs.Delete(KDbFile2);
    TEST2(err, KErrNone);   
    //Create the database
    TInt failingAllocationNo2 = 0;
    err = KErrNoMemory;
    while(err == KErrNoMemory)
        {
        TheTest.Printf(_L(" %d"), ++failingAllocationNo2);
        OomPreStep(failingAllocationNo2);
        RMessage2 msg;
        TSqlSrvFileData& fdata = TheServer->FileData();
        TRAP(err, fdata.SetL(msg, KDbFile2().Length(), 0, (const TDesC8*)&KDbFile2));
        if(err == KErrNone)
            {
            TSecurityPolicy defaultPolicy(TSecurityPolicy::EAlwaysPass);
            CSqlSecurityPolicy* policy = NULL;
            TRAP(err, policy = CSqlSecurityPolicy::NewL(defaultPolicy));
            if(err == KErrNone)
                {
                CSqlSrvDatabase* db = NULL;
                TRAP(err, db = CSqlSrvDatabase::CreateSecureL(fdata, policy));
                if(err == KErrNone)
                    {
                    delete db;
                    }
                else
                    {
                    TEST(!db);
                    }
                }
            }
        OomPostStep();
        }
    
    (void)TheFs.Delete(KDbFile2);
    
    delete TheServer;
    TheServer = NULL;
     
    TEST2(err, KErrNone);   
    TheTest.Printf(_L("\r\n===CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::CreateSecureL() OOM test succeeded at heap failure rate of %d ===\r\n"), 
                   failingAllocationNo + failingAllocationNo2);
    }

void DoTests()
	{
#ifndef _DEBUG
    TheTest.Start(_L("This test can be run only in debug mode!"));
#else	
    CActiveScheduler* scheduler = new CActiveScheduler;
    TEST(scheduler != NULL);
    CActiveScheduler::Install(scheduler);
	
    TheTest.Start(_L(" @SYMTestCaseID:PDS-SQL-UT-4167 CSqlSrvDatabase::CreateL() OOM unit test"));
    CreateDatabaseOomTest();

    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4168 CSqlSrvDatabase::OpenL() OOM unit test - non-secure database"));
    OpenDatabaseOomTest();

    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4169 CSqlSrvDatabase::CreateSecureL() OOM unit test"));
    CreateSecureDatabaseOomTest();

    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4170 CSqlSrvDatabase::OpenL() OOM unit test - secure database"));
    OpenSecureDatabaseOomTest();
    
    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4171 CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM unit test"));
    OpenAttachDatabaseOomTest();

    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4172 CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::AttachDbL() OOM unit test - 2"));
    OpenAttachDatabaseOomTest2();

    TheTest.Next(_L(" @SYMTestCaseID:PDS-SQL-UT-4173 CSqlSrvDatabase::OpenL() & CSqlSrvDatabase::CreateL() OOM unit test"));
    OpenCreateDatabaseOomTest();

    delete scheduler;
#endif //_DEBUG    
	}

TInt E32Main()
	{
	TheTest.Title();
	
	CTrapCleanup* tc = CTrapCleanup::New();
	TheTest(tc != NULL);
	
	__UHEAP_MARK;
	
	CreateTestEnv();
	DoTests();
	DestroyTestEnv();

	__UHEAP_MARKEND;
	
	TheTest.End();
	TheTest.Close();
	
	delete tc;

	User::Heap().Check();
	return KErrNone;
	}
