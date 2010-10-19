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

LOCAL_D RTest                   TheTest(_L("t_cenrep_pma_oom.exe"));

_LIT( KCentralRepositoryServerName, "Centralrepositorysrv");

_LIT(KInstallDirFile,           "c:\\private\\10202BE9\\persists\\installdir.bin");

_LIT(KInstallPMATxtFileSrc,     "z:\\private\\10202BE9\\f1000701.txi");
_LIT(KInstallPMATxtFileTgt,     "c:\\private\\10202BE9\\f1000701.txt");

_LIT(KInstallPMACreFileSrc,     "z:\\private\\10202BE9\\f1000700.cri");
_LIT(KInstallPMACreFileTgt,     "c:\\private\\10202BE9\\f1000700.cre");

//Test repositories Uid
const TUid KTestRepositoryUid={0xf1000701};

static TUid KCurrentTestUid;

//Burst rate for __UHEAP_SETBURSTFAIL
#ifdef _DEBUG
const TInt KBurstRate = 20;
#endif

///////////////////////////////////////////////////////////////////////////////////////
//Test macros and functions
LOCAL_C void Check(TInt aValue, TInt aLine)
    {
    if(!aValue)
        {
        TRAPD(err, CleanupCDriveL());
        if (err != KErrNone)
            {
            RDebug::Print( _L( "*** CleanupCDriveL also failed with error %d expecting KErrNone\r\n"), err );
            }
        TheTest(EFalse, aLine);
        }
    }
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
#define TEST(arg) ::Check((arg), __LINE__)
#define TEST2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__)
#define TESTHEX2(aValue, aExpected) ::Check(aValue, aExpected, __LINE__, ETrue)
#define TESTKErrNone(aValue) ::Check(aValue,0,__LINE__);

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
static void CloseTServerResources(TAny*)
    {
    TServerResources::Close();
    }

/////////////////////////////////////////////////////////////////////////////////////////
class CenrepSrvOOMTest :public CBase
{
public:
    void GetL();
    void FindL();
    void ResetL();
    void NotifyL();
    void SetL();
    void CreateL();
    void DeleteL();
    void MoveL();

