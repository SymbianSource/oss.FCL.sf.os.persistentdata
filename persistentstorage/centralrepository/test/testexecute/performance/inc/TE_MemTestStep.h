// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 
#ifndef __TE_MEMTESTSTEP_H__
#define __TE_MEMTESTSTEP_H__

#include <test/testexecutestepbase.h>

_LIT(KMemTestName, "CentRepMemTestLogger");

#ifdef __CENTREP_SERVER_MEMTEST__
// Class for storing initial and final heap size readings for specific
// location and identifier
class TMemTestResult
	{
public:
	inline TMemTestResult(TInt32 aLocation, TInt32 aIdentifier, TBool aIsComplete) 
		: iLocation(aLocation), iIdentifier(aIdentifier), iIsComplete(aIsComplete) { }
	//location where the memory reading is done
	TInt32 iLocation;
	//identifier of the memory reading (e.g. repository id, 10th reading etc)
	TInt32 iIdentifier;
	//whether both initial and final heap size readings are valid
	TBool iIsComplete;
	//heap size before the operation starts
	TInt32 iInitHeapSize;
	//heap size after the operation ends
	TInt32 iFinalHeapSize;
	};
#endif //__CENTREP_SERVER_MEMTEST__
	
class CMemTestStep : public CTestStep
	{
public:
	CMemTestStep();
	virtual TVerdict doTestStepL(void);
protected:
#ifdef __CENTREP_SERVER_MEMTEST__
	virtual void ProcessMemTestResults(TAny* aRawData, TInt32 aRawCount, RArray<TMemTestResult>& aMemTestResults);
#endif //__CENTREP_SERVER_MEMTEST__
	};

#endif // __TE_MEMTESTSTEP_H__
