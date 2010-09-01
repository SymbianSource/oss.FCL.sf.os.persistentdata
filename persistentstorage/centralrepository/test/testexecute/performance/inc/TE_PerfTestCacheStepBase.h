/**
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
* This file define all the common values thoughout your test project
* 
*
*/



/**
 @file
*/
#ifndef __TE_PERFTESTCACHESTEPBASE_H__
#define __TE_PERFTESTCACHESTEPBASE_H__

#include <test/testexecutestepbase.h>

class CPerfTestCacheStepBase : public CTestStep
	{
public:
	CPerfTestCacheStepBase() {};
	void CommonTestStepL(void);
	};

#endif //__TE_PERFTESTCACHESTEPBASE_H__