    CenrepSrvOOMTest();
    ~CenrepSrvOOMTest();
    static CenrepSrvOOMTest* NewL();
#ifdef SYMBIAN_CENTREP_SUPPORT_MULTIROFS
    void DoHeapRepositoryContentCheckL();
#endif
private:
    void ConstructL();
public:
    CServerRepository* iServerRepo;
    CSessionNotifier* iSessionNotif;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CenrepSwiOOMTest :public CBase
{
public:
    void InstallTxtPmaL(TBool aIsSetup);
    void InstallCrePmaL(TBool aIsSetup);
    
    CenrepSwiOOMTest();
    ~CenrepSwiOOMTest();
    static CenrepSwiOOMTest* NewL();
private:
    void ConstructL();
    CCentRepSWIWatcher* iSwiWatcher;
public:
};
//////////////////////////////////////////////////////////////////////////////////////////////////

CenrepSrvOOMTest::CenrepSrvOOMTest(){}

CenrepSrvOOMTest::~CenrepSrvOOMTest()
    {
    if(iServerRepo)
        {
        if(iSessionNotif)
            {
            iServerRepo->Close();
            }
        delete iServerRepo;
        }
    if(iSessionNotif)
        {
        delete iSessionNotif;
        }

    // Cache must be disabled here. Otherwise, if any idle repositories exists, they will
    // still be open (have their pointers in the iOpenRepositories list) and the list will leak.
    TServerResources::iCacheManager->DisableCache();
    TServerResources::iObserver->CloseiOpenRepositories();
    }

CenrepSrvOOMTest* CenrepSrvOOMTest::NewL()
    {
    CenrepSrvOOMTest* self=new (ELeave)CenrepSrvOOMTest;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CenrepSrvOOMTest::ConstructL()
    {
    iServerRepo=new (ELeave)CServerRepository();
    iSessionNotif=new (ELeave) CSessionNotifier();

    iServerRepo->OpenL(KCurrentTestUid,*iSessionNotif);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////

CenrepSwiOOMTest::CenrepSwiOOMTest(){}

CenrepSwiOOMTest::~CenrepSwiOOMTest()
    {
    delete iSwiWatcher;
    }

CenrepSwiOOMTest* CenrepSwiOOMTest::NewL()
    {
    CenrepSwiOOMTest* self=new (ELeave)CenrepSwiOOMTest;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CenrepSwiOOMTest::ConstructL()
    {
    iSwiWatcher = CCentRepSWIWatcher::NewL(TServerResources::iFs);
    }

//Getting various information and security policy from the repository
//TInt                 CServerRepository::Get(TUint32 aId,T& aVal)
//TServerSetting*     CServerRepository::GetSetting(TUint32 aId)
void CenrepSrvOOMTest::GetL()
    {
    TInt err=KErrNone;
    TInt keyInt=0;
    TServerSetting* srvsetting;
    //----------Getting the TInt(1-15)-----------------------------
    for(TInt i=1;i<=15;i++)
        {
        err=iServerRepo->Get(i,keyInt);
        //Remember the policy check is done at the session level
        TESTKErrNone(err);
        TEST2(i,keyInt);
        srvsetting=iServerRepo->GetSetting(i);
        TEST(srvsetting->Key()==static_cast<TUint32>(i));
        }
    //---------Getting the TReal(16-19)---------------------------
    TReal keyValue;
    err=iServerRepo->Get(16,keyValue);
    TESTKErrNone(err);
    TEST(keyValue==10.1010);
    srvsetting=iServerRepo->GetSetting(16);
    TESTHEX2(srvsetting->Key(),16);

    err=iServerRepo->Get(19,keyValue);
    TESTKErrNone(err);
    TEST(keyValue==13.1313);

    //----------Getting the String(20-23)-----------------------
    TBuf8<50> keyString;
    _LIT(KFourteen,"fourteen");
    err=iServerRepo->Get(20,keyString);
    TESTKErrNone(err);
    //Even ascii(8 bytes) are stored as 16 bytes!!!
    TPtr16 str16((TUint16*) keyString.Ptr(),keyString.Length()/2,keyString.Length()/2);
    TEST(str16.Compare(KFourteen)==0);
    srvsetting=iServerRepo->GetSetting(20);
    TESTHEX2(srvsetting->Key(),20);

    //---------Getting the String8(0x79)------------------------
    TBuf8<50> keyString8;
    _LIT8(KString8,"string8");
    err=iServerRepo->Get(0x79,keyString8);
    TESTKErrNone(err);
    TEST(keyString8.Compare(KString8)==0);
    srvsetting=iServerRepo->GetSetting(0x79);
    TESTHEX2(srvsetting->Key(),0x79);

    //---------Getting the binary(0x82)-------------------------
    TBuf8<50> keyBinary;
    _LIT8(KBinary8,"\x12\x34\xAB\xCD");
    err=iServerRepo->Get(0x82,keyBinary);
    TESTKErrNone(err);
    //temporarily added to solve coverage problem in WINSCW
    TEST(keyBinary.Compare(KBinary8)==0);
    srvsetting=iServerRepo->GetSetting(0x82);
    TESTHEX2(srvsetting->Key(),0x82);

    //----------Getting individual policy-----------------------
    //Current Exe has caps AllFiles+WriteDeviceData+ReadDeviceData
    RThread currentThread;
    TSecurityPolicy secPolicy;
    //default policy
    secPolicy=iServerRepo->GetDefaultReadAccessPolicy();
    TEST(secPolicy.CheckPolicy(currentThread)==1);
    secPolicy=iServerRepo->GetDefaultWriteAccessPolicy();
    TEST(secPolicy.CheckPolicy(currentThread)==1);

    //check settings policies
    //0x2 int 2 1 cap_rd=CommDD cap_wr=WriteDeviceData
    //0x19 int 25 0 //defined in range policies with cap_rd=NetworkServices
    secPolicy=iServerRepo->GetReadAccessPolicy(2);
    TEST(secPolicy.CheckPolicy(currentThread)==0);
    secPolicy=iServerRepo->GetWriteAccessPolicy(2);
    TEST(secPolicy.CheckPolicy(currentThread)==1);
    secPolicy=iServerRepo->GetReadAccessPolicy(25);
    TEST(secPolicy.CheckPolicy(currentThread)==0);

    }

// now that write operations must be done in transactions, setting up this helper
// function to perform single Sets for the purpose of this test.
template<class T>
static TInt RepositorySingleSetL(CServerRepository& aRep, TUint32 aKey, const T& aValue)
    {
    aRep.StartTransaction(EConcurrentReadWriteTransaction);
    aRep.CleanupCancelTransactionPushL();
    aRep.TransactionSetL(aKey, aValue);
    CleanupStack::Pop();
    TUint32 keyInfo;
    return User::LeaveIfError(aRep.CommitTransaction(keyInfo));
    };

//Setting OOM testing
void CenrepSrvOOMTest::SetL()
    {
    TInt ret=KErrNone;
    TInt intValue=0;
    TReal realValue=0;

    //---------------SetL-----------------------------------
    //Setting an integer key
    ret = RepositorySingleSetL(*iServerRepo, 0x60, 600);
    TESTKErrNone(ret);
    ret=iServerRepo->Get(0x60,intValue);
    TESTHEX2(intValue, 600);

    //Setting a real key
    ret = RepositorySingleSetL(*iServerRepo, 0x66, 99.99);
    TESTKErrNone(ret);
    ret=iServerRepo->Get(0x66,realValue);
    TEST(realValue==99.99);

    //Setting a string key
    //Even ascii(8 bytes) are stored as 16 bytes!!!
    _LIT8(KString,"sixhundred");
    TBuf8<50> stringChangeValue=KString();
    ret = RepositorySingleSetL(*iServerRepo, 0x69, stringChangeValue);
    TESTKErrNone(ret);

    TBuf8<50> keyString;
    ret=iServerRepo->Get(0x69,keyString);
    TESTKErrNone(ret);
    TEST(keyString.Compare(KString)==0);
    }

// now that write operations must be done in transactions, setting up this helper
// function to perform single Creates for the purpose of this test.
template<class T>
static TInt RepositorySingleCreateL(CServerRepository& aRep, TUint32 aKey, const T& aValue, TUint32* aMeta)
    {
    aRep.StartTransaction(EConcurrentReadWriteTransaction);
    aRep.CleanupCancelTransactionPushL();
    aRep.TransactionCreateL(aKey, aValue, aMeta);
    CleanupStack::Pop();
    TUint32 keyInfo;
    return User::LeaveIfError(aRep.CommitTransaction(keyInfo));
    };

void CenrepSrvOOMTest::CreateL()
    {
    TInt ret=KErrNone;
    TInt intValue=0;
    TReal realValue=0;

    //--------------CreateL-----------------------------------
    //Creating an integer key
    TInt intRetValue;
    intValue=9000;
    RepositorySingleCreateL(*iServerRepo, 0x90, intValue, NULL);
    TESTKErrNone(ret);
    ret=iServerRepo->Get(0x90,intRetValue);
    TESTKErrNone(ret);
    TESTHEX2(intRetValue, intValue);

    //Creating a real key
    TReal realRetValue;
    realValue=33.3333;
    RepositorySingleCreateL(*iServerRepo, 0x92, realValue, NULL);
    TESTKErrNone(ret);
    ret=iServerRepo->Get(0x92,realRetValue);
    TESTKErrNone(ret);
    TEST(realValue==realRetValue);

    //Creating a string key
    _LIT8(KStringValue,"creatingkey");
    TBuf8<50> stringCreateValue=KStringValue();
    RepositorySingleCreateL(*iServerRepo, 0x93, stringCreateValue, NULL);
    TESTKErrNone(ret);
    }

// now that write operations must be done in transactions, setting up this helper
// function to perform single Creates for the purpose of this test.
static TInt RepositorySingleDeleteL(CServerRepository& aRep, TUint32 aKey)
    {
    aRep.StartTransaction(EConcurrentReadWriteTransaction);
    aRep.CleanupCancelTransactionPushL();
    aRep.TransactionDeleteL(aKey);
    CleanupStack::Pop();
    TUint32 keyInfo;
    return User::LeaveIfError(aRep.CommitTransaction(keyInfo));
    };

void CenrepSrvOOMTest::DeleteL()
    {
    TInt ret=KErrNone;
    //--------------Delete-----------------------------------
    //Find the settings 0x10A-0x10C to ensure it is still there
    RSettingPointerArray matchingArray;
    TUint32 partialId=0x100;
    TUint32 idMask=0xFFFFFFF0;
    ret=iServerRepo->FindSettings(partialId,idMask,matchingArray);
    if(ret==KErrNoMemory)
        {
        matchingArray.Close();
        User::LeaveNoMemory();
        }

    TESTKErrNone(ret);
    TEST2(matchingArray.Count(),3);
    matchingArray.Close();

    //Deleting settings 0x10A to 0x10C
    for(TInt i=0x10A;i<=0x10C;i++)
        {
        RepositorySingleDeleteL(*iServerRepo, i);
        TESTKErrNone(ret);
        }
    //After deleting try to find the persistent settings again
    ret=iServerRepo->FindSettings(partialId,idMask,matchingArray);
    if(ret==KErrNoMemory)
        {
        matchingArray.Close();
        User::LeaveNoMemory();
        }
    TESTKErrNone(ret);
    TEST2(matchingArray.Count(),0);
    matchingArray.Close();

    //-------------DeleteRange---------------------------------
    //Deleting settings 0x1 to 0xF
    TClientRequest dummyrequest;
    TUint32 errId=0;
    partialId=0;
    idMask=0xFFFFFFF0;
    ret=iServerRepo->FindSettings(partialId,idMask,matchingArray);
    if(ret==KErrNoMemory)
        {
        matchingArray.Close();
        User::LeaveNoMemory();
        }
    TESTKErrNone(ret);
    TEST2(matchingArray.Count(),15);
    matchingArray.Close();

    //Deleting settings using the DeleteRange
    dummyrequest.SetParam(0,partialId);
    dummyrequest.SetParam(1,idMask);
    dummyrequest.SetPolicyCheck(ETrue);

    // write operation must take place in a transaction
    iServerRepo->StartTransaction(EConcurrentReadWriteTransaction);
    iServerRepo->CleanupCancelTransactionPushL();
    iServerRepo->TransactionDeleteRangeL(dummyrequest,errId);
    CleanupStack::Pop();
    TUint32 keyInfo;
    User::LeaveIfError(iServerRepo->CommitTransaction(keyInfo));

    //Now try to find the key being deleted
    ret=iServerRepo->FindSettings(partialId,idMask,matchingArray);
    if(ret==KErrNoMemory)
        {
        matchingArray.Close();
        User::LeaveNoMemory();
        }
    TESTKErrNone(ret);
    TEST2(matchingArray.Count(),0);
    matchingArray.Close();
    }

//Setting existing key value then follow by commit
void CenrepSrvOOMTest::MoveL()
    {
    RSettingPointerArray matchingArray;
    TUint32 idMask=0xFFFFFFF0;

    TClientRequest dummyrequest;
    TUint32 errId=0;
    TUint32 sourcePartialId=0x110;
    TUint32 targetPartialId=0x120;
    idMask=0xFFFFFFF0;

    TKeyFilter srcKeyIdentifier = {sourcePartialId, idMask};
    TPckg<TKeyFilter> pSrcIdentifier(srcKeyIdentifier);
    TKeyFilter tgtKeyIdentifier = {targetPartialId, idMask};
    TPckg<TKeyFilter> pTgtIdentifier(tgtKeyIdentifier);

    //First check to ensure the target key before move does not exist
    User::LeaveIfError(iServerRepo->FindSettings(targetPartialId,idMask,matchingArray));
    TEST2(matchingArray.Count(),0);
    matchingArray.Close();

    //moving from 0x110(0x11B,0x11C,0x11E) to 0x120
    dummyrequest.SetParam(0, &pSrcIdentifier);
    dummyrequest.SetParam(1, &pTgtIdentifier);
    dummyrequest.SetPolicyCheck(ETrue);

    // write operation must take place in a transaction
    User::LeaveIfError(iServerRepo->StartTransaction(EConcurrentReadWriteTransaction));
    iServerRepo->CleanupCancelTransactionPushL();
    User::LeaveIfError(iServerRepo->TransactionMoveL(dummyrequest,errId));
    CleanupStack::Pop();
    TUint32 keyInfo;
    User::LeaveIfError(iServerRepo->CommitTransaction(keyInfo));

    //Now try to find the key being moved
    User::LeaveIfError(iServerRepo->FindSettings(targetPartialId,idMask,matchingArray));
    TEST2(matchingArray.Count(),3);
    matchingArray.Close();
    }

//Finding keys from the settings
//TInt FindSettings(TUint32 aPartialId,TUint32 aIdMask,RSettingPointerArray& aMatches)
//Guarantees the heap free in aMatches if this function fail
void CenrepSrvOOMTest::FindL()
    {
    TInt ret=KErrNone;
    RSettingPointerArray foundIdArray;
    TUint32 partialId=0;
    TUint32 idMask=0;
    //-----------Finding settings array using partial id & mask------

    //------------------Real type---------------------------------
    //0x42,0x44,0x45,0x48
    partialId=0x40;
    idMask=0xFFFFFFF0;
    User::LeaveIfError(iServerRepo->FindSettings(partialId,idMask,foundIdArray));
    TESTKErrNone(ret);
    TEST2(foundIdArray.Count(),4);
    foundIdArray.Close();

    //-----------------String type-------------------------------
    //0x51,0x54,0x5B
    partialId=0x50;
    idMask=0xFFFFFFF0;
    User::LeaveIfError(iServerRepo->FindSettings(partialId,idMask,foundIdArray));
    TESTKErrNone(ret);
    TEST2(foundIdArray.Count(),3);
    foundIdArray.Close();

    //--------------------Int type----------------------------------------------------
    partialId=0x30;
    idMask=0xFFFFFFF0;
    //This should return only 0x30,0x34,0x35,0x39,0x3B( 5 items)
    User::LeaveIfError(iServerRepo->FindSettings(partialId,idMask,foundIdArray));
    TESTKErrNone(ret);
    TEST2(foundIdArray.Count(),5);

    //----------------Find comparison using EEqual & ENotEqual------------------------
    TInt searchValue=100;
    TClientRequest dummyrequest;
    RArray<TUint32> idArray;
    //Set the policycheck to always pass
    dummyrequest.SetPolicyCheck(ETrue);
    //Comparison using Equal
    TRAP(ret,iServerRepo->FindCompareL(foundIdArray,searchValue,EEqual,idArray));
    if(ret==KErrNoMemory)
        {
        //do not need to reset idArray as it is done inside the function itself when it returns not KErrNone
        foundIdArray.Close();
        User::LeaveNoMemory();
        }
    TEST2(idArray.Count(),2);
    TEST((idArray[0]==0x30 && idArray[1]==0x34) || (idArray[0]==0x34 && idArray[1]==0x30));
    idArray.Close();
    //Comparison using ENotEqual
    TRAP(ret,iServerRepo->FindCompareL(foundIdArray,searchValue,ENotEqual,idArray));
    if(ret==KErrNoMemory)
        {
        //do not need to reset idArray as it is done inside the function itself when it returns not KErrNone
        foundIdArray.Close();
        User::LeaveNoMemory();
        }
    TEST2(idArray.Count(),3);
    idArray.Close();
    foundIdArray.Close();

    }

//Resetting settings
void CenrepSrvOOMTest::ResetL()
    {
    TInt ret=KErrNone;
    TInt retValue=0;
    TReal realValue=0;

    //-------------Single key reset----------------------------

    ret = RepositorySingleSetL(*iServerRepo, 1, 500);
    TESTKErrNone(ret);
    ret=iServerRepo->Get(1,retValue);
    TESTHEX2(retValue, 500);

    //Resetting individual settings
    ret=iServerRepo->ResetL(1);
    TEST2(ret, KErrNotSupported);

    //Check for value once being reset
    ret=iServerRepo->Get(1,retValue);
    TEST(retValue==500);

    //------------All keys reset------------------------------
    ret = RepositorySingleSetL(*iServerRepo, 17, 3.1343424);
    TESTKErrNone(ret);

    //Reset all settings from Rom
    ret = iServerRepo->ResetAllL();
    TEST2(ret, KErrNotSupported);

    //Check for value once all being reset

    ret=iServerRepo->Get(17,realValue);
    TEST(realValue==3.1343424);
    }

void CenrepSrvOOMTest::NotifyL()
    {
    TInt err=KErrNone;

    //addding individual requests
    for(TInt i=0;i<10;i++)
        {
        TClientRequest dummyRequest;
        User::LeaveIfError(iSessionNotif->AddRequest(i,dummyRequest));
        }

    //adding group requests
    for(TInt i=0;i<10;i++)
        {
        TClientRequest dummyRequest;
        TUint32 partialId=100*i;
        TUint32 idMask=0xFFFFFFF0;
        User::LeaveIfError(iSessionNotif->AddRequest(partialId,idMask,dummyRequest));
        }

    //cancelling individual requests
    User::LeaveIfError(iSessionNotif->CancelRequest(5));
    //Check to ensure that it has been deleted so calling cancel again will return KErrNotFound
    err=iSessionNotif->CancelRequest(5);
    TEST2(err,KErrNotFound);

    //cancelling group requests
    User::LeaveIfError(iSessionNotif->CancelRequest(500,0xFFFFFFF0));
    err=iSessionNotif->CancelRequest(500,0xFFFFFF0);
    TEST2(err,KErrNotFound);

    //Finally cancel ALL requests
    iSessionNotif->CancelAllRequests();

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
LOCAL_C void OpenCloseL(TBool /*aOOMMode*/)
{
    CServerRepository* serverRepo=new (ELeave)CServerRepository();
    TCleanupItem cleanupItem(&ReleaseRepository, serverRepo);
    CleanupStack::PushL(cleanupItem);

    CSessionNotifier* sessNotif=new (ELeave)CSessionNotifier();
    CleanupStack::PushL(sessNotif);

    //  test access to a valid repository
    serverRepo->OpenL(KCurrentTestUid,*sessNotif);
    serverRepo->Close();

    CleanupStack::PopAndDestroy(sessNotif);
    CleanupStack::PopAndDestroy(1);
}

LOCAL_C void DoMultiRofsReposTestL( TUid aReposUid, const TDesC& aTestDesc, TInt aExpectedResult)
    {
    
    TheTest.Next(aTestDesc);

    TInt err;
    TInt tryCount = 0;
    KCurrentTestUid = aReposUid;
    do
        {
        __UHEAP_MARK;

        //Initializing the server resources
        TServerResources::InitialiseL();
        CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
        
        // find out the number of open handles
        TInt startProcessHandleCount;
        TInt startThreadHandleCount;
        RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);

        __UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, ++tryCount, KBurstRate);
        
        TRAP(err, OpenCloseL(ETrue));
        if(err!=KErrNoMemory)
            {
            TEST2(err, aExpectedResult);
            }
        
        __UHEAP_RESET;
        
        // check that no handles have leaked
        TInt endProcessHandleCount;
        TInt endThreadHandleCount;
        RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);
        TEST2(startProcessHandleCount, endProcessHandleCount);
        TEST2(startThreadHandleCount, endThreadHandleCount);

        //Freeing the server resources
        CleanupStack::Pop(); //CloseTServerResources
        TServerResources::Close();

        __UHEAP_MARKEND;
        } while(err == KErrNoMemory);

    TEST2(err, aExpectedResult);
    TheTest.Printf(_L("- server succeeded at heap failure rate of %i\n"), tryCount);

    }

void CenrepSwiOOMTest::InstallTxtPmaL(TBool aIsSetup)
    {
    if(aIsSetup)
        {
        RFs fs;
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        CFileMan* fm = CFileMan::NewL(fs);
        CleanupStack::PushL(fm);

        // Copy a txt PMA keyspace into install dir & Reset read-only bit
        //  This keyspace also exists in ROM as PMA keyspace
        User::LeaveIfError(fm->Copy(KInstallPMATxtFileSrc, KInstallPMATxtFileTgt));
        User::LeaveIfError(fm->Attribs(KInstallPMATxtFileTgt,0,KEntryAttReadOnly,TTime(0)));
                
        CleanupStack::PopAndDestroy(2); // fs and fm
        }
    else
        {
        iSwiWatcher->HandleSWIEventL(ESASwisInstall | ESASwisStatusSuccess);
        }
    }

void CenrepSwiOOMTest::InstallCrePmaL(TBool aIsSetup)
    {
    if(aIsSetup)
        {
        RFs fs;
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        CFileMan* fm = CFileMan::NewL(fs);
        CleanupStack::PushL(fm);

        // Copy a cre PMA keyspace into install dir & Reset read-only bit
        User::LeaveIfError(fm->Copy(KInstallPMACreFileSrc, KInstallPMACreFileTgt));
        User::LeaveIfError(fm->Attribs(KInstallPMACreFileTgt,0,KEntryAttReadOnly,TTime(0)));

        CleanupStack::PopAndDestroy(2); // fs and fm
        }
    else
        {
        iSwiWatcher->HandleSWIEventL(ESASwisInstall | ESASwisStatusSuccess);
        }
    }

LOCAL_C void StartupInstallL(TBool aIsSetup)
{
    if(aIsSetup)
        {
        // Set up files for test
        RFs fs;
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        CFileMan* fm = CFileMan::NewL(fs);
        CleanupStack::PushL(fm);

        // Clean out files
        TInt err=fs.Delete(KInstallDirFile);
        if((err!=KErrNone)&&(err!=KErrNotFound))
            {
            User::Leave(err);
            }

        // Cause directory listing with no files to be written
        CCentRepSWIWatcher* swiWatcher = CCentRepSWIWatcher::NewL(TServerResources::iFs);
        delete swiWatcher;

        // Copy a txt PMA keyspace into install dir & Reset read-only bit
        User::LeaveIfError(fm->Copy(KInstallPMATxtFileSrc, KInstallPMATxtFileTgt));
        User::LeaveIfError(fm->Attribs(KInstallPMATxtFileTgt,0,KEntryAttReadOnly,TTime(0)));
        
        // Copy a cre PMA keyspace into install dir & Reset read-only bit
        User::LeaveIfError(fm->Copy(KInstallPMACreFileSrc, KInstallPMACreFileTgt));
        User::LeaveIfError(fm->Attribs(KInstallPMACreFileTgt,0,KEntryAttReadOnly,TTime(0)));
        

        CleanupStack::PopAndDestroy(2); // fs and fm
        }
    else
        {
        CCentRepSWIWatcher* swiWatcher = CCentRepSWIWatcher::NewL(TServerResources::iFs);
        delete swiWatcher;
        }
}

// Type definition for pointer to function
// Used for functions that can't use CenrepSrvOOMTest::ConstructL
typedef void (*FuncPtrL) (TBool);
/**
Wrapper function to call all OOM test functions
@param        testFuncL pointer to OOM test function
@param        aTestDesc test function name
*/
LOCAL_C void DoOOMNoServReposL( FuncPtrL atestFuncL, const TDesC& aTestDesc, TBool aOOMMode)
    {
    TheTest.Next(aTestDesc);

    TInt err;
    TInt tryCount = 0;
    do
        {
        __UHEAP_MARK;

        //Initializing the server resources
        TServerResources::InitialiseL();
        CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
        
        // find out the number of open handles
        TInt startProcessHandleCount;
        TInt startThreadHandleCount;
        RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);

        (*atestFuncL)(ETrue);

        if(aOOMMode)
            {
            __UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, ++tryCount, KBurstRate);
            }

        TRAP(err, (*atestFuncL)(EFalse));
        if(err!=KErrNoMemory)
            {
            TESTKErrNone(err);
            }

        if(aOOMMode)
            {
            __UHEAP_RESET;
            }

        // check that no handles have leaked
        TInt endProcessHandleCount;
        TInt endThreadHandleCount;
        RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);
        TEST2(startProcessHandleCount, endProcessHandleCount);
        TEST2(startThreadHandleCount, endThreadHandleCount);

