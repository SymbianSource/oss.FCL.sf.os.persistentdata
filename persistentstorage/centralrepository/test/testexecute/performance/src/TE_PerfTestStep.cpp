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


#include <e32std.h>
#include <centralrepository.h>
#include "t_cenrep_helper.h"
#include <hal.h>
#include <hal_data.h>
#include "srvPerf.h"
#include "srvreqs.h"
#include "TE_PerfTestStep.h"

#ifdef __CENTREP_SERVER_PERFTEST__

// Function DecodeEventAndData
// Extract the high 8-bit event Id and the low 24-bit data
// from the 32 bit integer.
//
// param aEntry - the 32 bit integer to decode
// param aEventId - return the EventId Id.
// param aData - return the 24-bit data value
inline
void DecodeEventAndData(TUint32 aEntry, TUint& aEventId, TUint32& aData)
	{
	aEventId = (aEntry >> KEventIdShiftBits) & KEventIdMask;
	aData = aEntry & KDataMask;
	}

// Function FindRepUid
// Search for the given Repository UID in aArray.
//
// param aArray - the RArray containing a number of repositories.
// param aRepUid - the rep UID to search for.
// Return - if aRepUid is in the array, return its index in array,
//          else return KErrNotFound.
TInt FindRepUid(const RArray<TRepStatistics>& aArray, TUint32 aRepUid)
	{
	for (TInt i = 0; i < aArray.Count(); i++)
		{
		if (aArray[i].MatchUid(aRepUid))
			return i;
		}
	return KErrNotFound;
	}

// Function GetFuncCodeName
// Map CentRep function code to string.
//
// param aFunc - the function code to map to string.
// Return - textual representation of the function code.
_LIT(KEInitialise, "Open");
_LIT(KECreateInt, "ECreateInt");
_LIT(KECreateReal, "ECreateReal");
_LIT(KECreateString, "ECreateString");
_LIT(KEDelete, "EDelete");
_LIT(KEGetInt, "EGetInt");
_LIT(KESetInt, "ESetInt");
_LIT(KEGetReal, "EGetReal");
_LIT(KESetReal, "ESetReal");
_LIT(KEGetString, "EGetString");
_LIT(KESetString, "ESetString");
_LIT(KEFind, "EFind");
_LIT(KEFindEqInt, "EFindEqInt");
_LIT(KEFindEqReal, "EFindEqReal");
_LIT(KEFindEqString, "EFindEqString");
_LIT(KEFindNeqInt, "EFindNeqInt");
_LIT(KEFindNeqReal, "EFindNeqReal");
_LIT(KEFindNeqString, "EFindNeqString");
_LIT(KEGetFindResult, "EGetFindResult");
_LIT(KENotifyRequestCheck, "ENotifyRequestCheck");
_LIT(KENotifyRequest, "ENotifyRequest");
_LIT(KENotifyCancel, "ENotifyCancel");
_LIT(KENotifyCancelAll, "ENotifyCancelAll");
_LIT(KEGroupNotifyRequest, "EGroupNotifyRequest");
_LIT(KEGroupNotifyCancel, "EGroupNotifyCancel");
_LIT(KEReset, "EReset");
_LIT(KEResetAll, "EResetAll");
_LIT(KETransactionStart, "ETransactionStart");
_LIT(KETransactionCommit, "ETransactionCommit");
_LIT(KETransactionCancel, "ETransactionCancel");
_LIT(KEMove, "EMove");
_LIT(KETransactionState, "ETransactionState");
_LIT(KETransactionFail, "ETransactionFail");
_LIT(KEDeleteRange, "EDeleteRange");
_LIT(KEGetSetParameters, "EGetSetParameters");
_LIT(KEClose, "EClose");
_LIT(KEEvict, "Evict");
_LIT(KUnknownFunction, "UnknownCode");

