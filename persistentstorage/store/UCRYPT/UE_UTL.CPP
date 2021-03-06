// Copyright (c) 1998-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "UE_STD.H"

#pragma BullseyeCoverage off
#pragma CTC SKIP

//
// Panic the process with STORE-Encryption as the category.
//
GLDEF_C void Panic(TEncryptionPanic aPanic)
	{
	_LIT(KCategory,"STORE-Encryption");
	User::Panic(KCategory,aPanic);
	}

#pragma CTC ENDSKIP
#pragma BullseyeCoverage on
