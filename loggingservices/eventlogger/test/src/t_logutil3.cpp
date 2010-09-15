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
#include "t_logutil3.h"
#include <bautils.h>
#include <barsc.h>
#include <centralrepository.h>
#include <logserv.rsg>
#include "LOGREPDEFS.H"

static void LoadSettingsFromResourceFileL(TInt& aContactMatchCount, TLogContactNameFormat& aContactNameFormat)
	{
	_LIT(KLogResourceFile,"z:\\private\\101f401d\\logserv.rsc");
	TFileName fileName(KLogResourceFile);
	
	RFs fs;
	CleanupClosePushL(fs);
	User::LeaveIfError(fs.Connect());
	BaflUtils::NearestLanguageFile(fs, fileName);

	// Open resource file
	RResourceFile rscFile;
	CleanupClosePushL(rscFile);
	rscFile.OpenL(fs, fileName);//OpenL() requires AllFiles capability
	
	TResourceReader reader;
	
	HBufC8* buf = rscFile.AllocReadLC(R_LOG_CONTACT_MATCH_COUNT);
	reader.SetBuffer(buf);
	aContactMatchCount = reader.ReadInt16();
	CleanupStack::PopAndDestroy(buf);
	
	buf = rscFile.AllocReadLC(R_LOG_CONTACT_NAME_FORMAT);
	reader.SetBuffer(buf);
	aContactNameFormat = static_cast <TLogContactNameFormat> (reader.ReadInt16());
	CleanupStack::PopAndDestroy(buf);
		
	CleanupStack::PopAndDestroy(&rscFile);
	CleanupStack::PopAndDestroy(&fs);
	}

//This function reads logeng repository file and returns the integer value of the given key. 
static TInt LogGetRepositoryValueL(CRepository& aRepository, TInt aKey)
	{		
	TInt val = -1;		
	User::LeaveIfError(aRepository.Get(aKey, val));
	return val;
	}

//The function opens the LogEng repository (KUidLogengRepository) and gets the values of 
//KContactMatchCountRepKey and KContactNameFormatRepKey resources.
//If it is impossible to read the values from the repository, then they will be loaded from LogServ resource file.
void LogGetContactmatchCountAndNameFormatL(TInt& aContactMatchCount, TLogContactNameFormat& aContactNameFormat)
	{
	CRepository* repository = NULL;
	TRAPD(err, repository = CRepository::NewL(KUidLogengRepository));		
	if(err == KErrCorrupt)
		{
		__ASSERT_DEBUG(!repository, User::Invariant());
		User::Leave(err);
		}
	else if(err == KErrNone)
		{
		CleanupStack::PushL(repository);
		aContactMatchCount = LogGetRepositoryValueL(*repository, KContactMatchCountRepKey);
		aContactNameFormat = static_cast <TLogContactNameFormat> (LogGetRepositoryValueL(*repository, KContactNameFormatRepKey));
		CleanupStack::PopAndDestroy(repository);
		}
	else
		{
		__ASSERT_DEBUG(!repository, User::Invariant());
		LoadSettingsFromResourceFileL(aContactMatchCount, aContactNameFormat);
		}
	}
