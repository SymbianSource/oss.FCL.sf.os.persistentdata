// Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#include <bautils.h>
#include "logservpanic.h"
#include "LogServResourceInterpreter.h"

// Constants
const TInt KLogResourceFileGranularity = 2;

// Literal constants
_LIT(KResourceFileClient,"\\private\\101f401d\\logserv.rsc");
_LIT(KResourceFileWrapper,"\\resource\\logeng\\logwrap.rsc");


/////////////////////////////////////////////////////////////////////////////////////////
// -----> CLogServResourceInterpreter (source)
/////////////////////////////////////////////////////////////////////////////////////////
CLogServResourceInterpreter::CLogServResourceInterpreter(RFs& aFsSession) :	
	iFsSession(aFsSession), iResourceFiles(KLogResourceFileGranularity),
	iResourceStringCache(CResourceStringCacheEntry::Offset()),
	iResourceStringCacheIter(iResourceStringCache) 
	{
	}

CLogServResourceInterpreter::~CLogServResourceInterpreter()
	{
	const TInt count = iResourceFiles.Count();
	for(TInt i=0; i<count; i++)
		{
		TResourceFileEntry& entry = iResourceFiles[i];
		entry.iFile.Close();
		}
	iResourceFiles.Close();

	// Iterate through list of cache entries deleting them in turn.	
	CResourceStringCacheEntry* item ;
	iResourceStringCacheIter.SetToFirst() ;
	while ((item = iResourceStringCacheIter++) != NULL)
      {
        iResourceStringCache.Remove(*item);
        delete item;
        }
    }

void CLogServResourceInterpreter::ConstructL()
	{
	// Load standard resource files
	LoadResourceFileL(KResourceFileClient, ELogServer);
	LoadResourceFileL(KResourceFileWrapper, ELogWrap);
	}

