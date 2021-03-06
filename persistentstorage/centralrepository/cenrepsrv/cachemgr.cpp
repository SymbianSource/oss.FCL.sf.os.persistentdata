// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "srvrepos_noc.h"
#include "cachemgr.h"
#include <bsul/bsul.h>

#define UNUSED_VAR(a) a = a

_LIT(KCacheLit, "CoarseGrainedCache");
_LIT(KDefaultCacheSizeLit, "size");
_LIT(KDefaultEvictionTimeoutLit, "timeout");
    
CRepositoryCacheManager* CRepositoryCacheManager::NewLC(RFs& aFs)
	{
	CRepositoryCacheManager* self = new(ELeave) CRepositoryCacheManager;
	CleanupStack::PushL(self);
	self->ConstructL(aFs);
	return self;
	}

CRepositoryCacheManager::~CRepositoryCacheManager()
	{
	DisableCache(ETrue);
	}

void CRepositoryCacheManager::ConstructL(RFs& aFs)
	{
	CTimer::ConstructL();
	
	BSUL::CIniFile16* iniFile = NULL;
	TInt res = KErrNone;
	TBuf<KMaxFileName> iniFileName;
	
	iniFileName.Copy( *TServerResources::iInstallDirectory );
	iniFileName.Append( KCacheMgrIniFile );	
	TRAP(res, iniFile = BSUL::CIniFile16::NewL(aFs, iniFileName, ETrue));
	if(res==KErrNotFound)
		{
		// if RomDirectory exists
		if (TServerResources::iRomDirectory)
			{
			iniFileName.Copy( *TServerResources::iRomDirectory );
			iniFileName.Append( KCacheMgrIniFile );	
			TRAP(res, iniFile = BSUL::CIniFile16::NewL(aFs, iniFileName, ETrue));
			}
		if(res==KErrNotFound)
			{
			__CENTREP_TRACE1("CENTREP: Central Repository Cache Parameters ini file %S not found. Default values will be used.", &KCacheMgrIniFile);
			return;
			}
		}
	if (res != KErrNone)
		{
		User::Leave(res);
		}
		
	CleanupStack::PushL(iniFile);
	
	TBuf<20> buffer;
	TPtrC ptr(buffer);
	
	// find the value
	res = iniFile->FindVar(KCacheLit(), KDefaultCacheSizeLit(), ptr);
	TLex lex(ptr);

	TBool valueFound = EFalse;
	
	// if the value can't be found or can't be converted into a positive integer, use the default
	if (res != KErrNone || lex.Val(iCacheSize) != KErrNone || iCacheSize < 0)	
		{
		iCacheSize = KDefaultCacheSize;
		}
	else
		{
		valueFound = ETrue;			
		}
		
	// if the value can be found, convert it
	if (iniFile->FindVar(KCacheLit(), KDefaultEvictionTimeoutLit(), ptr) == KErrNone)
		{
		TInt tempTimeout;
		lex.Assign(ptr);
		// if the value can be converted into a positive integer, assign it to iDefaultTimeout.
		if (lex.Val(tempTimeout) == KErrNone && tempTimeout >= 0)
			{
			iDefaultTimeout = tempTimeout;
			valueFound = ETrue;
			}
		}
	
#ifdef _DEBUG
	// in Debug mode, if the Cache ini file exists either in install directory or 
	// rom directory but does not contains the correct section "CoarseGrainedCache"
	// nor any key of "size" and "timeout", the server panics.
	if(! valueFound)
	{
	Panic(ECacheIniFileCorrupted);
	}
#else
	UNUSED_VAR(valueFound);
#endif		

	CleanupStack::PopAndDestroy(iniFile);
	}

void CRepositoryCacheManager::EnableCache(TInt aDefaultTimeout, TInt aCacheSize)
	{
	if (aDefaultTimeout>0)
		{
		iDefaultTimeout = aDefaultTimeout;
		}
	if (aCacheSize>0)
		{
		iCacheSize = aCacheSize;
		}
	
	EnableCache();
	}

void CRepositoryCacheManager::EnableCache()
	{
	// If disabled, enable
	if (!iEnabled)
		{
		iEnabled = ETrue;
		__CENTREP_TRACE2("CENTREP: Cache Enabled. Size:%d Default Timeout:%d", iCacheSize, iDefaultTimeout.Int());
		}
	}
		