        //Freeing the server resources
        CleanupStack::Pop(); //CloseTServerResources
        TServerResources::Close();

        __UHEAP_MARKEND;
        } while(err == KErrNoMemory);

     TESTKErrNone(err);
     if(aOOMMode)
        {
        TheTest.Printf(_L("- server succeeded at heap failure rate of %i\n"), tryCount);
        }
    }

// Type definition for pointer to member function.
// Used in calling the CRegistryDataTest member function for testing.
typedef void (CenrepSrvOOMTest::*ClassFuncPtrL) (void);
/**
Wrapper function to call all OOM test functions
@param        testFuncL pointer to OOM test function
@param        aTestDesc test function name
@param        aOOMMode to enable/disable the OOM environment
*/
LOCAL_C void DoOOMTestL(ClassFuncPtrL testFuncL, const TDesC& aTestDesc,TBool aOOMMode)
    {
    TheTest.Next(aTestDesc);

    TInt err=KErrNone;
    TInt tryCount = 0;
    do
        {
        __UHEAP_MARK;
        //Clear any files in the persist directory
        CleanupCDriveL();
                
        //Initializing the server resources
        TServerResources::InitialiseL();
        CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));

        CenrepSrvOOMTest* theTest=CenrepSrvOOMTest::NewL();
        CleanupStack::PushL(theTest);

        TInt startProcessHandleCount;
        TInt startThreadHandleCount;
        RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);

        if(aOOMMode)
            {
            __UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, ++tryCount, KBurstRate);
            }
        TRAP(err, (theTest->*testFuncL)());

        if(aOOMMode)
            {
            __UHEAP_RESET;
            }

        if(err!=KErrNoMemory)
            {
            TESTKErrNone(err);
            }

        CleanupStack::PopAndDestroy(theTest);

        // check that no handles have leaked
        TInt endProcessHandleCount;
        TInt endThreadHandleCount;
        RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);

        TEST2(startProcessHandleCount, endProcessHandleCount);
        TEST2(startThreadHandleCount, endThreadHandleCount);

        //Freeing the server resources
        CleanupStack::Pop(); //CleanupTServerResources
        TServerResources::Close();
        __UHEAP_MARKEND;
        } while(err == KErrNoMemory);

     TESTKErrNone(err);
     if(aOOMMode)
         {
         TheTest.Printf(_L("- server succeeded at heap failure rate of %i\n"), tryCount);
         }
    }

