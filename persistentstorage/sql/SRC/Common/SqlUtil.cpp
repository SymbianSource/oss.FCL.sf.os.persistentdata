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

#include <e32svr.h>
#include <sqldb.h>		//ESqlAtRow, ESqlAtEnd, ESqlErrGeneral
#include "SqlUtil.h"
#include "SqlPanic.h"	//SqlPanic(), TSqlPanic
#include "sqlite3.h"	//SQLITE_OK, SQLITE_ROW, SQLITE_DONE
#include "UTraceSql.h"

/**
SQL panic category.

@internalComponent
*/
_LIT(KPanicCategory,"SqlDb");

/**
Panics the caller with aPanicCode panic code.
The call will terminate the thread where it is called from.

@param aPanicCode Panic code.

@internalComponent
*/
void SqlPanic(TSqlPanic aPanicCode)
	{
	User::Panic(KPanicCategory, aPanicCode);
	}
	
/**
Panics the client with aPanicCode panic code.
This function is used by the SQL server to panic the caller (the client).

@param aMessage Client's message
@param aPanicCode Panic code.

@leave KSqlLeavePanic

@return KErrNone

@internalComponent
*/
TInt SqlPanicClientL(const RMessage2& aMessage, TSqlPanic aPanicCode)
	{
	aMessage.Panic(KPanicCategory, aPanicCode);
	__SQLLEAVE(KSqlLeavePanic);
	return KErrNone;
	}
	
/**
Processes SQL database error code and OS error code and returns unified error code.
If aSqlError == SQLITE_ROW then the function returns KSqlAtRow.
If aSqlError == SQLITE_DONE then the function returns KSqlAtEnd.
If aSqlError == SQLITE_NOMEM then the function returns KErrNoMemory.
If aOsError != KErrNone then the function returns aOsError.
Otherwise the function converts aSqlError to one of error codes in [KSqlErrGeneral..KSqlErrStmtExpired] range.

@param aSqlError SQL database error code.
@param aOsError OS error code.

@return Database specific error code.

@panic SqlDb 4 in debug mode - if aSqlError < 0
@panic SqlDb 4 in debug mode - if aOsError > 0

@internalComponent
*/
TInt Sql2OsErrCode(TInt aSqlError, TInt aOsError)
	{
	__SQLASSERT(aSqlError >= SQLITE_OK && aOsError <= KErrNone, ESqlPanicBadArgument);
	TInt err = KErrNone;
	if(aOsError == KErrDiskFull)
		{//Whatever is the aSqlError value, even SQLITE_OK, never ignore KErrDiskFull errors
		 //(For example: ROLLBACK statement execution, when the disk is full).
		err = aOsError;
		}
	else if(aSqlError == SQLITE_ROW)
		{
		err = KSqlAtRow;
		}
	else if(aSqlError == SQLITE_DONE)
		{
		err = KSqlAtEnd;
		}
	else if(aSqlError == SQLITE_NOMEM)
		{
		err = KErrNoMemory;
		}
	else if(aSqlError == SQLITE_AUTH)
		{
		err = KErrPermissionDenied;
		}
	else if(aSqlError == SQLITE_NOTADB)
		{
		err = KSqlErrNotDb;	
		}
	else if(aSqlError > SQLITE_OK)
		{
		err = aOsError != KErrNone ? aOsError : KSqlErrGeneral - aSqlError + 1;
		}
	return err;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////   class Util   ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined _LOGGING || defined SYMBIAN_TRACE_SQL_ERR

/**
This function is used to log the message "msg" containing the "err" error code.
The message "msg" should contain the format specifier %d.

The function is used when _LOGGING or SYMBIAN_TRACE_SQL_ERR is defined.

@param aMsg Error message
@param aErr Error code

@internalComponent
*/	
void Util::ErrorPrint(const TDesC& aMsg, TInt aErr)
	{
	SYMBIAN_TRACE_SQL_ERR_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EError), aMsg, aErr));
	RDebug::Print(aMsg, aErr);
	}

/**
This macro should be used to log the message "msg" containing the "str" string.
The message "msg" should contain the format specifier %S.

The function is used when _LOGGING or SYMBIAN_TRACE_SQL_ERR is defined.

@param aMsg Error message
@param aErr Error code

@internalComponent
*/	
void Util::ErrorPrint(const TDesC& aMsg, const TDesC& aStr)
	{
	SYMBIAN_TRACE_SQL_ERR_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EError), aMsg, &aStr));
	RDebug::Print(aMsg, &aStr);
	}

#endif  //_LOGGING || SYMBIAN_TRACE_SQL_ERR

#if defined _ASSERTIONS

/**
The function prints out a "SQL panic" message to the console and panics the thread where it is called from.
It gives a useful information about the found error together with the source file name and line number where
it occurred.

The function is used when _ASSERTIONS is defined.

@param aFile Source file name
@param aLine Source line number
@param aPanicCode Panic code

@return KErrNone

@internalComponent
*/	
TInt Util::Assert(const TText* aFile, TInt aLine, TInt aPanicCode)
	{
	TBuf<16> tbuf;
	Util::GetTimeStr(tbuf);
	TBuf<80> buf;
	_LIT(KFormat,"**%S* SQL panic %d, at %S(%d)");
	TPtrC fname(Filename(aFile));
	SYMBIAN_TRACE_SQL_ERR_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EError), KSqlPanic, aPanicCode, &fname, aLine));
	buf.Format(KFormat, &tbuf, aPanicCode, &fname, aLine);
	RDebug::Print(buf);
	::SqlPanic(static_cast <TSqlPanic> (aPanicCode));
	return KErrNone;
	}

