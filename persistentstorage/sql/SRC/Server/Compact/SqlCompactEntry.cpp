// Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <e32debug.h>
#include <hal.h>
#include <sqldb.h>
#include "SqlPanic.h"
#include "SqlCompactEntry.h"
#include "SqlCompactTimer.h"
#include "SqliteSymbian.h"		//TSqlFreePageCallback

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Creates a new CSqlCompactEntry instance.

@param aFullName The full database name, including the path.
@param aConnFactoryL MSqlCompactConn factory function.
@param aSettings Background compaction settings/thresholds
@param aTimer The background compaction timer object

When the free pages threshold is reached, the background compaction 
for this entry will be kicked-off.

@return A pointer to the created CSqlCompactEntry instance

@leave KErrNoMemory, an out of memory condition has occurred;
                     Note that the function may also leave with some other database specific 
                     errors categorised as ESqlDbError, and other system-wide error codes.

@panic SqlDb 4 In _DEBUG mode. Too short or too long database name (aFullName parameter)
@panic SqlDb 4 In _DEBUG mode. NULL aConnFactoryL.
*/
CSqlCompactEntry* CSqlCompactEntry::NewLC(const TDesC& aFullName, TSqlCompactConnFactoryL aConnFactoryL, 
										  const TSqlCompactSettings& aSettings, CSqlCompactTimer& aTimer)
	{
	__SQLASSERT(aFullName.Length() > 0 && aFullName.Length() <= KMaxFileName, ESqlPanicBadArgument); 
	__SQLASSERT(aConnFactoryL != NULL, ESqlPanicBadArgument);
	CSqlCompactEntry* self = new (ELeave) CSqlCompactEntry(aSettings, aTimer);
	CleanupStack::PushL(self);
	self->ConstructL(aFullName, aConnFactoryL);
	return self;
	}

/**
Destroys the CSqlCompactEntry instance. The database connection will be closed.
*/
CSqlCompactEntry::~CSqlCompactEntry()
	{
	if(iState == CSqlCompactEntry::EInProgress)
		{
		iTimer.DeQueue(*this);
		}
	if(iConnection)
		{
		iConnection->Release();	
		}
	iFullName.Close();
	}

/**
Increments the entry reference counter.

@return The new reference counter value.
*/
TInt CSqlCompactEntry::AddRef()
	{
	SQLCOMPACTENTRY_INVARIANT();
	return ++iRefCounter;
	}

/**
Decrements the entry reference counter.
If the counter reaches zero, the CSqlCompactEntry instance will be destroyed.

@return The new reference counter value.
*/
TInt CSqlCompactEntry::Release()
	{
	SQLCOMPACTENTRY_INVARIANT();
	TInt rc = --iRefCounter;
	if(rc == 0)
		{
		delete this;	
		}
	return rc;
	}

/**
SQLite calls this function when the free pages count reaches the threshold.
The callback must have been registered at the moment of the database connection creation in order this to happen.
The callback implementation will schedule a background compaction (kicking-off the timer).
If a background compaction has already been scheduled, the implementation will only update the iPageCount data
meber value.

@param aThis A pointer to the CSqlCompactEntry object for which the free page count reached or is above the threshold.
@param aFreePageCount Free pages count.

@panic SqlDb 4 In _DEBUG mode. NULL aThis parameter.
@panic SqlDb 4 In _DEBUG mode. aFreePageCount is negative or zero.
*/
/* static */ void CSqlCompactEntry::FreePageCallback(void* aThis, TInt aFreePageCount)
	{
	__SQLASSERT(aThis != NULL, ESqlPanicBadArgument); 
	__SQLASSERT(aFreePageCount > 0, ESqlPanicBadArgument); 
	
	CSqlCompactEntry& entry = *(static_cast <CSqlCompactEntry*> (aThis));
	if(entry.iFreePageCallbackDisabled)
		{//The callback is disabled during the background compaction step.
		 //The server is single-threaded, so no other client can activate the callback.
		 //During the background compaction step the callback can be activated only by the completion of the background 
		 //compaction in which case if "entry.iPageCount" is bigger than the threshold, the page counter will be set from here
		 //and set second time from CSqlCompactEntry::Compact() - the counter value will be reduced twice.
		return;	
		}
	
	entry.iPageCount = aFreePageCount;
	if(entry.iState == CSqlCompactEntry::EInactive)
		{
		entry.iState = CSqlCompactEntry::EInProgress;
		entry.iTimer.Queue(entry);
		}
	}

/**
Initializes the CSqlCompactEntry data members with their default values.

@param aSettings Background compaction settings/thresholds
*/
CSqlCompactEntry::CSqlCompactEntry(const TSqlCompactSettings& aSettings, CSqlCompactTimer& aTimer) :
	iSettings(aSettings),
	iTimer(aTimer),
	iRefCounter(1),
	iState(CSqlCompactEntry::EInactive)
	{
	}

