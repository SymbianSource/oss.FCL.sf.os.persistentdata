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
// t_oomcenrep.cpp
// 
//

#include "t_cenrep_helper.h"
#include <e32test.h>
#include <f32file.h>
#include <utf.h>
#include "srvsess.h"
#include "sessmgr.h"
#include "srvres.h"
#include "srvreqs.h"
#include "cachemgr.h"
#include "clientrequest.h"
#include "install.h"
#include <bautils.h>

LOCAL_D RTest                TheTest (_L ("t_cenrep_pma_multirofs.exe"));

///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
LOCAL_C void Check(TInt aValue, TInt aExpected, TInt aLine, TBool aIsHexFormat=EFalse)
    {
    if(aValue != aExpected)
        {
        if(aIsHexFormat)
            {
            RDebug::Print(_L("*** Expected error: 0x%x, got: 0x%x\r\n"), aExpected, aValue);
            }
        else
            {
            RDebug::Print(_L("*** Expected error: %d, got: %d\r\n"), aExpected, aValue);
            }
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"), err );
            }
        TheTest(EFalse, aLine);
        }
    }

#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)
#define TESTHEX2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__, ETrue)

/////////////////////////////////////////////////////////////////////////////////////////
static void CloseTServerResources(TAny*)
    {
    TServerResources::Close();
    }

/////////////////////////////////////////////////////////////////////////////////////////
class CenrepSrvPmaMultiRofsTest :public CBase
{
public:
    CenrepSrvPmaMultiRofsTest();
    ~CenrepSrvPmaMultiRofsTest();
    static CenrepSrvPmaMultiRofsTest* NewL(TUid aReposUid);
    void DoHeapRepositoryContentCheckL();

private:
    void ConstructL(TUid aReposUid);
public:
    CServerRepository* iServerRepo;
    CSessionNotifier* iSessionNotif;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

CenrepSrvPmaMultiRofsTest::CenrepSrvPmaMultiRofsTest(){}

CenrepSrvPmaMultiRofsTest::~CenrepSrvPmaMultiRofsTest()
    {
    if (iServerRepo)
        {
        if (iSessionNotif)
            {
            iServerRepo->Close();
            }
        delete iServerRepo;
        }
    if (iSessionNotif)
        delete iSessionNotif;

    // Cache must be disabled here. Otherwise, if any idle repositories exists, they will
    // still be open (have their pointers in the iOpenRepositories list) and the list will leak.
    TServerResources::iCacheManager->DisableCache();
    TServerResources::iObserver->CloseiOpenRepositories();
    }

CenrepSrvPmaMultiRofsTest* CenrepSrvPmaMultiRofsTest::NewL(TUid aReposUid)
    {
    CenrepSrvPmaMultiRofsTest* self=new (ELeave)CenrepSrvPmaMultiRofsTest;
    CleanupStack::PushL(self);
    self->ConstructL(aReposUid);
    CleanupStack::Pop(self);
    return self;
    }

void CenrepSrvPmaMultiRofsTest::ConstructL(TUid aReposUid)
    {
    iServerRepo=new (ELeave)CServerRepository();
    iSessionNotif=new (ELeave) CSessionNotifier();

    iServerRepo->OpenL(aReposUid,*iSessionNotif);
    }

//Deletes the CServerRepository object properly
LOCAL_C void ReleaseRepository(TAny* aServerRepository)
    {
    CServerRepository* serverRepository = static_cast<CServerRepository*>(aServerRepository);

    TServerResources::iCacheManager->DisableCache();
    TServerResources::iObserver->CloseiOpenRepositories();
    delete serverRepository;
    TServerResources::iCacheManager->EnableCache();
    }

//Opening a repository and closing the repository
LOCAL_C void OpenCloseL(TUid aReposUid)
{
    CServerRepository* serverRepo=new (ELeave)CServerRepository();
    TCleanupItem cleanupItem(&ReleaseRepository, serverRepo);
    CleanupStack::PushL(cleanupItem);

    CSessionNotifier* sessNotif=new (ELeave)CSessionNotifier();
    CleanupStack::PushL(sessNotif);

    // test access to a valid repository
    serverRepo->OpenL(aReposUid,*sessNotif);
    serverRepo->Close();

    CleanupStack::PopAndDestroy(sessNotif);
    CleanupStack::PopAndDestroy(1);
}

// Type definition for pointer to function
// Used for functions that can't use CenrepSrvPmaMultiRofsTest::ConstructL
/**
Wrapper function to call all test functions
@param  testFuncL pointer to test function
@param  aTestDesc test function name
*/
LOCAL_C void DoMultiRofsReposTestL( TUid aReposUid, const TDesC& aTestDesc, TInt aExpectedResult)
    {
    TheTest.Next(aTestDesc);

    TInt err;
    __UHEAP_MARK;

    //Initializing the server resources
    TServerResources::InitialiseL ();

    TRAP(err, OpenCloseL(aReposUid));
    TEST2(err, aExpectedResult);

    //Freeing the server resources
    TServerResources::Close();

    __UHEAP_MARKEND;

    }

void CenrepSrvPmaMultiRofsTest::DoHeapRepositoryContentCheckL()
    {
    CServerRepository* srv=iServerRepo;
    //check setting and its meta
    TServerSetting* setting=NULL;

    setting=srv->GetSetting(1);
    TESTHEX2(setting->iKey,1);
    TESTHEX2(setting->iMeta,0x80000010);
    //points to global default policy here
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);

