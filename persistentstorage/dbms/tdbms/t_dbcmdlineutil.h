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

#ifndef T_DBCMDLINEUTIL_H
#define T_DBCMDLINEUTIL_H

#include <f32file.h>

class RTest;

struct TCmdLineParams
	{
	inline TCmdLineParams()
		{
		SetDefaults();
		}
	
	inline void SetDefaults()
		{
		iDriveName.Copy(_L("c:"));
		iLogFileName.Zero();
		}

	TDriveName	iDriveName;
	TFileName	iLogFileName;
	};

void GetCmdLineParams(RTest& aTest, const TDesC& aTestName, TCmdLineParams& aCmdLineParams);
void PrepareDbName(const TDesC& aDeafultDbName, const TDriveName& aDriveName, TDes& aDbName);
void LogConfig(RFile& aLogFile, const TCmdLineParams& aCmdLineParams);

#endif//T_DBCMDLINEUTIL_H