// Type definition for pointer to member function.
// Used in calling the CRegistryDataTest member function for testing.
typedef void (CenrepSwiOOMTest::*ClassSwiFuncPtrL) (TBool);
/**
Wrapper function to call all OOM test functions
@param        testFuncL pointer to OOM test function
@param        aTestDesc test function name
@param        aOOMMode to enable/disable the OOM environment
*/
LOCAL_C void DoOOMSwiTestL(ClassSwiFuncPtrL aTestFuncL, const TDesC& aTestDesc,TBool aOOMMode, TInt aExpectedError)
    {
    TheTest.Next(aTestDesc);

    TInt err=KErrNone;
    TInt tryCount = 0;
    do
        {
        __UHEAP_MARK;
        
        CleanupCDriveL();
        //Initializing the server resources
        TServerResources::InitialiseL();
        CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
        
        TInt startProcessHandleCount;
        TInt startThreadHandleCount;
        RThread().HandleCount(startProcessHandleCount, startThreadHandleCount);

        CenrepSwiOOMTest* theTest=CenrepSwiOOMTest::NewL();
        CleanupStack::PushL(theTest);

        // Set up test
        (theTest->*aTestFuncL)(ETrue);

        if(aOOMMode)
            {
            __UHEAP_SETBURSTFAIL(RAllocator::EBurstFailNext, ++tryCount, KBurstRate);
            }
        RDebug::Print(_L("tryCount = %d"),tryCount);
        
        TRAP(err, (theTest->*aTestFuncL)(EFalse));

        if(aOOMMode)
            {
            __UHEAP_RESET;
            }

        if(err!=KErrNoMemory)
            {
            TEST2(err, aExpectedError);
            }

        CleanupStack::PopAndDestroy(theTest);

        // check that no handles have leaked
        TInt endProcessHandleCount;
        TInt endThreadHandleCount;
        RThread().HandleCount(endProcessHandleCount, endThreadHandleCount);

        TEST2(startProcessHandleCount, endProcessHandleCount);
        TEST2(startThreadHandleCount, endThreadHandleCount);

        //Freeing the server resources
        CleanupStack::Pop(); //CleanupTServerResources
        TServerResources::Close();
        __UHEAP_MARKEND;
        } while(err == KErrNoMemory);

    TEST2(err, aExpectedError);
     if(aOOMMode)
        {
        TheTest.Printf(_L("- server succeeded at heap failure rate of %i\n"), tryCount);
        }
    }