    setting=srv->GetSetting(2);
    TESTHEX2(setting->iKey,2);
    TESTHEX2(setting->iMeta,0xA0000010);
    //points to global default policy here
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);

    setting=srv->GetSetting(3);
    TESTHEX2(setting->iKey,3);
    TESTHEX2(setting->iMeta,0x800000FF);
    //points to global default policy here
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);

    setting=srv->GetSetting(4);
    TESTHEX2(setting->iKey,4);
    TESTHEX2(setting->iMeta,0x80000010);
    TESTHEX2(setting->iAccessPolicy->LowKey(),4);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),0);

    setting=srv->GetSetting(5);
    TESTHEX2(setting->iKey,5);
    TESTHEX2(setting->iMeta,0xC0000063);
    //points to global default policy here
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);

    setting=srv->GetSetting(6);
    TESTHEX2(setting->iKey,6);
    TESTHEX2(setting->iMeta,0x90000010);
    TESTHEX2(setting->iAccessPolicy->LowKey(),6);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),0);

    setting=srv->GetSetting(7);
    TESTHEX2(setting->iKey,7);
    TESTHEX2(setting->iMeta,0x80000010);
    TESTHEX2(setting->iAccessPolicy->LowKey(),7);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),0);

    setting=srv->GetSetting(9);
    TESTHEX2(setting->iKey,9);
    TESTHEX2(setting->iMeta,0x80000010);
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);

    setting=srv->GetSetting(10);
    TESTHEX2(setting->iKey,10);
    TESTHEX2(setting->iMeta,0x80000010);
    TESTHEX2(setting->iAccessPolicy->LowKey(),10);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),0);

    setting=srv->GetSetting(300);
    TESTHEX2(setting->iKey,300);
    TESTHEX2(setting->iMeta,0x900003E7);
    TESTHEX2(setting->iAccessPolicy->LowKey(),KUnspecifiedKey);
    TESTHEX2(setting->iAccessPolicy->HighKey(),1);
    TESTHEX2(setting->iAccessPolicy->KeyMask(),1);
    }

LOCAL_C void DoAdditionalCheckingL(TUid aReposUid)
    {
    TServerResources::InitialiseL ();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
    
    CenrepSrvPmaMultiRofsTest* test=CenrepSrvPmaMultiRofsTest::NewL(aReposUid);
    CleanupStack::PushL(test);
    test->DoHeapRepositoryContentCheckL();

    CleanupStack::PopAndDestroy(); //test
    CleanupStack::Pop(); //CloseTServerResources

    TServerResources::Close();
    }