/**
Initializes the created CSqlCompactEntry instance.
Schedules a background compaction if the free pages count is above the threshold.

@param aFullName The full database name, including the path.
@param aConnFactoryL MSqlCompactConn factory function.

@panic SqlDb 4 In _DEBUG mode. Too short or too long database name (aFullName parameter)
@panic SqlDb 4 In _DEBUG mode. NULL aConnFactoryL.
@panic SqlDb 7 In _DEBUG mode. The CSqlCompactEntry instance has been initialized already.
*/
void CSqlCompactEntry::ConstructL(const TDesC& aFullName, TSqlCompactConnFactoryL aConnFactoryL)
	{
	__SQLASSERT(aFullName.Length() > 0 && aFullName.Length() <= KMaxFileName, ESqlPanicBadArgument); 
	__SQLASSERT(aConnFactoryL != NULL, ESqlPanicBadArgument);
	__SQLASSERT(!iConnection, ESqlPanicInternalError);
	
	__SQLLEAVE_IF_ERROR(iFullName.Create(aFullName));

	//The second parameter of TSqlFreePageCallback's constructor is expected the be threshold in pages.
	//But the connection is not established yet and the page size is not known. Hence the threshold parameter
	//value is initialized with the threshold in Kbs. The connection construction is expected to convert
	//the threshold from Kbs to pages when the connection with the database is established.
	TSqlFreePageCallback callback(this, iSettings.iFreePageThresholdKb, &CSqlCompactEntry::FreePageCallback);
	iConnection = (*aConnFactoryL)(aFullName, callback);
	__SQLASSERT(iConnection != NULL, ESqlPanicInternalError);
	
	//"callback.iThreshold > 0" is an indication that the background compaction should be kicked-off
	if(callback.iThreshold > 0) 
		{
		//Kick-off the compaction timer, if the number of the free pages is above the threshold.
		CSqlCompactEntry::FreePageCallback(this, callback.iThreshold);
		}
		
	SQLCOMPACTENTRY_INVARIANT();
	}

/**
Performs a compaction step on the database.
If the number of the free pages is bigger than the number of pages removed in one compaction step,
the function will reschedule itself for another compaction step.
If the database file is corrupted, then the function will remove the database entry from the timer queue - 
the database won't be compacted anymore.

@return KErrNoMemory, an out of memory condition has occurred;
                      Note that the function may also return some other database specific 
                      errors categorised as ESqlDbError, and other system-wide error codes.

@panic SqlDb 7 In _DEBUG mode. iPageCount <= 0 - no free pages to be processed.
@panic SqlDb 7 In _DEBUG mode. iState != CSqlCompactEntry::EInProgress.
*/
TInt CSqlCompactEntry::Compact()
	{
	//RDebug::Print(_L("++ CSqlCompactEntry::Compact() ++\r\n"));
	SQLCOMPACTENTRY_INVARIANT();
	__SQLASSERT(iPageCount > 0, ESqlPanicInternalError);
	__SQLASSERT(iState == CSqlCompactEntry::EInProgress, ESqlPanicInternalError);
	TInt processedPageCount = 0;
	iFreePageCallbackDisabled = ETrue;
	TInt err = Connection().Compact(iPageCount, processedPageCount, iSettings.iStepLength);
	iFreePageCallbackDisabled = EFalse;
	__SQLASSERT(processedPageCount >= 0, ESqlPanicInternalError);
	if(err == KErrNone)
		{
		iPageCount -= processedPageCount;
		__SQLASSERT(iPageCount >= 0, ESqlPanicInternalError);
		}
	TBool stopCompaction = err == KSqlErrCorrupt || err == KSqlErrNotDb || err == KErrCorrupt || err == KErrDisMounted;
	if(iPageCount <= 0 || stopCompaction)
		{//No more pages to compact or the file is corrupted . Stop the compacting, move to EInactive state, remove from the timer queue.
		ResetState();
		iTimer.DeQueue(*this);
		}
	SQLCOMPACTENTRY_INVARIANT();
	return err;
	}

/**
Returns the full database name, including the path.

@return Full database name.
*/
const TDesC& CSqlCompactEntry::FullName() const
	{
	SQLCOMPACTENTRY_INVARIANT();
	return iFullName;
	}

/**
Resets the CSqlCompactEntry internal state.
That means - (1) no scheduled compaction step and (2) no pending free pages to be removed.
*/
void CSqlCompactEntry::ResetState()
	{
	SQLCOMPACTENTRY_INVARIANT();
	iState = CSqlCompactEntry::EInactive;
	iPageCount = 0;
	SQLCOMPACTENTRY_INVARIANT();
	}

/**
Returns a reference to the MSqlCompactConn interface.
@return A reference to the MSqlCompactConn interface.

@panic SqlDb 7 NULL MSqlCompactConn interface.
*/
MSqlCompactConn& CSqlCompactEntry::Connection()
	{
	SQLCOMPACTENTRY_INVARIANT();
	__SQLASSERT_ALWAYS(iConnection != NULL, ESqlPanicInternalError);
	return *iConnection;
	}

#ifdef _DEBUG
/**
CSqlCompactEntry invariant.
*/
void CSqlCompactEntry::Invariant() const
	{
	__SQLASSERT(iFullName.Length() > 0 && iFullName.Length() <= KMaxFileName, ESqlPanicInternalError); 
	__SQLASSERT(iConnection != NULL, ESqlPanicInternalError);
	__SQLASSERT(iRefCounter > 0, ESqlPanicInternalError);
	__SQLASSERT(iState == CSqlCompactEntry::EInactive || iState == CSqlCompactEntry::EInProgress, ESqlPanicInternalError);
	__SQLASSERT(iPageCount >= 0, ESqlPanicInternalError);
	iSettings.Invariant();
	}
#endif//_DEBUG
