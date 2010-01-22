// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "TE_DefectTestStep.h"
#include "TE_PerfTestUtilities.h"
#include <centralrepository.h>
#include "t_cenrep_helper.h"
#include "srvdefs.h"
#include "srvreqs.h"
#include <bautils.h>
#include "cachemgr.h"

#ifdef __CENTREP_SERVER_CACHETEST__

const TUid KUidLargeRepository          = { 0xCCCCCC01 };
const TUid KUidRep1						= { 0x00000100 };
const TUid KUidLargeCreRepository       = { 0xCCCCCC02 };

const TUint32 KNewInt = 1000;
const TInt KIntValue = 1234;

const TInt KNumOfAttempts = 10;

_LIT(KZRep1File, 				"z:\\private\\10202BE9\\00000100.txt");
_LIT(KCRep1TxtFile,				"c:\\private\\10202BE9\\persists\\00000100.txt");
_LIT(KCRep1CreFile,				"c:\\private\\10202BE9\\persists\\00000100.cre");
#endif

CPerfTestDefectStep057491::CPerfTestDefectStep057491()
	{
	SetTestStepName(KPerfTestDefectStepDEF057491);
	}
	
TVerdict CPerfTestDefectStep057491::doTestStepL()
	{
#ifndef __CENTREP_SERVER_CACHETEST__
	return EPass;
#else	
	SetTestStepResult(EFail);
	
	CRepository* rep;
	TInt64 totalRomMs=0;
	TInt64 totalCreMs=0;
	TInt64 totalRepWriteMs=0;
	
   	TUint32 startFastCounter;	
   	TUint64 fastCounterromOpenMicroseconds;
   	TUint64 fastCounterrepWriteMicroseconds;
   	TUint64 fastCountercreOpenMicroseconds;
	
	// Flush and disable cache for this test
	TInt r;
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TEST(r==KErrNone);

	// Time opening and writing repository
	for(TInt i=0; i<KNumOfAttempts;i++)
		{
		// Get start time for ROM file
	    startFastCounter = User::FastCounter();
		User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeRepository));
		
		// Get finish time for ROM file
		fastCounterromOpenMicroseconds = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);
		totalRomMs+=fastCounterromOpenMicroseconds;
		Logger().WriteFormat(_L("Time to open large repos from ROM      file = %lu microseconds\n"), fastCounterromOpenMicroseconds);
		
	    startFastCounter = User::FastCounter();
		TInt r = rep->Create(KNewInt, KIntValue);
		fastCounterrepWriteMicroseconds = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);
		totalRepWriteMs+=fastCounterrepWriteMicroseconds;
		Logger().WriteFormat(_L("Time to write large repos              file = %lu microseconds\n"), fastCounterrepWriteMicroseconds);

		TEST(r==KErrNone);
		
		// Close repository
		CleanupStack::PopAndDestroy(rep);
		
		// Get start time and ticks for .cre file
	    startFastCounter = User::FastCounter();
		
		// Open repository 
		User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeRepository));
		
		// Get finish time and ticks for .cre file
		fastCountercreOpenMicroseconds  = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);
		totalCreMs+=fastCountercreOpenMicroseconds;
		Logger().WriteFormat(_L("Time to open large repos from persists file = %lu microseconds\n"), fastCountercreOpenMicroseconds);

#if !defined(__WINS__) && !defined(__WINSCW__)
		TEST(fastCounterromOpenMicroseconds>fastCountercreOpenMicroseconds);
#endif		
		const TUint32 	KIntKey1 		= 8847616;
		const TInt 		KIntKey1Value 	= 1;
		const TUint32 	KIntKey2 		= 139068928;
		const TInt 		KIntKey2Value 	= 30;
		const TUint32 	KIntKey3 		= 2373975808UL;
		const TInt 		KIntKey3Value 	= -1920991488;
		
		// Test 3 random int key values from repository
		TInt keyValue;
		r=rep->Get(KIntKey1, keyValue);
		TEST(r == KErrNone);
		TEST(keyValue == KIntKey1Value);

		r=rep->Get(KIntKey2, keyValue);
		TEST(r == KErrNone);
		TEST(keyValue == KIntKey2Value);
		
		r=rep->Get(KIntKey3, keyValue);
		TEST(r == KErrNone);
		TEST(keyValue == KIntKey3Value);
		
		// Close repository
		CleanupStack::PopAndDestroy(rep);

		CleanupFileFromCDriveL(KUidLargeRepository);
		}
	
	Logger().WriteFormat(_L("Average time to open large repos from ROM      file = %lu ms\n"), totalRomMs/KNumOfAttempts);
	Logger().WriteFormat(_L("Average time to open large repos from persists file = %lu ms\n"), totalCreMs/KNumOfAttempts);		
	Logger().WriteFormat(_L("Average time to write large repos to  persists file = %lu ms\n"), totalRepWriteMs/KNumOfAttempts);		

	RFs fs;			
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
	CFileMan* fm = CFileMan::NewL(fs);
	CleanupStack::PushL(fm);
	// Copy a repository txt file to the C:
	User::LeaveIfError(fm->Copy(KZRep1File, KCPersistsDir));
	// Reset read-only bit
	User::LeaveIfError(fm->Attribs(KCRep1TxtFile,0,KEntryAttReadOnly,TTime(0)));
	// Check it exists
	TEST( BaflUtils::FileExists (fs, KCRep1TxtFile));	
	
	// Open repository
	User::LeaveIfNull(rep = CRepository::NewLC(KUidRep1));
	
	// Create a setting
	r = rep->Create(KNewInt, KIntValue);
	TEST(r == KErrNone);
	
	// Check persists file exists
	TEST(BaflUtils::FileExists (fs, KCRep1CreFile));	

	// Check that .txt file is gone	
	TEST(BaflUtils::FileExists (fs, KCRep1TxtFile) == EFalse);	

	// Close the repository
	CleanupStack::PopAndDestroy(rep);
	
	CleanupStack::PopAndDestroy(2);		// fm, fs

	CleanupFileFromCDriveL(KUidRep1);

	// Enable cache
	r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TEST(r==KErrNone);

	SetTestStepResult(EPass);
	
	return TestStepResult();