void CRepositoryCacheManager::DisableCache(TBool aFullFlush)
	{
	// If enabled, disable
	if (iEnabled)
		{
		// cancel any outstanding timer
		Cancel();

		FlushCache(aFullFlush);

		iEnabled = EFalse;
		__CENTREP_TRACE("CENTREP: Cache Disabled");
		}
	}

void CRepositoryCacheManager::RescheduleTimer(const TTime& aTimeInUTC)
	{
	
	TTime now;
	now.UniversalTime();
	
	//Get the 64bit time interval between now and the cache timeout
	TTimeIntervalMicroSeconds interval64 = aTimeInUTC.MicroSecondsFrom(now);
	TTimeIntervalMicroSeconds32 interval32(iDefaultTimeout);
	
	//If the interval is positive, i.e. the timeout is in the future, convert 
	//this interval to a 32 bit value, otherwise use the default timeout
	if(interval64 > 0)
		{
		//If the interval value is less than the maximum 32 bit value cast
		//interval to 32 bit value, otherwise the interval is too large for 
		//a 32 bit value so just set the interval to the max 32 bit value
		const TInt64 KMax32BitValue(KMaxTInt32);
		interval32 = (interval64 <= KMax32BitValue) ? 
				static_cast<TTimeIntervalMicroSeconds32>(interval64.Int64()): KMaxTInt32;
		}

	//Reschedule the timer
	After(interval32);

	}

void CRepositoryCacheManager::RemoveIdleRepository(CSharedRepository* aRepository)
	{
	if (iEnabled)
		{
		TInt i;
		TInt count=iIdleRepositories.Count();
		for(i=count-1; i>=0; i--)
			{
			if(iIdleRepositories[i].iSharedRepository==aRepository)
				{
				break;
				}
			}
		
		// Idle repository might not be found in the list if multiple clients try to open the same 
		// repository at the same time. First client will remove it and second one will not find it
		if(i>=0)
			{
			__CENTREP_TRACE1("CENTREP: Cache Hit when opening repository %x", aRepository->Uid().iUid);

			iTotalCacheUsage -= iIdleRepositories[i].iSharedRepository->Size();		
			iIdleRepositories.Remove(i);
			
			// If this was the first repository on the list, it means its timer is still ticking. 
			// We have to stop it and ...
			if (i==0)
				{
				Cancel();
				// if there's still other repositories in the list, reschedule the timer with the
				// new top-of-the-list  
				if (iIdleRepositories.Count())
					{
					RescheduleTimer(iIdleRepositories[0].iCacheTime);
					}
				}
			}
		else
			{
			__CENTREP_TRACE1("CENTREP: Cache Miss when opening repository %x", aRepository->Uid().iUid);
			}
		}
	}

#ifdef CACHE_OOM_TESTABILITY
  	// This code is only for tesing and doesn't go into MCL
	// Hence hide the leave in a macro instead of making StartEvictionL
#define TEST_CODE_LEAVE(x) User::Leave(x)
#endif	

