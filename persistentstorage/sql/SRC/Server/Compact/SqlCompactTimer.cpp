// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "SqlCompactTimer.h"
#include "SqlPanic.h"
#include "SqlCompactEntry.h"

/**
Creates new CSqlCompactTimer instance.

@param aIntervalMs The timer intrerval (ms).

@panic SqlDb 4 In _DEBUG mode. Zero or negative aIntervalMs.
*/
CSqlCompactTimer* CSqlCompactTimer::NewL(TInt aIntervalMs)
	{
	__SQLASSERT(aIntervalMs > 0, ESqlPanicBadArgument);
	CSqlCompactTimer* self = new (ELeave) CSqlCompactTimer(aIntervalMs);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
Destroys the object.
*/
CSqlCompactTimer::~CSqlCompactTimer()
	{
	Cancel();
	}

/**
If the queue is not empty, the timer will be restarted.
*/
void CSqlCompactTimer::Restart()
	{
	SQLCOMPACTTIMER_INVARIANT();
	if(!iQueue.IsEmpty())
		{
		Cancel();
		After(iIntervalMicroSec);
		}
	SQLCOMPACTTIMER_INVARIANT();
	}

/**
Adds a database entry, that requires a background compaction, to the queue.

@param aEntry The database entry to be compacted.

@see CSqlCompactEntry
*/
void CSqlCompactTimer::Queue(CSqlCompactEntry& aEntry)
	{
	SQLCOMPACTTIMER_INVARIANT();
	iQueue.AddFirst(aEntry);
	if(!IsActive())
		{
		After(iIntervalMicroSec);
		}
	SQLCOMPACTTIMER_INVARIANT();
	}

/**
Removes the specified database entry from the queue.

@param aEntry The database entry to be removed from the queue.

@see CSqlCompactEntry
*/
void CSqlCompactTimer::DeQueue(CSqlCompactEntry& aEntry)
	{
	SQLCOMPACTTIMER_INVARIANT();
	iQueue.Remove(aEntry);
	if(iQueue.IsEmpty())
		{
		Cancel();
		}
	SQLCOMPACTTIMER_INVARIANT();
	}

/**
Initializes the CSqlCompactTimer instance.

@param aIntervalMs The timer intrerval (ms).

@panic SqlDb 4 In _DEBUG mode. Zero or negative aIntervalMs.
*/
CSqlCompactTimer::CSqlCompactTimer(TInt aIntervalMs) :
	CTimer(CActive::EPriorityIdle),
	iIntervalMicroSec(aIntervalMs * 1000),
	iQueue(_FOFF(CSqlCompactEntry, iLink))
	{
	__SQLASSERT(aIntervalMs > 0, ESqlPanicBadArgument);
	}

/**
Initializes the created CSqlCompactTimer instance.
*/
void CSqlCompactTimer::ConstructL()
	{
	CTimer::ConstructL();	
	CActiveScheduler::Add(this);
	SQLCOMPACTTIMER_INVARIANT();
	}

/**
CActive::RunL() implementation.
The RunL() implementation picks-up the last CSqlCompactEntry object from the queue and calls its Compact() method.
At the end of the call, if the queue is not empty (CSqlCompactEntry::Compact() may remove the object from the queue if
the compaction has been completed), the timer will be reactivated.

@panic SqlDb 7 The queue is empty.
@panic SqlDb 7 In _DEBUG mode. The last entry in the queue is NULL.
*/
void CSqlCompactTimer::RunL()
	{
	SQLCOMPACTTIMER_INVARIANT();
	__SQLASSERT_ALWAYS(!iQueue.IsEmpty(), ESqlPanicInternalError);	
	CSqlCompactEntry* entry = iQueue.Last();
	__SQLASSERT(entry, ESqlPanicInternalError);	
	(void)entry->Compact();
	if(!iQueue.IsEmpty())
		{
		After(iIntervalMicroSec);
		}
	SQLCOMPACTTIMER_INVARIANT();
	}

#ifdef _DEBUG
/**
CSqlCompactTimer invariant.
*/
void CSqlCompactTimer::Invariant() const
	{
	__SQLASSERT(iIntervalMicroSec > 0, ESqlPanicInternalError);
	if(!iQueue.IsEmpty())
		{
		CSqlCompactEntry* entry = iQueue.Last();
		__SQLASSERT(entry != NULL, ESqlPanicInternalError);
		}
	}
#endif//_DEBUG