const TDesC* GetFuncCodeName(TUint aFunc)
	{
	switch (aFunc)
		{
		case EInitialise: return(&KEInitialise);
		case ECreateInt: return(&KECreateInt);
		case ECreateReal: return(&KECreateReal);
		case ECreateString: return(&KECreateString);
		case EDelete: return(&KEDelete);
		case EGetInt: return(&KEGetInt);
		case ESetInt: return(&KESetInt);
		case EGetReal: return(&KEGetReal);
		case ESetReal: return(&KESetReal);
		case EGetString: return(&KEGetString);
		case ESetString: return(&KESetString);
		case EFind: return(&KEFind);
		case EFindEqInt: return(&KEFindEqInt);
		case EFindEqReal: return(&KEFindEqReal);
		case EFindEqString: return(&KEFindEqString);
		case EFindNeqInt: return(&KEFindNeqInt);
		case EFindNeqReal: return(&KEFindNeqReal);
		case EFindNeqString: return(&KEFindNeqString);
		case EGetFindResult: return(&KEGetFindResult);
		case ENotifyRequestCheck: return(&KENotifyRequestCheck);
		case ENotifyRequest: return(&KENotifyRequest);
		case ENotifyCancel: return(&KENotifyCancel);
		case ENotifyCancelAll: return(&KENotifyCancelAll);
		case EGroupNotifyRequest: return(&KEGroupNotifyRequest);
		case EGroupNotifyCancel: return(&KEGroupNotifyCancel);
		case EReset: return(&KEReset);
		case EResetAll: return(&KEResetAll);
		case ETransactionStart: return(&KETransactionStart);
		case ETransactionCommit: return(&KETransactionCommit);
		case ETransactionCancel: return(&KETransactionCancel);
		case EMove: return(&KEMove);
		case ETransactionState: return(&KETransactionState);
		case ETransactionFail: return(&KETransactionFail);
		case EDeleteRange: return(&KEDeleteRange);
		case EGetSetParameters: return (&KEGetSetParameters);
		case EClose: return(&KEClose);
		case EEvict: return (&KEEvict);
		}
		return &KUnknownFunction;
	}
#endif //__CENTREP_SERVER_PERFTEST__

//--------------------
// class CPerfTestStep
//--------------------

