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

#include "TE_PerfTestUtilities.h"
#include "srvdefs.h"
#include <hal.h>
#include <hal_data.h>

TReal FastCountToMillisecondsInReal(TInt aFastCount)
	{
	TInt freqInHz;
	HAL::Get(HAL::EFastCounterFrequency, freqInHz);
	TReal freqInkHz = (TReal)freqInHz / 1000;
	return (TReal)aFastCount / freqInkHz;
	}

TReal FastCountToMicrosecondsInReal(TInt aFastCount)
	{
	TInt freqInHz;
	HAL::Get(HAL::EFastCounterFrequency, freqInHz);
	TReal freqInkHz = (TReal)freqInHz / 1000000LL;
	return (TReal)aFastCount / freqInkHz;
	}

TUint64 FastCountToMillisecondsInInt(TInt aFastCount)
	{
	TInt freqInHz;
	HAL::Get(HAL::EFastCounterFrequency, freqInHz);
	return (aFastCount*1000LL)/freqInHz;
	}

TUint64 FastCountToMicrosecondsInInt(TInt aFastCount)
	{
	TInt freqInHz;
	HAL::Get(HAL::EFastCounterFrequency, freqInHz);
	return (aFastCount*1000000LL)/freqInHz;
	}

TInt FreeRamSize()
	{
	TInt freeRamSize = 0;
	
	HAL::Get(HALData::EMemoryRAMFree, freeRamSize);
	
	return freeRamSize;
	}

