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

#ifndef __LOGSERVRESOURCEINTERPRETER_H__
#define __LOGSERVRESOURCEINTERPRETER_H__

#include <barsc.h>
#include <barsread.h>
#include <logcli.h>

/**
Gives an access to the server resource file and maintains a cache of the retrieved resource entries.

@internalComponent
*/
class CLogServResourceInterpreter : public CBase
	{
public:
	enum TResourceType
		{
		ELogWrap = 0,
		ELogServer
		};

public:
	static CLogServResourceInterpreter* NewL(RFs& aFsSession);
	~CLogServResourceInterpreter();
    void CreateResourceReaderLC(TResourceReader& aReader, TInt aId, TResourceType aType) ;//Create a resource reader for a specific resource entry

private:
    /**
    @internalComponent
    */
	struct TResourceFileEntry
	    {
		RResourceFile iFile;
		TResourceType iType;
	    };

    /**
    @internalComponent
    */
	class CResourceStringCacheEntry : public CBase
	    {
	public:
		static CResourceStringCacheEntry* NewL(TInt aResourceId, CLogServResourceInterpreter::TResourceType aResourceType, HBufC8* aResourceString);
		~CResourceStringCacheEntry();
		TUint ResourceId(void) ;
		CLogServResourceInterpreter::TResourceType ResourceType(void) ;
		HBufC8* ResourceString (void) ;
		static TInt Offset();
		
	private:
		CResourceStringCacheEntry (TInt aResourceId, CLogServResourceInterpreter::TResourceType aType, HBufC8* aResourceString);
		
	private :
		TSglQueLink iLink;
		TUint iResourceId ;
		CLogServResourceInterpreter::TResourceType iResourceType ;
		HBufC8* iResourceString ;
	    };

    CLogServResourceInterpreter(RFs& aFsSession);
    void ConstructL();
	void LoadResourceFileL(const TDesC& aName, TResourceType aType);
	const RResourceFile& ResourceFileForType(TResourceType aType) const;
	CResourceStringCacheEntry* FindCachedResourceString(const TInt aId, TResourceType aType) ;
	HBufC8* GetResourceBufferL(TInt aId, TResourceType aType) ;
		
private:
	RFs& iFsSession;
	RArray<TResourceFileEntry> iResourceFiles;
	//Used to cache strings read from resource file
	TSglQue<CResourceStringCacheEntry> iResourceStringCache;
	TSglQueIter<CResourceStringCacheEntry> iResourceStringCacheIter;
	};

#endif