CLogServResourceInterpreter* CLogServResourceInterpreter::NewL(RFs& aFsSession)
	{
	CLogServResourceInterpreter* self = new(ELeave) CLogServResourceInterpreter(aFsSession);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CLogServResourceInterpreter::CreateResourceReaderLC(TResourceReader& aReader, TInt aId, TResourceType aType)
	{
	HBufC8* buf = GetResourceBufferL(aId, aType) ;		

	// Resource reader needs to be created with a copy of the cache buffer as otherwise
	// our cache buffer will be deleted when the resource reader is deleted!
	aReader.SetBuffer(buf->AllocLC());
	}
	
HBufC8* CLogServResourceInterpreter::GetResourceBufferL(TInt aId, TResourceType aType)
	{

	// Attempt to find the requested resource in the cache
	CResourceStringCacheEntry* cacheEntry = FindCachedResourceString(aId, aType) ;	

	HBufC8* buf ;
	if (!cacheEntry)
		{
		// Not found in cache, load from resource file and add to the
		// linked list which forms the cache. 
		buf = ResourceFileForType(aType).AllocReadLC(aId);
		cacheEntry = CResourceStringCacheEntry::NewL(aId, aType, buf) ;
		iResourceStringCache.AddLast(*cacheEntry) ;
		
		// buf is now the responsibility of iResourceStringCache and
		// will be tidied up by the destructor...
		CleanupStack::Pop(buf);
		}
	else
		{
		buf = cacheEntry->ResourceString();
		}
	return buf ;
	}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CLogServResourceInterpreter::LoadResourceFileL(const TDesC& aName, TResourceType aType)
	{
	LOGTEXT3("CLogServResourceInterpreter::LoadResourceFileL(%S, %d)", &aName, aType);

	// Find the resource file
    TFileName fileName;
	fileName.Copy(RProcess().FileName());

    TParse parse;
    parse.Set(aName, &fileName, NULL);
	fileName = parse.FullName();

	// Get language of resource file
	BaflUtils::NearestLanguageFile(iFsSession, fileName);

	// Check the entry exists on this drive (e.g. if we are running the log server
	// from RAM, then default to the ROM if no RSC on the current drive exists).
	TEntry fsEntry;
	TInt err = iFsSession.Entry(fileName, fsEntry);
	if	(err == KErrNotFound || err == KErrPathNotFound)
		{
		// Switch to ROM (we might already have been launching from the ROM,
		// in which case this will have no effect anyway).
		fileName[0] = 'Z';
		}
	else
		{
		User::LeaveIfError(err);
		}

	// Open resource file
	TResourceFileEntry entry;
	entry.iType = aType;
	CleanupClosePushL(entry.iFile);

	LOGTEXT2("CLogServResourceInterpreter::LoadResourceFileL() - localized name is: %S", &fileName);
	
	entry.iFile.OpenL(iFsSession, fileName);
	
	LOGTEXT("CLogServResourceInterpreter::LoadResourceFileL() - resource file open");

	// I'm leaving this in just in case somebody decides to try and add this code
	// without realising the consequences...
#ifdef __CAN_BREAK_BC__
	//
	// Can't use BAFL 'NAME' support in resource file, since this
	// will break data compatibility with old software using the existing
	// logwrap and logcli RSG header files :((
	entry.iFile.ConfirmSignatureL();
	__ASSERT_ALWAYS(entry.iFile.Offset() != 0, Panic(ELogNoResourceName));
#endif

	User::LeaveIfError(iResourceFiles.Append(entry));
	CleanupStack::Pop(&entry.iFile);

	LOGTEXT("CLogServResourceInterpreter::LoadResourceFileL() - end");
	}

const RResourceFile& CLogServResourceInterpreter::ResourceFileForType(TResourceType aType) const
	{
	const RResourceFile* ret = NULL;
	//
	const TInt count = iResourceFiles.Count();
	for(TInt i=0; i<count; i++)
		{
		const TResourceFileEntry& entry = iResourceFiles[i];
		if	(entry.iType == aType)
			{
			ret = &entry.iFile;
			break;
			}
		}
	__ASSERT_ALWAYS(ret != NULL, Panic(ELogNoResourceForId));
	return *ret;
	}


CLogServResourceInterpreter::CResourceStringCacheEntry * CLogServResourceInterpreter::FindCachedResourceString(const TInt aId, TResourceType aType)
	{
	CLogServResourceInterpreter::CResourceStringCacheEntry* item ;
	
	// Iterate through linked list of cache entries looking for a 
	// match - if no match found will drop out with a NULL pointer.
	iResourceStringCacheIter.SetToFirst() ;
	while ((item = iResourceStringCacheIter++) != NULL)
		{
		if ((item->ResourceType() == aType) && (item->ResourceId() == aId))
			{
			break ;
			}
		};
	return item ;
	}
	

/////////////////////////////////////////////////////////////////////////////////////////
// -----> CLogServResourceInterpreter::CResourceStringCacheEntry (source)
/////////////////////////////////////////////////////////////////////////////////////////	
CLogServResourceInterpreter::CResourceStringCacheEntry::CResourceStringCacheEntry(TInt aResourceId, CLogServResourceInterpreter::TResourceType aType, HBufC8* aResourceString) : iResourceId (aResourceId), iResourceType (aType), iResourceString(aResourceString)
	{
	}


CLogServResourceInterpreter::CResourceStringCacheEntry::~CResourceStringCacheEntry()
	{
	delete iResourceString ;
	}


CLogServResourceInterpreter::CResourceStringCacheEntry* CLogServResourceInterpreter::CResourceStringCacheEntry::NewL(TInt aResourceId, CLogServResourceInterpreter::TResourceType aType, HBufC8* aResourceString)
	{
	return new(ELeave) CResourceStringCacheEntry(aResourceId, aType, aResourceString);
	}
	
TUint CLogServResourceInterpreter::CResourceStringCacheEntry::ResourceId(void)
	{
	return iResourceId ;
	}
	
CLogServResourceInterpreter::TResourceType CLogServResourceInterpreter::CResourceStringCacheEntry::ResourceType(void)
	{
	return iResourceType ;
	}

HBufC8* CLogServResourceInterpreter::CResourceStringCacheEntry::ResourceString (void)
	{
	return iResourceString ;
	}
	
TInt CLogServResourceInterpreter::CResourceStringCacheEntry::Offset() 
	{
    return (_FOFF(CLogServResourceInterpreter::CResourceStringCacheEntry, iLink));	
	}

