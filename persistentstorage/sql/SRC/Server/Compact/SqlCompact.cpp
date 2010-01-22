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

#include "SqlPanic.h"
#include "SqlCompact.h"
#include "SqlCompactEntry.h"
#include "SqlCompactTimer.h"
#include "SqlUtil.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initializes the background compaction stettings with their default values.
*/
TSqlCompactSettings::TSqlCompactSettings() :
	iStepLength(KSqlCompactStepLengthMs),
	iFreePageThresholdKb(KSqlCompactFreePageThresholdKb)
	{
	}

#ifdef _DEBUG
/**
CSqlCompactSettings invariant.
*/
void TSqlCompactSettings::Invariant() const
	{
	__SQLASSERT(iStepLength > 0, ESqlPanicInternalError);
	__SQLASSERT(iFreePageThresholdKb >= 0, ESqlPanicInternalError);
	}
#endif//_DEBUG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
The granularity of the container maintained by CSqlCompactor

@see CSqlCompactor
@internalComponent
*/
const TInt KEntriesGranularity = 16;

/**
Creates a new CSqlCompactor instance.

@param aConnFactoryL MSqlCompactConn factory function.
@param aCompactStepInterval The time intrerval (ms) between the background compaction steps.

@return A pointer to the created CSqlCompactor instance

@leave KErrNoMemory, an out of memory condition has occurred;
                     Note that the function may also leave with some other database specific 
                     errors categorised as ESqlDbError, and other system-wide error codes.

@panic SqlDb 4 In _DEBUG mode. NULL aConnFactoryL.
@panic SqlDb 4 In _DEBUG mode. Zero or negative aCompactStepInterval.
*/
CSqlCompactor* CSqlCompactor::NewL(TSqlCompactConnFactoryL aConnFactoryL, TInt aCompactStepInterval)
	{
	__SQLASSERT(aConnFactoryL != NULL, ESqlPanicBadArgument);
	__SQLASSERT(aCompactStepInterval > 0, ESqlPanicBadArgument);
	CSqlCompactor* self = new (ELeave) CSqlCompactor(aConnFactoryL);
	CleanupStack::PushL(self);
	self->ConstructL(aCompactStepInterval);
	CleanupStack::Pop(self);
	return self;
	}

/**
Destroys the CSqlCompactor instance. 
Any entries left in the container will be destroyed.
*/
CSqlCompactor::~CSqlCompactor()
	{
	for(TInt idx=iEntries.Count()-1;idx>=0;--idx)
		{
		__SQLASSERT(iEntries[idx] != NULL, ESqlPanicInternalError);
		while(iEntries[idx]->Release() != 0)
			{
			}
		}
	iEntries.Close();
	delete iTimer;
	}

/**
Restarts the background compaction timer. 
This function should be called from the server side session object's ServiceL(). 
If ServiceL() is being executed at the moment, it is very likely that the SQL client(s) will issue another
request few ms later. In order to not delay the processing of the client(s) requests, ServiceL() should call
at the end RestartTimer(). If there are database entries scheduled for a background compaction, the compaction
will be delayed by the time interval used by the CSqlCompactTimer object (default value - KSqlCompactStepInterval).

@see CSqlCompactTimer
@see KSqlCompactStepInterval
*/
void CSqlCompactor::RestartTimer()
	{
	SQLCOMPACTOR_INVARIANT();
	iTimer->Restart();
	SQLCOMPACTOR_INVARIANT();
	}

/**
If an entry referring to a database with name aFullName does not exist in the container, a new entry will be created,
a connection with the database established.
If an entry with the specidfied name already exists, no new entry wil be created, the reference counter of the existing one 
will be incremented.

@param aFullName The full database name, including the path.
@param aSettings Per-database background compaction settings

@leave KErrNoMemory, an out of memory condition has occurred;
                     Note that the function may also leave with some other database specific 
                     errors categorised as ESqlDbError, and other system-wide error codes.

@panic SqlDb 4 In _DEBUG mode. Too short or too long database name (aFullName parameter)
@panic SqlDb 7 In _DEBUG mode. An entry with the specidfied name has been found but the entry is NULL.
*/
void CSqlCompactor::AddEntryL(const TDesC& aFullName, const TSqlCompactSettings& aSettings)
	{
	__SQLASSERT(aFullName.Length() > 0 && aFullName.Length() <= KMaxFileName, ESqlPanicBadArgument);
	SQLCOMPACTOR_INVARIANT();
	CSqlCompactEntry* entry = NULL;
	TInt idx = iEntries.FindInOrder(aFullName, &CSqlCompactor::Search);
	if(idx == KErrNotFound)
		{
		entry = CSqlCompactEntry::NewLC(aFullName, iConnFactoryL, aSettings, *iTimer);
		TLinearOrder<CSqlCompactEntry> order(&CSqlCompactor::Compare);
		__SQLLEAVE_IF_ERROR(iEntries.InsertInOrder(entry, order));
		CleanupStack::Pop(entry);
		}
	else
		{
		entry = iEntries[idx];
		__SQLASSERT(entry != NULL, ESqlPanicInternalError);
		(void)entry->AddRef();
		}
	SQLCOMPACTOR_INVARIANT();
	}

