// Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "LOGPANIC.H"

_LIT(KLogPanicString, "LogWrap");

#pragma BullseyeCoverage off
#pragma CTC SKIP

GLDEF_C void Panic(TLogPanic aPanic)
	{
	User::Panic(KLogPanicString, aPanic);
	}

#pragma CTC ENDSKIP
#pragma BullseyeCoverage on