TBool CRepositoryCacheManager::StartEviction(CSharedRepository*& aRepository)
	{
	// find the item in the cache and remove it if it exists to reset the timer
	RemoveIdleRepository(aRepository);

	TInt64 lastTop = 0;
	
	if (iIdleRepositories.Count())
		{
		lastTop = iIdleRepositories[0].iCacheTime.Int64();
		}

	// Execute the forced eviction algorithm only if it will make sense
	// The eviction makes sense if:
	// - there's anything in the cache to evict
	// - the repository we're trying to cache can fit in the cache after evictions
	if (iIdleRepositories.Count() && (aRepository->Size() < iCacheSize))
		{
		// Check to see if current cache size + the current repository size is overshooting the limit
		if (iTotalCacheUsage + aRepository->Size() > iCacheSize)
			{
			// Forced eviction
			__CENTREP_TRACE3("CENTREP: Cache Size Exceeded: Current(%d)+Size(%d)>Cache(%d)", iTotalCacheUsage, aRepository->Size(), iCacheSize);
			
			// Sort in the forced eviction order
			TLinearOrder<TRepositoryCacheInfo> forcedSortOrder(CRepositoryCacheManager::ForcedEvictionSortOrder);
			iIdleRepositories.Sort(forcedSortOrder);
			
			// Evict one by one until there's enough cache space or we run out of idle reps
			do
				{
				__CENTREP_TRACE1("CENTREP: Forced Eviction of repository %x", iIdleRepositories[0].iSharedRepository->Uid().iUid);			
				iTotalCacheUsage -= iIdleRepositories[0].iSharedRepository->Size();
				Evict(0);
				iIdleRepositories.Remove(0);		
				} while (iIdleRepositories.Count() && (iTotalCacheUsage + aRepository->Size() > iCacheSize));
			
#ifdef CENTREP_TRACE			
			if (!iIdleRepositories.Count())
				{
				__CENTREP_TRACE("CENREP: Cache Emptied by Forced Eviction");
				}
#endif				
			// Re-sort to timer order for normal operation
			TLinearOrder<TRepositoryCacheInfo> timerSortOrder(CRepositoryCacheManager::TimerEvictionSortOrder);
			iIdleRepositories.Sort(timerSortOrder);
			};
		}
	
	// See if there's enough space now
	if (iTotalCacheUsage + aRepository->Size() > iCacheSize)
		{
		return EFalse;
		}

	// Create new item for the cache and insert it in the list
	TRepositoryCacheInfo repInfo;
	
	repInfo.iCacheTime.UniversalTime();
	repInfo.iCacheTime += TTimeIntervalMicroSeconds32(iDefaultTimeout);
	repInfo.iSharedRepository = aRepository;
	
	TLinearOrder<TRepositoryCacheInfo> timerSortOrder(CRepositoryCacheManager::TimerEvictionSortOrder);
	// With the same timeout value assigned to all repositories, no two repositories can have the same 
	// timeout theoretically, so InsertInOrder is sufficient. But in practice, because of the poor 
	// resolution of the NTickCount() function used by TTime::UniversalTime(), InsertInOrderAllowRepeats 
	// should be used instead of InsertInOrder to allow for duplicate timer values caused by two 
	// repositories cached in quick succession (<1ms)
	TInt err = iIdleRepositories.InsertInOrderAllowRepeats(repInfo, timerSortOrder);
#ifdef CACHE_OOM_TESTABILITY
  	// This code is only for tesing and doesn't go into MCL
  	if (err == KErrNoMemory)	
  		{
  		TServerResources::iObserver->RemoveOpenRepository(aRepository);
  		aRepository = NULL;
  		// Should Leave here for the OOM tests to successfully complete. 
		TEST_CODE_LEAVE(err);
  		}
#endif	
	if (err!=KErrNone)
		{
		return EFalse;
		}

	iTotalCacheUsage += repInfo.iSharedRepository->Size();
	
	// Only reset timer if necessary. This operation takes time and doing it every time reduces performance considerably
	if (lastTop != iIdleRepositories[0].iCacheTime.Int64())
		{
		// reset timer to the new top-of-the-list
		Cancel();
		RescheduleTimer(iIdleRepositories[0].iCacheTime);
		}
		
	return ETrue;
	}

void CRepositoryCacheManager::FlushCache(TBool aFullFlush)
	{
	// iterate through idle repositories (loaded in memory, scheduled to be evicted)
	TInt idleRepCount = iIdleRepositories.Count();
	for(TInt repCount = idleRepCount - 1; repCount >= 0 ; repCount--)	
		{
		// check if there are any observers listening (to see if any client is connected to this repository)
		if (aFullFlush || (TServerResources::iObserver->FindConnectedRepository(iIdleRepositories[repCount].iSharedRepository->Uid())==KErrNotFound))
			{
			// if the client has already been disconnected, unload from memory
			Evict(repCount);
			}
		}
	// this whole iteration and search above can be replaced by a simple reference counter variable check,
	// if the server is redesigned using a resource manager type pattern with CSharedRepository object as a resource
	
	// empty the list
	iIdleRepositories.Reset();
	
	iTotalCacheUsage = 0;
	__CENTREP_TRACE1("CENTREP: Cache Flush: %d repositories flushed", idleRepCount);
	}
	