/**
Decrements the reference counter of the specified entry.
If the counter reaches zero, the entry will be destroyed and removed form the container, the database connection - closed.

@param aFullName The full database name, including the path.
*/
void CSqlCompactor::ReleaseEntry(const TDesC& aFullName)
	{
	SQLCOMPACTOR_INVARIANT();
	TInt idx = iEntries.FindInOrder(aFullName, &CSqlCompactor::Search);
	__SQLASSERT(idx >= 0, ESqlPanicInternalError);
	if(idx >= 0)
		{
		CSqlCompactEntry* entry = iEntries[idx];
		__SQLASSERT(entry != NULL, ESqlPanicInternalError);
		if(entry)
			{
			if(entry->Release() == 0)
				{
				iEntries.Remove(idx);
				}
			}
		}
	SQLCOMPACTOR_INVARIANT();
	}

/**
Initializes the CSqlCompactor data members with their default values.

@param aConnFactoryL MSqlCompactConn factory function.

@panic SqlDb 4 In _DEBUG mode. NULL aConnFactoryL.
*/
CSqlCompactor::CSqlCompactor(TSqlCompactConnFactoryL aConnFactoryL) :
	iConnFactoryL(aConnFactoryL),
	iEntries(KEntriesGranularity)
	{
	__SQLASSERT(aConnFactoryL != NULL, ESqlPanicBadArgument);
	}

/**
Initializes the created CSqlCompactor instance.

@param aCompactStepInterval The time interval between the background compaction steps.

@panic SqlDb 4 In _DEBUG mode. Zero or negative aCompactStepInterval.
*/
void CSqlCompactor::ConstructL(TInt aCompactStepInterval)
	{
	__SQLASSERT(aCompactStepInterval > 0, ESqlPanicBadArgument);
	iTimer = CSqlCompactTimer::NewL(aCompactStepInterval);
	}

/**
Static method used internally for performing a search in the container using database name as a key.

@param aFullName The full database name, including the path.
@param aEntry CSqlCompactor reference.
*/
/* static */TInt CSqlCompactor::Search(const TDesC* aFullName, const CSqlCompactEntry& aEntry)
	{
	__SQLASSERT(&aEntry != NULL, ESqlPanicInternalError);
	__SQLASSERT(aFullName != NULL, ESqlPanicInternalError);
	const TDesC& fullName = *aFullName;
	__SQLASSERT(fullName.Length() > 0 && fullName.Length() <= KMaxFileName, ESqlPanicInternalError);
	__SQLASSERT(aEntry.FullName().Length() > 0 && aEntry.FullName().Length() <= KMaxFileName, ESqlPanicInternalError);
	return fullName.CompareF(aEntry.FullName());
	}

/**
Static method used internally for performing a search in the container using a CSqlCompactEntry reference as a key.
*/
/* static */TInt CSqlCompactor::Compare(const CSqlCompactEntry& aLeft, const CSqlCompactEntry& aRight)
	{
	__SQLASSERT(&aLeft != NULL, ESqlPanicInternalError);
	__SQLASSERT(&aRight != NULL, ESqlPanicInternalError);
	__SQLASSERT(aLeft.FullName().Length() > 0 && aLeft.FullName().Length() <= KMaxFileName, ESqlPanicInternalError);
	__SQLASSERT(aRight.FullName().Length() > 0 && aRight.FullName().Length() <= KMaxFileName, ESqlPanicInternalError);
	return aLeft.FullName().CompareF(aRight.FullName());
	}

#ifdef _DEBUG
/**
CSqlCompactor invariant.
*/
void CSqlCompactor::Invariant() const
	{
	__SQLASSERT(iConnFactoryL != NULL, ESqlPanicInternalError);
	__SQLASSERT(iTimer != NULL, ESqlPanicInternalError);
	iTimer->Invariant();
	for(TInt idx=iEntries.Count()-1;idx>=0;--idx)
		{
		__SQLASSERT(iEntries[idx] != NULL, ESqlPanicInternalError);
		iEntries[idx]->Invariant();
		}
	}
#endif//_DEBUG