#ifdef SYMBIAN_CENTREP_SUPPORT_MULTIROFS
void CenrepSrvOOMTest::DoHeapRepositoryContentCheckL()
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
    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));
    
    KCurrentTestUid = aReposUid;
    CenrepSrvOOMTest* test=CenrepSrvOOMTest::NewL();
    CleanupStack::PushL(test);
    test->DoHeapRepositoryContentCheckL();

    CleanupStack::PopAndDestroy();
    CleanupStack::Pop(); //CloseTServerResources

    TServerResources::Close();
    }

/**
@SYMTestCaseID PDS-CENTRALREPOSITORY-UT-4126
@SYMTestCaseDesc Verifying that CRE generated will have the latest CRE version which is currently 3
@SYMTestPriority High
@SYMTestActions  Validating that CRE files generated with post REQ42876 code will always contain at least version 3.
                 The unit test will load a txt repository and then modify some settings so that it gets persisted
                 in the persists directory. The test then read the cre files to verify that the version persisted
                 is the latest which is 3.
@SYMTestExpectedResults The correct file version is returned.
@SYMREQ REQ42876
*/
LOCAL_C void DoPersistedVersionCheckingL()
    {
    TheTest.Next(_L(" @SYMTestCaseID:PDS-CENTRALREPOSITORY-UT-4126 Verifying CRE generated will always be version 3 "));
    TServerResources::InitialiseL();
    CleanupStack::PushL(TCleanupItem(CloseTServerResources, 0));

    const TUid KTestUid={0xF1000702};
    KCurrentTestUid=KTestUid;

    CenrepSrvOOMTest* test=CenrepSrvOOMTest::NewL();
    CleanupStack::PushL(test);

    //persist immediately
    test->iServerRepo->CommitChangesL(EPma);

    //now check the version of the CRE file
    CHeapRepository* heap=CHeapRepository::NewL(KTestUid);
    CleanupStack::PushL(heap);
    TUint8 creVersion;
    heap->CreateRepositoryFromCreFileL(TServerResources::iFs,_L("c:\\private\\10202be9\\persists\\protected\\f1000702.cre"),creVersion);
#ifdef SYMBIAN_INCLUDE_APP_CENTRIC
    TEST2(creVersion,KPersistFormatSupportsPma);
#else    
    TEST2(creVersion,KPersistFormatSupportsIndMetaIndicator);
#endif
    CleanupStack::PopAndDestroy(2,test);
    CleanupStack::Pop(); //CloseTServerResources
    TServerResources::Close();
    }