// Evict removes items from iOpenRepositories list, not from iIdleRepositories list
void CRepositoryCacheManager::Evict(TInt aIdleRepIndex)
	{
	// find,remove and delete the idle repositories' pointers in the open repositories list 
	TServerResources::iObserver->RemoveOpenRepository(iIdleRepositories[aIdleRepIndex].iSharedRepository);
	}
		
void CRepositoryCacheManager::RunL()
	{
	TTime now;

	now.UniversalTime();

	TInt count = iIdleRepositories.Count();
	
	// repositories that are involved in active transactions are not idle.
	// this checks to make sure that we're not trying to reclaim memory that
	// is actually still currently in use.
	
	for (TInt i = 0;i < count;i++)
		{
		if (iIdleRepositories[i].iCacheTime > now)
			{
			break;
			}
		
		if (iIdleRepositories[i].iSharedRepository->IsTransactionActive())
			{
			__CENTREP_TRACE1("CRepositoryCacheManager::RunL - rescheduling UID 0x%x, in active transaction",
					iIdleRepositories[i].iSharedRepository->Uid().iUid);
			StartEviction(iIdleRepositories[i].iSharedRepository);
			return;
			}
		}
	

	// Try to evict all the repositories which have expired. There might be more than one repository
	// destined to expire at the same time, or there are more than one repository with expiry times
	// between the scheduled expiry time and now (which theoretically should have been the same, but maybe
	// because of other procesor activity, the timer Active Object just got late a bit)
	while((iIdleRepositories.Count()) && (iIdleRepositories[0].iCacheTime<=now))
		{
		__CENTREP_TRACE1("CENTREP: Normal Eviction of repository %x", iIdleRepositories[0].iSharedRepository->Uid().iUid);			
		// Always remove from the top of the sorted list
		iTotalCacheUsage -= iIdleRepositories[0].iSharedRepository->Size();		
		Evict(0);
		iIdleRepositories.Remove(0);		
		};
		
	// reschedule to run again at the expiry date of next repository on the list, if any
	if (iIdleRepositories.Count())
		{
		RescheduleTimer(iIdleRepositories[0].iCacheTime);
		}
	else
		{
		__CENTREP_TRACE("CENTREP: Cache Empty/Timer Disabled");			
		}
	}

TInt CRepositoryCacheManager::ForcedEvictionSortOrder(const TRepositoryCacheInfo &aRepository1, const TRepositoryCacheInfo &aRepository2)
	{
/*
   The code in the comments below is the original simple-to-read version of the algebraically
   simplified production code. 

	TTime now;

	now.UniversalTime();

	// we calculate the ages of the repositories by taking the difference between now and when
	// they were last became idle. When refactoring, the calculation of the absolute ages will be 
	// eleminated and the age difference between the repositories will be used in the formula instead
	
	TTimeIntervalMicroSeconds age1 = now.MicroSecondsFrom(aRepository1.iCacheTime);
	TTimeIntervalMicroSeconds age2 = now.MicroSecondsFrom(aRepository2.iCacheTime);
	
	// then divide the resulting numbers by conversion constant to get a number in a compatible unit
	// to the size. This operation reduces the microsecond-based values to having an approx. max
	// of 100K units

	TInt t1 = age1.Int64()/KTimeoutToSizeConversion;
	TInt t2 = age2.Int64()/KTimeoutToSizeConversion;
	
	// the resulting normalized time difference values are added to the size of the repository
	// resulting in an implicit %50 weight in the overall importance value 
	// An approx. maximum size of a repository is assumed to be around 100K
	
	TInt importance1 = t1+aRepository1.iSharedRepository->Size();
	TInt importance2 = t2+aRepository2.iSharedRepository->Size();
	
	// the difference between the importances of the repositories determine the sorting order

	return static_cast<TInt>(importance1-importance2);
*/	
	//	after refactoring, the resulting formula is this one
	return static_cast<TInt>(((aRepository1.iCacheTime.Int64()-aRepository2.iCacheTime.Int64())/KTimeoutToSizeConversion)+(aRepository1.iSharedRepository->Size()-aRepository2.iSharedRepository->Size()));	
	}

TInt CRepositoryCacheManager::TimerEvictionSortOrder(const TRepositoryCacheInfo &aRepository1, const TRepositoryCacheInfo &aRepository2)
	{
	return static_cast<TInt>(aRepository1.iCacheTime.Int64()-aRepository2.iCacheTime.Int64());
	}