// doTestStepL
// Implement the pure virtual function.
// This test fetches the performance data collected by
// CentRep server, sums the CPU time spent in servicing
// each type of request and time spent in opening each
// repository, and prints out the total.
TVerdict CPerfTestStep::doTestStepL()
	{
#ifdef __CENTREP_SERVER_PERFTEST__

	TBuf<KMaxFileName> sharedString; 

	TPtrC resultSection;
	TInt testMode;
	TInt bRet;
	// get test mode from ini file
	bRet = GetIntFromConfig(ConfigSection(), KIniTestMode, testMode);
	TESTL(bRet==1);

	_LIT(KBlankLine, "\n");
	Logger().WriteFormat(KBlankLine);	 	
	
	if (testMode == ETiming)
		{
		bRet = GetStringFromConfig(ConfigSection(), KResultsSection, resultSection);
		TESTL(bRet==1);
		}
	
	if (testMode == EBoot)
		{
		_LIT(KDoTestStepDisclaimer1, "For this test boot-up is considered to be finished when CommsDat repository is loaded 9 times.");
		Logger().WriteFormat(KDoTestStepDisclaimer1);
		_LIT(KDoTestStepDisclaimer2, "This is when the watcher thread is used to make re-attempts to connect to the network.");
		Logger().WriteFormat(KDoTestStepDisclaimer2);
		}
	
	// Setup IPC args to retreive performance data from server.
	TUint bufSize = KCentRepPerfTestArraySize * sizeof(TUint32);
	TAny* buf = User::AllocL(bufSize);
	TPtr8 bufDesc(static_cast<TUint8*>(buf), bufSize);

	TUint numValidEntries;
	TPckg<TUint> pckg(numValidEntries);

	TInt ret = SetGetParameters(TIpcArgs(EGetPerfResults, &bufDesc, &pckg));
	if (ret != KErrNone)
		{
		_LIT(KSetGetParamFailFmt, "Send msg err %d");
		ERR_PRINTF2(KSetGetParamFailFmt, ret);
		User::Free(buf);
		return EFail;
		}

	if (testMode == EBoot)
		{
		_LIT(KNumValidEntriesFmt, "Num Valid entries = %d");
		Logger().WriteFormat(KNumValidEntriesFmt, numValidEntries);
		}

	RArray<TUint32> perfData(sizeof(TUint32),static_cast<TUint32*>(buf),KCentRepPerfTestArraySize);
	CleanupClosePushL(perfData);

	// Need to find the tick frequency before using ConvertTickToSecAndMilli.
	HAL::Get(HALData::EFastCounterFrequency, iTickFreq);

	TUint sec;
	TUint milliSec;

	const TUint KNumIpcs = EEvict + 1; 
	TIpcStatistics ipcStats[KNumIpcs];
	RArray<TRepStatistics> repStats;
	CleanupClosePushL(repStats);
    TUint32 totalElapsedTicks = 0;
	const TDesC* eventName;

	// Process the data

	// Performance data has 3 parts. First event ID and CPU tick
	// to service the request. 2nd & 3rd, if event is open/close/evict
	// the time of the event and repository UID.
	TInt i;
	for (i=0; i<numValidEntries && i<KCentRepPerfTestArraySize;)
		{
		TUint eventId;
		TUint32 ticksToServiceIpc;
		TUint32 repositoryUid;

		// This entry is elapsed ticks to service the request
		DecodeEventAndData(perfData[i++], eventId, ticksToServiceIpc);
		totalElapsedTicks += ticksToServiceIpc;
		ipcStats[eventId].AddData(ticksToServiceIpc);

		TBool repOpenCloseEvict = eventId == EInitialise || eventId == EClose || eventId == EEvict;

		if (repOpenCloseEvict)
			{
			TESTL(i < numValidEntries);
			TUint32 eventTime = perfData[i++];

			TESTL(i < numValidEntries);
			repositoryUid = perfData[i++];

			// Only rep open is stored in repStats array.
			// Close and evict go into the ipcStats.
			if (eventId == EInitialise)
				{
				TInt idx = FindRepUid(repStats, repositoryUid);
				if (idx == KErrNotFound)
					{
					repStats.Append(TRepStatistics(repositoryUid, ticksToServiceIpc));
					}
				else
					{
					repStats[idx].AddData(ticksToServiceIpc);
					}
				} // if eventId == EInitialise

			if (testMode == EBoot)
				{
				// Display the time that this event occur.
				eventName = GetFuncCodeName(eventId);
				ConvertTickToSecAndMilli(eventTime, sec, milliSec);
				_LIT(KRepOpenCloseFmt, "REP %X %S at %d.%03d");
				Logger().WriteFormat(KRepOpenCloseFmt, repositoryUid, eventName, sec, milliSec);
				}
			} // if repOpenCloseEvict
		} // for i


	// Display the IPC statistics
	Logger().WriteFormat(KBlankLine);
	Logger().WriteFormat(KBlankLine);
	_LIT(KIpcStatsFmt, "IPC Statistics:");
	Logger().WriteFormat(KIpcStatsFmt);

	TInt numIpcMsgs = 0;
	TInt numLogItems = 0;
	TBuf<50> keyName;
	_LIT(KIndividualIpcStat, "%S used %d times, sum of CPU time = %d ms");
	for (i = 0; i < KNumIpcs; i++)
		{
		if (ipcStats[i].iUseCount == 0)
			{
			continue;
			}

		numIpcMsgs += ipcStats[i].iUseCount;
		eventName = GetFuncCodeName(i);
		ConvertTickToSecAndMilli(ipcStats[i].iSumElapsedTicks, sec, milliSec);
		if (sec > 0)
			{
			milliSec += sec * 1000;
			}
		Logger().WriteFormat(KIndividualIpcStat, eventName, ipcStats[i].iUseCount, milliSec);
		if (testMode == ETiming)
			{
			sharedString.AppendFormat(_L(" %S %d %d"), eventName, ipcStats[i].iUseCount, milliSec);
			numLogItems++;
			}
		}
	if (testMode == ETiming)
		{
		TBuf<2> numofItems;
		numofItems.Num(numLogItems);
		sharedString.Insert(0, numofItems);
		WriteSharedDataL(resultSection, sharedString, ESetText);
		}

	// Display Repository statistics
	Logger().WriteFormat(KBlankLine);
	Logger().WriteFormat(KBlankLine);
	_LIT(KRepStatsFmt, "Repository Statistics:");
	Logger().WriteFormat(KRepStatsFmt);

	if (testMode == ETiming)
		{
		numLogItems = 0;
		sharedString.Copy(_L(" "));
		}
	_LIT(KIndividualRepStat, "%X used %d times, sum of load time %d ms");
	for (i = 0; i < repStats.Count(); i++)
		{
		ConvertTickToSecAndMilli(repStats[i].iSumLoadTicks, sec, milliSec);
		if (sec > 0)
			{
			milliSec += sec * 1000;
			}
		Logger().WriteFormat(KIndividualRepStat, repStats[i].iRepUid,
			repStats[i].iUseCount, milliSec);
		if (testMode == ETiming)
			{
			sharedString.AppendFormat(_L(" %08X %d %d"), repStats[i].iRepUid, repStats[i].iUseCount, milliSec);
			numLogItems++;
			}
		}
		
	if (testMode == ETiming)
		{
		TBuf<2> numofItems;
		numofItems.Num(numLogItems);
		sharedString.Insert(1, numofItems);
		WriteSharedDataL(resultSection, sharedString, EAppendText);
		
		Logger().WriteFormat(KBlankLine);
		}

	if (testMode == EBoot)
		{
		// show total CPU used by CentralRepository server.
		ConvertTickToSecAndMilli(totalElapsedTicks, sec, milliSec);
		if (sec > 0)
			{
			milliSec += sec * 1000;
			}

		Logger().WriteFormat(KBlankLine);
		Logger().WriteFormat(KBlankLine);
		_LIT(KTotalCpuFmt, "Total CPU time used by CentRepSrv: %d ms");
		Logger().WriteFormat(KTotalCpuFmt, milliSec);

		_LIT(KNoteAboutTotal, "The above does not include kernel processing time and client idle time for the %d IPC messages.\n");
		Logger().WriteFormat(KNoteAboutTotal, numIpcMsgs);
		}

	// Stop performance test data recording in the server
	TInt r = SetGetParameters(TIpcArgs(EStopPerfTests));
	TEST(r==KErrNone);
		
	CleanupStack::PopAndDestroy(); // repStats
	CleanupStack::PopAndDestroy(); // perfData
	return EPass;
#else
	_LIT(KWarnTestMacroOff, "Performance test macro __CENTREP_SERVER_PERFTEST__ is disabled. Test not run.");
	WARN_PRINTF1(KWarnTestMacroOff);
	return EPass;
#endif
	}

