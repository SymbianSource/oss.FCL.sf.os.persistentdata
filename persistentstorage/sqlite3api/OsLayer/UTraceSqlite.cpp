// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

//The file is used only to compile and include in the image only a single
//copy of the trace strings.
#define __SQLITETRACE_STRINGS__
#include "UTraceSqlite.h"

#ifdef SYMBIAN_TRACE_SQLITE_FUNC

/**
Create the TSqlUTraceProfiler object which inserts a UTF trace, used to signal a function entry

@param aFunctionStr 	A "const char" pointer describing the function to be profiled.
						Currently this is the type signature of the function returned by 
						the __PRETTY_FUNCTION__ macro.
@param aObj  			A object pointer used to provide context for the function.

@internalComponent
*/
TSqlUTraceProfiler::TSqlUTraceProfiler(const TAny* aObj, const char* aFunctionStr):
iObj(aObj), iFunctionStr(reinterpret_cast<const TUint8*>(aFunctionStr))
	{
	UTF::Printf(UTF::TTraceContext(UTF::ESystemCharacteristicMetrics), KProfilerBegin, &iFunctionStr, iObj);
	}

/**
Destroys TSqlUTraceProfiler object which inserts a UTF trace, used to signal a function exit

@internalComponent
*/
TSqlUTraceProfiler::~TSqlUTraceProfiler()
	{
	UTF::Printf(UTF::TTraceContext(UTF::ESystemCharacteristicMetrics), KProfilerEnd, &iFunctionStr, iObj);
	}

#endif //SYMBIAN_TRACE_SQLITE_FUNC
