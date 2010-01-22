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

#include "SqlPanic.h"			//ESqlPanicInvalidObj, ESqlPanicObjExists
#include "SqlDatabaseImpl.h"	//CSqlDatabaseImpl

/**
Returns a reference to the implementation object of RSqlDatabase - CSqlDatabaseImpl.

@panic SqlDb 2 Create() or Open() has not previously been called on this RSqlDatabase object.

@internalComponent
*/
inline CSqlDatabaseImpl& TSqlScalarFullSelectQuery::Impl() const
	{
	__SQLASSERT_ALWAYS(iDatabaseImpl != NULL, ESqlPanicInvalidObj);
	return *iDatabaseImpl;	
	}

/**
Initializes TSqlScalarFullSelectQuery data members with default values.
*/
EXPORT_C TSqlScalarFullSelectQuery::TSqlScalarFullSelectQuery() :
	iDatabaseImpl(NULL)
	{
	}
	
/**
Initializes TSqlScalarFullSelectQuery object.

@param aDatabase	A reference to the RSqlDatabase object that represents 
                    the database on which scalar fullselect queries will be executed.
*/
EXPORT_C TSqlScalarFullSelectQuery::TSqlScalarFullSelectQuery(RSqlDatabase& aDatabase) :
	iDatabaseImpl(&aDatabase.Impl())
	{
	}
	
/**
Initializes/reinitializes TSqlScalarFullSelectQuery object.

@param aDatabase	A reference to the RSqlDatabase object that represents 
                    the database on which scalar fullselect queries will be executed.
*/
EXPORT_C void TSqlScalarFullSelectQuery::SetDatabase(RSqlDatabase& aDatabase)
	{
	SQLUTRACE_PROFILER(this);
	iDatabaseImpl = &aDatabase.Impl();
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single 32-bit integer value and returns that value.

@param aSqlStmt 16-bit SELECT sql query

@return 32-bit integer column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectIntL(const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));

	TInt res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlInt, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single 64-bit integer value and returns that value.

@param aSqlStmt 16-bit SELECT sql query

@return 64-bit integer column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt64 TSqlScalarFullSelectQuery::SelectInt64L(const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));

	TInt64 res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlInt64, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single real value and returns that value.

@param aSqlStmt 16-bit SELECT sql query

@return Real column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TReal TSqlScalarFullSelectQuery::SelectRealL(const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));

	TReal res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlReal, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single text value and copies that value to the place refered by aDest parameter.

If the destination buffer is not big enough, the function will copy as much data as 
possible and will return positive value - the character length of the text column.

@param aSqlStmt 16-bit SELECT sql query
@param aDest Refers to the place where the column data will be copied

@return KErrNone, if the function completes successfully,
		Positive value, The text column value length in characters, in case if the receiving buffer 
						is not big enough.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectTextL(const TDesC& aSqlStmt, TDes& aDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));
	
	TPtr8 ptr(reinterpret_cast <TUint8*> (const_cast <TUint16*> (aDest.Ptr())), aDest.MaxLength() * sizeof(TUint16));
	TInt err = Impl().ExecScalarFullSelectL(aSqlStmt, ESqlText, ptr);
	aDest.SetLength(ptr.Length() / sizeof(TUint16));
	return err;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single binary value and copies that value to the place refered by aDest parameter.

If the destination buffer is not big enough, the function will copy as much data as 
possible and will return positive value - the byte length of the binary column.

@param aSqlStmt 16-bit SELECT sql query
@param aDest Refers to the place where the column data will be copied

@return KErrNone, if the function completes successfully,
		Positive value, The binary column value length in bytes, in case if the receiving buffer 
						is not big enough.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectBinaryL(const TDesC& aSqlStmt, TDes8& aDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aSqlStmt));
	
	return Impl().ExecScalarFullSelectL(aSqlStmt, ESqlBinary, aDest);
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single 32-bit integer value and returns that value.

@param aSqlStmt 8-bit SELECT sql query

@return 32-bit integer column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectIntL(const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);	
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));

	TInt res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlInt, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single 64-bit integer value and returns that value.

@param aSqlStmt 8-bit SELECT sql query

@return 64-bit integer column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt64 TSqlScalarFullSelectQuery::SelectInt64L(const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));

	TInt64 res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlInt64, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single real value and returns that value.

@param aSqlStmt 8-bit SELECT sql query

@return Real column value.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TReal TSqlScalarFullSelectQuery::SelectRealL(const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));
	
	TReal res;
	TPtr8 ptr(reinterpret_cast <TUint8*> (&res), sizeof(res));
	(void)Impl().ExecScalarFullSelectL(aSqlStmt, ESqlReal, ptr);
	return res;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single text value and copies that value to the place refered by aDest parameter.

If the destination buffer is not big enough, the function will copy as much data as 
possible and will return positive value - the character length of the text column.

@param aSqlStmt 8-bit SELECT sql query
@param aDest Refers to the place where the column data will be copied

@return KErrNone, if the function completes successfully,
		Positive value, The text column value length in characters, in case if the receiving buffer 
						is not big enough.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectTextL(const TDesC8& aSqlStmt, TDes& aDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));

	TPtr8 ptr(reinterpret_cast <TUint8*> (const_cast <TUint16*> (aDest.Ptr())), aDest.MaxLength() * sizeof(TUint16));
	TInt err = Impl().ExecScalarFullSelectL(aSqlStmt, ESqlText, ptr);
	aDest.SetLength(ptr.Length() / sizeof(TUint16));
	return err;
	}
	
/**
Executes a SELECT query which is expected to return a single row consisting of
a single binary value and copies that value to the place refered by aDest parameter.

If the destination buffer is not big enough, the function will copy as much data as 
possible and will return positive value - the character length of the text column.

@param aSqlStmt 8-bit SELECT sql query
@param aDest Refers to the place where the column data will be copied

@return KErrNone, if the function completes successfully,
		Positive value, The binary column value length in bytes, in case if the receiving buffer 
						is not big enough.

@leave  KErrNotFound, If there is no record,
        The function may leave with database specific errors categorised as ESqlDbError and
  		other system-wide error codes.
*/
EXPORT_C TInt TSqlScalarFullSelectQuery::SelectBinaryL(const TDesC8& aSqlStmt, TDes8& aDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam, 1, &aSqlStmt));
	
	return Impl().ExecScalarFullSelectL(aSqlStmt, ESqlBinary, aDest);
	}
