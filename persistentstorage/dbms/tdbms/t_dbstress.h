// Copyright (c) 1998-2009 Nokia Corporation and/or its subsidiary(-ies).
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

// MSVC++ up to 5.0 has problems with expanding inline functions
// This disables the mad warnings for the whole project
#if defined(NDEBUG) && defined(__VC32__) && _MSC_VER<=1100
#pragma warning(disable : 4710)			// function not expanded. MSVC 5.0 is stupid
#endif


#include <d32dbms.h>
#include <s32file.h>
#include <e32test.h>
#include <e32math.h>

enum TAccount {ECash=0,EJohn,ESam,EBen,EPenny};
const TInt KAccountIDs=EPenny-ECash+1;

GLREF_C TInt StartThread(RThread& aThread,TRequestStatus& aStat);
GLREF_C TInt Random(TInt aRange);

GLREF_D TPtrC KTestDatabase,KTestDir,KLogFile;
GLREF_D RTest test;
GLREF_D TInt NewCount;
GLREF_D TInt OldCount;
GLREF_D TInt TransId;

class Timer
	{
public:
	void Start();
	TInt64 Stop();
	void Print();
private:
	TTime iTime;
	};

GLREF_D Timer RunTimer;