#else //_ASSERTIONS

/**
The function panics the thread where it is called from.

The function is used when _ASSERTIONS is not defined.

@param Not used
@param Not used
@param aPanicCode Panic code

@return KErrNone

@internalComponent
*/	
TInt Util::Assert(const TText*, TInt, TInt aPanicCode)
	{
	::SqlPanic(static_cast <TSqlPanic> (aPanicCode));
	return KErrNone;
	}
	
#endif //_ASSERTIONS

#if defined _NOTIFY || defined SYMBIAN_TRACE_SQL_ERR
	
/**
The function prints out a "SQL leave" message to the console and leaves with aError error code.
It gives a usefull information about the found error together with the source file name and line number where
it occured.

The function is used when _NOTIFY is defined.

@param aFile Source file name
@param aLine Source line number
@param aError Error code

@internalComponent
*/	
void Util::Leave(const TText* aFile, TInt aLine, TInt aError)
	{
	SYMBIAN_TRACE_SQL_ERR_ONLY(TPtrC filename(Filename(aFile)));
	SYMBIAN_TRACE_SQL_ERR_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EError), KSqlLeave, aError, &filename, aLine));	

#ifdef _NOTIFY
	TBuf<16> tbuf;
	Util::GetTimeStr(tbuf);
	TPtrC f(Filename(aFile));
	TBuf<80> buf;
	_LIT(KFormat,"**%S* SQL leave, error=%d at %S(%d)\r\n");
	buf.Format(KFormat, &tbuf, aError, &f, aLine);
	RDebug::Print(buf);
#endif //_NOTIFY
	User::Leave(aError);
	}
	
/**
The function prints out a "SQL leave" message to the console and leaves with aError error code, if it is 
negative.
It gives a usefull information about the found error together with the source file name and line number where
it occured.

The function is used when _NOTIFY is defined.

@param aFile Source file name
@param aLine Source line number
@param aError Error code

@internalComponent
*/	
TInt Util::LeaveIfError(const TText* aFile, TInt aLine, TInt aError)
	{
	if(aError<0)
		Util::Leave(aFile,aLine,aError);
	return aError;
	}

/**
The function prints out a "SQL leave" message to the console and leaves with KErrNoMemory if 
aPtr parameter is NULL.

The function is used when _NOTIFY is defined.

@param aFile Source file name
@param aLine Source line number
@param aPtr The pointer to be tested against NULL value.

@internalComponent
*/	
void* Util::LeaveIfNull(const TText* aFile, TInt aLine, void* aPtr)
	{
	if(!aPtr)
		{
		Util::Leave(aFile, aLine, KErrNoMemory);
		}
	return aPtr;
	}

/**
The function is used by the SQL server.
It prints out a "SQL panic" message to the console and panic the client.
It gives a usefull information about the found error together with the source file name and line number where
it occured.

The function is used when _NOTIFY is defined.

@param aFile Source file name
@param aLine Source line number
@param aMessage The client message, which processing caused the panic.
@param aPanicCode Error code

@leave KSqlLeavePanic

@return KErrNone;

@internalComponent
*/	
TInt Util::PanicClientL(const TText* aFile, TInt aLine, const RMessage2& aMessage, TInt aPanicCode)
	{
	SYMBIAN_TRACE_SQL_ERR_ONLY(TPtrC filename(Filename(aFile)));
	SYMBIAN_TRACE_SQL_ERR_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EError), KSqlPanicClient, aPanicCode, &filename, aLine));	
	
#ifdef _NOTIFY
	TBuf<16> tbuf;
	Util::GetTimeStr(tbuf);
	TPtrC fname(Filename(aFile));
	TBuf<80> buf;
	_LIT(KFormat,"**%S* SQL panic=%d at %S(%d)\r\n");
	buf.Format(KFormat, &tbuf, aPanicCode, &fname, aLine);
	RDebug::Print(buf);
#endif
	return ::SqlPanicClientL(aMessage, static_cast <TSqlPanic> (aPanicCode));
	}

#endif//defined _NOTIFY || SYMBIAN_TRACE_SQL_ERR

#if defined _ASSERTIONS || defined _NOTIFY ||defined SYMBIAN_TRACE_SQL_ERR

/**
Formats the current time into aWhere descriptor.

@param aWhere Output parameter. The current time will be formatted there. The buffer length should be at least 16 characters.

The function is used when _ASSERT or _NOTIFY or SYMBIAN_TRACE_SQL_ERR is defined.

@internalComponent
*/
void Util::GetTimeStr(TDes& aWhere)
	{
	TTime time;
	time.HomeTime();
	TDateTime dt = time.DateTime();
	aWhere.Format(_L("%02d:%02d:%02d.%06d"), dt.Hour(), dt.Minute(), dt.Second(), dt.MicroSecond());
	};
	
/**
The function creates and returns TPtrC object which points to aFile parameter.

@param aFile File name
@return TPtrC object pointing to aFile parameter.

The function is used when _ASSERT or _NOTIFY or SYMBIAN_TRACE_SQL_ERR is defined.

@internalComponent
*/	
TPtrC Util::Filename(const TText* aFile)
	{
	TPtrC p(aFile);
	TInt ix = p.LocateReverse('\\');
	if(ix<0)
		ix=p.LocateReverse('/');
	if(ix>=0)
		p.Set(p.Mid(1+ix));
	return p;
	}

#endif//defined _ASSERTIONS || defined _NOTIFY || SYMBIAN_TRACE_SQL_ERR