// ConvertTickToSecAndMilli
// Convert FastCounter ticks to seconds and milli-seconds.
//
// param aTick - ticks from User::FastCounter
// param aSec - return number of seconds in aTick
// param aMilliSec - return number of ms left behind after number of
//                   seconds is subtracted from aTick.
void CPerfTestStep::ConvertTickToSecAndMilli(TUint32 aTick, TUint& aSec, TUint& aMilliSec)
	{
	aSec = aTick / iTickFreq;
	aMilliSec = ((aTick - aSec * iTickFreq) * 1000 + (iTickFreq >> 1)) / iTickFreq;
	}

//------------------------
// class TRepStatistics
//------------------------

// AddData
// Increment number of times this rep is open, sum of CPU used
// to load it, and check cache miss.
void TRepStatistics::AddData(TUint32 aElapsedTicks)
	{
	iSumLoadTicks += aElapsedTicks;
	iUseCount++;
	if (aElapsedTicks >= KTicks2LoadFromCache)
		{
		iCacheMisses++;
		}
	}

//---------------------
// class TIpcStatistics
//---------------------

// AddData
// Increment number of times the IPC msg is received by server.
// Sum time to service this type of request.
void TIpcStatistics::AddData(TUint32 aElapsedTicks)
	{
	iUseCount++;
	iSumElapsedTicks += aElapsedTicks;
	}