/**
@SYMTestCaseID PDS-CENTRALREPOSITORY-UT-4124
@SYMTestCaseDesc Server side loading of a multiple ROFS PMA repository files
@SYMTestPriority High
@SYMTestActions  - Create the server class CServerRepository and used it to load the multi ROFS PMA keyspace.
                 - Test are repeated for the following keyspace combination such as TXT/TXT, CRE/TXT or TXT/CRE and CRE/CRE.
                 - Test will also verify the resulting merged settings which includes checking
                   individual metadata bits and security policy bits
                 - Test will also be repeated for opening PMA keyspace with non-PMA keyspace in different ROFS layers. 
                 
@SYMTestExpectedResults - Server loading of the multi rofs keyspace should not fail if the type of the keyspace is the same
                        - Server loading of the multi rofs keyspace should fail if the type of the keyspace is not the same
@SYMREQ REQ42876
*/
LOCAL_C void DoPMAMultiRofsTestL()
    {
    TheTest.Next (_L (" @SYMTestCaseID:PDS-CENTRALREPOSITORY-UT-4124 Centrep Server PMA MultiROFS Test"));

    const TUid KMultiRofsRepositoryUid0={0xF1000900};//TXT1(core) - CRE1(rofs1) - TXT1(rofs2) - CRE1(rofs3)
    const TUid KMultiRofsRepositoryUid1={0xF1000901};//TXT1(core) - TXT1(rofs1)
    const TUid KMultiRofsRepositoryUid2={0xF1000902};//CRE1(core) - CRE1(rofs1)
    const TUid KMultiRofsRepositoryUid3={0xF1000903};//TXT1(core) - CRE1(rofs1)
    const TUid KMultiRofsRepositoryUid4={0xF1000904};//CRE1(core) - TXT1(rofs1)
    const TUid KMultiRofsRepositoryUid5={0xF1000905};//TXT1(core) - TXT0(rofs1)
    const TUid KMultiRofsRepositoryUid6={0xF1000906};//TXT0(core) - TXT1(rofs1)
    const TUid KMultiRofsRepositoryUid7={0xF1000907};//CRE1(core) - CRE0(rofs1)
    const TUid KMultiRofsRepositoryUid8={0xF1000908};//CRE0(core) - CRE1(rofs1)
    const TUid KMultiRofsRepositoryUid9={0xF1000909};//CRE1(core) - TXT0(rofs1)
    const TUid KMultiRofsRepositoryUid10={0xF100090A};//CRE0(core) - TXT1(rofs1)
    const TUid KMultiRofsRepositoryUid11={0xF100090B};//TXT1(core) - CRE0(rofs1)
    const TUid KMultiRofsRepositoryUid12={0xF100090C};//TXT0(core) - CRE1(rofs1)
    const TUid KMultiRofsRepositoryUid13={0xF100090D};//TXT1(core) - CRE1(rofs1) - TXT1(rofs2) - CRE0(rofs3)
    // Note: TXT1 means the file is PMA repository with .txt format
    //       TXT0 means the file is non-PMA repository with .txt format
    //       CRE1 means the file is PMA repository with .cre format
    //       CRE0 means the file is non-PMA repository with .cre format
    
    //First Testuid=KMultiRofsRepositoryUid
    //Testing the OOM of multi rofs processing
    // 0xF1000900 to 0xF1000904 expect KErrNone as they are all PMA keyspaces of the same UID across 
    //   multiple ROFS layers.
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid0, _L("Open Close OOM Multi ROFS Test 0"),KErrNone);
    DoAdditionalCheckingL(KMultiRofsRepositoryUid0);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid1, _L("Open Close OOM Multi ROFS Test 1"),KErrNone);
    DoAdditionalCheckingL(KMultiRofsRepositoryUid1);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid2, _L("Open Close OOM Multi ROFS Test 2"),KErrNone);
    DoAdditionalCheckingL(KMultiRofsRepositoryUid2);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid3, _L("Open Close OOM Multi ROFS Test 3"),KErrNone);
    DoAdditionalCheckingL(KMultiRofsRepositoryUid3);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid4, _L("Open Close OOM Multi ROFS Test 4"),KErrNone);
    DoAdditionalCheckingL(KMultiRofsRepositoryUid4);
    
    // 0xF1000905 to 0xF100090D expect KErrCorrupt as they have at least mixed one PMA keyspace and one 
    //  Non-PMA keyspace of the same UID across multiple ROFS layers.
    // No need for additional checking of merged value because it should be corrupt.
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid5, _L("Open Close OOM Multi ROFS Test 5"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid6, _L("Open Close OOM Multi ROFS Test 6"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid7, _L("Open Close OOM Multi ROFS Test 7"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid8, _L("Open Close OOM Multi ROFS Test 8"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid9, _L("Open Close OOM Multi ROFS Test 9"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid10, _L("Open Close OOM Multi ROFS Test 10"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid11, _L("Open Close OOM Multi ROFS Test 11"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid12, _L("Open Close OOM Multi ROFS Test 12"),KErrCorrupt);
    DoMultiRofsReposTestL(KMultiRofsRepositoryUid13, _L("Open Close OOM Multi ROFS Test 13"),KErrCorrupt);
    }

LOCAL_C void MainL()
    {
    CActiveScheduler* scheduler = new(ELeave)CActiveScheduler;
    CActiveScheduler::Install(scheduler);

    DoPMAMultiRofsTestL();
    CleanupCDriveL();

    delete scheduler;
    }

TInt E32Main ()
    {
    TheTest.Title ();
    TheTest.Start(_L("PMA Multi ROFS CenrepServ Test"));
    
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
