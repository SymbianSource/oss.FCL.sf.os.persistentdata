// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __SQLSRVBURINTERFACE_H__
#define __SQLSRVBURINTERFACE_H__

#include <f32file.h>

/**
MSqlSrvBurInterface is needed for performing "Backup&Restore" actions on secure SQL databases.

@internalComponent
*/
class MSqlSrvBurInterface
	{
public:		
	virtual RFs& Fs() = 0;
	virtual void GetBackUpListL(TSecureId aUid, RArray<TParse>& aFileList) = 0;
	};

#endif//__SQLSRVBURINTERFACE_H__