#endif
	}

CPerfTestDefectStep059633::CPerfTestDefectStep059633()
	{
	SetTestStepName(KPerfTestDefectStepDEF059633);
	}

TVerdict CPerfTestDefectStep059633::doTestStepL()
	{
#ifndef __CENTREP_SERVER_CACHETEST__
	return EPass;
#else	
	SetTestStepResult(EFail);

	// Flush and disable cache for this test
	TInt r;
	r = SetGetParameters(TIpcArgs(EDisableCache));
	TEST(r==KErrNone);

	CleanupFileFromCDriveL(KUidLargeRepository);
	CleanupFileFromCDriveL(KUidLargeCreRepository);

   	CRepository* rep; 
   	
   	TUint32 startFastCounter;
   	TUint64 fastCountertimeToOpenLargeIniRep;
   	TUint64 fastCountertimeToOpenLargeCreRep;
   	TUint64 fastCountertimeToOpenXLargeInis;
   	TUint64 fastCountertimeToOpenXLargeCres;
    
    //=========================================================================================
   	//Single .ini - opening
    startFastCounter = User::FastCounter();
	User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeRepository));
	fastCountertimeToOpenLargeIniRep = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);
	Logger().WriteFormat(_L("Time to open text based rep from ROM = %lu microseconds\n\n"), fastCountertimeToOpenLargeIniRep);
	CleanupStack::PopAndDestroy(rep);
	
    //=========================================================================================
	//KNumOfAttempts .ini
	
	startFastCounter = User::FastCounter();	
	for (TInt i=0; i<KNumOfAttempts; i++)
	    {
	    User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeRepository));
	    CleanupStack::PopAndDestroy(rep);
	    }
	fastCountertimeToOpenXLargeInis = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);

	//=========================================================================================
	//single .cre
	startFastCounter = User::FastCounter();	
	User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeCreRepository));
	fastCountertimeToOpenLargeCreRep = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);
	Logger().WriteFormat(_L("Time to open binary rep from ROM = %lu microseconds\n\n"), fastCountertimeToOpenLargeCreRep);
	CleanupStack::PopAndDestroy(rep);
	
	//=========================================================================================
	//KNumOfAttempts .cre
	startFastCounter = User::FastCounter();	
	for (TInt j=0; j<KNumOfAttempts; j++)
	    {	    
  	    User::LeaveIfNull(rep = CRepository::NewLC(KUidLargeCreRepository));
  	    CleanupStack::PopAndDestroy(rep);
	    }
	fastCountertimeToOpenXLargeCres = FastCountToMicrosecondsInInt(User::FastCounter() - startFastCounter);

    //=========================================================================================	    
	//average 
	TInt64 averageFastCounterTimeToOpenIni = fastCountertimeToOpenXLargeInis / KNumOfAttempts;
	Logger().WriteFormat(_L("Average time to open text based rep from ROM = %lu  microseconds\n\n"), averageFastCounterTimeToOpenIni);
	
	TInt64 averageFastCounterTimeToOpenCre = fastCountertimeToOpenXLargeCres / KNumOfAttempts;
	Logger().WriteFormat(_L("Average time to open binary rep from ROM = %lu  microseconds\n\n"), averageFastCounterTimeToOpenCre);
  	
  	//=========================================================================================
    //prove that opening .ini is slower than opening .cre files 
    // (even though the difference is now much less after the performance 
    // improvements of PREQ 1192, the test is now much more precise by using fastcounter)
#if !defined(__WINS__) && !defined(__WINSCW__)
    TEST(fastCountertimeToOpenLargeIniRep > fastCountertimeToOpenLargeCreRep);
    TEST(fastCountertimeToOpenXLargeInis > fastCountertimeToOpenXLargeCres);
    TEST(averageFastCounterTimeToOpenIni > averageFastCounterTimeToOpenCre);
#endif		

    //==========================================================================================
    //benchmarks - open
	#if !defined(__WINS__) && !defined(__WINSCW__)
	    #ifdef _DEBUG
      	  	const TInt64 KOpenCreTime  = 150000; //armv5 debug
	    #else
      	   const TInt64 KOpenCreTime  = 75000;  //armv5 urel
      	#endif
	
	TEST(averageFastCounterTimeToOpenCre < KOpenCreTime);      	
	
    #endif//!defined(__WINS__) && !defined(__WINSCW__)

    //========================================================================

	CleanupFileFromCDriveL(KUidLargeRepository);
	CleanupFileFromCDriveL(KUidLargeCreRepository);

	// Enable cache
	r = SetGetParameters(TIpcArgs(EEnableCache, KDefaultEvictionTimeout, KDefaultCacheSize));
	TEST(r==KErrNone);

	SetTestStepResult(EPass);
	
	return TestStepResult();
#endif
	}
