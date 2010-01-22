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
// TE_MemTestFileOpenStep.h
// 
//
 
#ifndef __TE_PERFTESTUTILITIES_H__
#define __TE_PERFTESTUTILITIES_H__

#include <e32base.h>

//Utility function that converts fast counter result to milliseconds in real
TReal FastCountToMillisecondsInReal(TInt aFastCount);

//Utility function that converts fast counter result to microseconds in real
TReal FastCountToMicrosecondsInReal(TInt aFastCount);

//Utility function that converts fast counter result to milliseconds in int
TUint64 FastCountToMillisecondsInInt(TInt aFastCount);

//Utility function that converts fast counter result to microseconds in int
TUint64 FastCountToMicrosecondsInInt(TInt aFastCount);

//Utility function that measures the free ram size in bytes
TInt FreeRamSize();

//Utility function to cleanup cache
void CleanupRepositoryCache();

#endif //__TE_PERFTESTUTILITIES_H__