/**
@SYMTestCaseID PDS-CENTRALREPOSITORY-UT-4127
@SYMTestCaseDesc Server side OOM loading of a multiple ROFS PMA repository files
@SYMTestPriority High
@SYMTestActions  - Create the server class CServerRepository and use it to load the multi ROFS PMA keyspace.
                 - Test are repeated for the following keyspace combination such as TXT/TXT, CRE/TXT or TXT/CRE and CRE/CRE.
                 - Test will also verify the resulting merged settings which includes checking
                   individual metadata bits and security policy bits
                 - Test will also be repeated for opening PMA keyspace with non-PMA keyspace in different ROFS layers.
@SYMTestExpectedResults - Server loading of the multi rofs keyspace should not fail and leak memory under OOM condition.
@SYMREQ REQ42876
*/
LOCAL_C void DoOOMMultiRofsTestL()
    {
    TheTest.Next(_L(" @SYMTestCaseID:PDS-CENTRALREPOSITORY-UT-4127 Centrep Server PMA MultiROFS OOM Test "));
    
    const TUid KMultiRofsRepositoryUid0={0xF1000900};
    const TUid KMultiRofsRepositoryUid1={0xF1000901};
    const TUid KMultiRofsRepositoryUid2={0xF1000902};
    const TUid KMultiRofsRepositoryUid3={0xF1000903};
    const TUid KMultiRofsRepositoryUid4={0xF1000904};
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
    
    //Testing the OOM of multi rofs processing
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
#endif

/**
@SYMTestCaseID PDS-CENTRALREPOSITORY-UT-4125
@SYMTestCaseDesc Server side OOM test on Centrep functionalities
@SYMTestPriority High
@SYMTestActions  - Call Get, Set, Create, Delete, Move, Find, Notify, Reset on a repository.
                 - Simulate SWI install and upgrade install on a repository.
                 - Do the above steps on again under OOM condition.
                 - Open and Close repository under OOM condition.
                 - Verify the resulting merged settings which includes checking
                   individual metadata bits and security policy bits
@SYMTestExpectedResults - Server loading of the multi rofs keyspace should not fail and leak memory under OOM condition.
@SYMREQ REQ42876
*/
LOCAL_C void DoOOMTestsL()
    {
    TheTest.Next(_L(" @SYMTestCaseID:PDS-CENTRALREPOSITORY-UT-4125 Starting CENREPSRV OOM Test "));
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL(fs);
    CFileMan* fm = CFileMan::NewL(fs);
    CleanupStack::PushL(fm);

    //Clear any files in the persist directory
    CleanupCDriveL();

    //First Testuid=KTestRepositoryUid
    KCurrentTestUid=KTestRepositoryUid;

    DoOOMTestL(&CenrepSrvOOMTest::GetL,_L("Get Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::FindL,_L("FindL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::NotifyL,_L("NotifyL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::ResetL,_L("ResetL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::SetL,_L("SetL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::CreateL,_L("CreateL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::DeleteL,_L("DeleteL Basic Test"),EFalse);
    DoOOMTestL(&CenrepSrvOOMTest::MoveL,_L("MoveL Basic Test"),EFalse);

    // Simulate response to SWI rom-upgrade and downgrade events
    DoOOMSwiTestL(&CenrepSwiOOMTest::InstallTxtPmaL,_L("SwiUpgradeROMRev1L Basic Test"),EFalse, KErrNotSupported);

    // Simulate response to SWI new rep install/uninstall event events
    DoOOMSwiTestL(&CenrepSwiOOMTest::InstallCrePmaL,_L("SwiInstallL Basic Test"),EFalse, KErrNotSupported);
    
    // Simulate SWI events before server startup
    DoOOMNoServReposL(&StartupInstallL, _L("Startup Upgrade Basic Test"), EFalse);

    //OOM Test aOOMMode=ETrue
    DoOOMNoServReposL(&OpenCloseL, _L("Open Close OOM Test"),ETrue);
    
    //Clear any files in the persist directory
    CleanupCDriveL();
    TInt r = KillProcess(KCentralRepositoryServerName);
    TEST2(r,KErrNone);
    
    DoOOMTestL(&CenrepSrvOOMTest::GetL,_L("Get OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::FindL,_L("FindL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::NotifyL,_L("NotifyL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::ResetL,_L("ResetL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::SetL,_L("SetL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::CreateL,_L("CreateL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::DeleteL,_L("DeleteL OOM Test"),ETrue);
    DoOOMTestL(&CenrepSrvOOMTest::MoveL,_L("MoveL OOM Test"),ETrue);

    //Clear any files in the persist directory
    CleanupCDriveL();

    DoOOMSwiTestL(&CenrepSwiOOMTest::InstallTxtPmaL,_L("SwiUpgradeROMRev1L OOM Test"),ETrue, KErrNotSupported);

    DoOOMSwiTestL(&CenrepSwiOOMTest::InstallCrePmaL,_L("SwiInstallL OOM Test"),ETrue, KErrNotSupported);
    
    DoOOMNoServReposL(&StartupInstallL, _L("Startup Upgrade OOM Test"), ETrue);

    DoPersistedVersionCheckingL();
#ifdef SYMBIAN_CENTREP_SUPPORT_MULTIROFS
    DoOOMMultiRofsTestL();
#endif
    
    CleanupCDriveL();

    CleanupStack::PopAndDestroy(2);    // fs and fm
    }

LOCAL_C void MainL()
    {
    CActiveScheduler* scheduler = new(ELeave)CActiveScheduler;
    CActiveScheduler::Install(scheduler);

    DoOOMTestsL();
    CleanupCDriveL();

    delete scheduler;
    }

TInt E32Main()
    {
    TheTest.Title ();
    TheTest.Start(_L("PMA OOM Cenrepserv Test"));
    
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
