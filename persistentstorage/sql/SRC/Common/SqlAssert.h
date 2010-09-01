// Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __SQLASSERT_H__
#define __SQLASSERT_H__

#include <e32std.h>
#include "UTraceSql.h"

//#define _SQLPROFILER	//Enable _SQLPROFILER if you want to use the TSqlResourceProfiler functions. Do not forget the same macro declaration in os_symbian.cpp file.

#ifdef _DEBUG
#define _ASSERTIONS
//#define _LOGGING
//#define _NOTIFY		//Enable _NOTIFY if you want to get more detailed output in _DEBUG mode
#endif

const TInt KSqlLeavePanic = -359;//The (last-1) error code from the reserved area for the SQL component.

//Forward declarations
class RMessage2;

/**
Set of useful functions to print diagnostic messages on the console when an error/leaving occurs.

@internalComponent
*/
class Util
	{
	friend void UtilFileNameTest();
	
public:
	static void GetTimeStr(TDes& aWhere);
	static TInt Assert(const TText* aFile, TInt aLine, TInt aPanicCode);
	static void Leave(const TText* aFile, TInt aLine, TInt aError);
	static TInt LeaveIfError(const TText* aFile, TInt aLine, TInt aError);
	static const void* LeaveIfNull(const TText* aFile, TInt aLine, const void* aPtr);
	static TInt PanicClientL(const TText* aFile, TInt aLine, const RMessage2& aMessage, TInt aPanicCode);
	static void ErrorPrint(const TDesC& aMsg, TInt aErr);
	static void ErrorPrint(const TDesC& aMsg, const TDesC& aStr);
	
private:
	static TPtrC Filename(const TText* aFile);
	
	};

#define __STRING(str) _S(str)

//This macro should be used when there is a need to panic the client/server if "expr" condition is not satisfied.
//Works in both debug and release modes.
#define __SQLASSERT_ALWAYS(expr, panicCode)	(void)((expr) || Util::Assert(__STRING(__FILE__), __LINE__, panicCode))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////       _ASSERTIONS          /////////////////////////////////////////////////////////
#ifdef _ASSERTIONS

//This macro should be used when there is a need to panic the client/server if "expr" condition is not satisfied.
//Works in only in debug mode. In release mode evaluates to nothing.
#define __SQLASSERT(expr, panicCode)	(void)((expr) || Util::Assert(__STRING(__FILE__), __LINE__, panicCode))

#else

#define __SQLASSERT(expr, panicCode) 	void(0)

#endif //_ASSERTIONS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////          _LOGGING          //////////////////////////////////////////////////////////
#if defined _LOGGING || defined SYMBIAN_TRACE_SQL_ERR

//This macro should be used to log the message "msg" containing the "err" error code.
//The message "msg" should contain the format specifier %d.
//Works only in debug mode. In release mode evaluates to nothing.
#define __SQLLOG_ERR(msg, err)		Util::ErrorPrint(msg, err)

//This macro should be used to log the message "msg" containing the "str" string.
//The message "msg" should contain the format specifier %S.
//Works only in debug mode. In release mode evaluates to nothing.
#define __SQLLOG_STRING(msg, str)	Util::ErrorPrint(msg, str)

#else

#define __SQLLOG_ERR(msg, err) 	    do {} while(0)
#define __SQLLOG_STRING(msg, str) 	do {} while(0)

#endif //_LOGGING || SYMBIAN_TRACE_SQL_ERR

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////          _NOTIFY          ///////////////////////////////////////////////////////////
#if defined _NOTIFY || defined SYMBIAN_TRACE_SQL_ERR

//This macro should be used to leave with "err" error code.
//In debug mode prints the file name and the line number where the leaving occurs and then leaves.
//In release mode only leaves.
#define __SQLLEAVE(err) 					Util::Leave(__STRING(__FILE__), __LINE__, err)

//This macro should be used to leave with "err" error code, if the error code is negative.
//In debug mode prints the file name and the line number where the leaving occurs and then leaves.
//In release mode only leaves.
#define __SQLLEAVE_IF_ERROR(err) 			Util::LeaveIfError(__STRING(__FILE__), __LINE__, err)

//This macro should be used to leave with KErrNoMemory if "ptr" argument is NULL.
//In debug mode prints the file name and the line number where the leaving occurs and then leaves.
//In release mode only leaves.
#define __SQLLEAVE_IF_NULL(ptr) 			Util::LeaveIfNull(__STRING(__FILE__), __LINE__, ptr)

//This macro should be used to panic the client and leave if "expr" condition is not satisfied.
//In debug mode prints the file name and the line number where the leaving occurs and then 
//panics the client and leaves.
//In release mode only panics the client and leaves.
#define __SQLPANIC_CLIENT(expr, msg, panicCode)	(void)((expr) || Util::PanicClientL(__STRING(__FILE__), __LINE__, msg, panicCode))

#else

#define __SQLLEAVE(err) 					User::Leave(err)
#define __SQLLEAVE_IF_ERROR(err) 			User::LeaveIfError(err)
#define __SQLLEAVE_IF_NULL(ptr) 			User::LeaveIfNull(ptr)
#define __SQLPANIC_CLIENT(expr, msg, panicCode)	(void)((expr) || ::SqlPanicClientL(msg, panicCode))

#endif //_NOTIFY || SYMBIAN_TRACE_SQL_ERR

#endif//__SQLASSERT_H__
