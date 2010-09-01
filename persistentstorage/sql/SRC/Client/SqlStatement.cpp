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

#include "SqlStatementImpl.h"		//CSqlStatementImpl

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Initialises the pointer to the implementation object to NULL.

@capability None
*/
EXPORT_C RSqlStatement::RSqlStatement() :
	iImpl(NULL)
	{
	}

/**
Prepares the supplied 16-bit SQL statement for execution.

An RSqlStatement object can prepare and execute a parameterised SQL statement or
an SQL statement without parameters.

The function can only deal with one SQL statement at a time, i.e. if you
supply more than one SQL statement, each separated by a ";" character, then
the function returns an error.

Note that when the statement is to be used to retrieve or write blob or text data
that is over 2Mb in size then it is recommended that the RSqlBlobReadStream and 
RSqlBlobWriteStream classes or the TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of reading and writing large 
amounts of blob or text data from a database.

@param aDatabase     A reference to the RSqlDatabase object that represents 
                     the database for which the SQL statement is being prepared.
@param aSqlStmt      A string of 16-bit wide characters containing the
                     SQL statement to be prepared.

@return KErrNone,     the SQL statement has been prepared for execution successfully;
        KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the SQL statement is invalid, for example, the supplied
                      string contains more than one SQL statement, or it
                      contains an empty SQL statement.
                      Note that database specific errors categorised as ESqlDbError
                      can also be returned;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage().
		KErrPermissionDenied, the calling application does not satisfy the relevant database security policies.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if current RSqlStatement object represents a handle which operates on a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
@see RSqlBlobReadStream
@see RSqlBlobWriteStream
@see TSqlBlob
*/	
EXPORT_C TInt RSqlStatement::Prepare(RSqlDatabase& aDatabase, const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexStrParam16, &aDatabase, &aSqlStmt));

	return CSqlStatementImpl::New(iImpl, aDatabase.Impl(), aSqlStmt);
	}
	
/**
Prepares the supplied 8-bit SQL statement for execution.

An RSqlStatement object can prepare and execute a parameterised SQL statement or
an SQL statement without parameters.

The function can only deal with one SQL statement at a time, i.e. if you
supply more than one SQL statement, each separated by a ";" character, then
the function returns an error.

Note that when the statement is to be used to retrieve or write blob or text data
that is over 2Mb in size then it is recommended that the RSqlBlobReadStream and 
RSqlBlobWriteStream classes or the TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of reading and writing large 
amounts of blob or text data from a database.

@param aDatabase     A reference to the RSqlDatabase object that represents 
                     the database for which the SQL statement is being prepared.
@param aSqlStmt      A string of 8-bit wide characters containing the
                     SQL statement to be prepared.


@return KErrNone,     the SQL statement has been prepared for execution successfully;
        KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the SQL statement is invalid, for example, the supplied
                      string contains more than one SQL statement, or it
                      contains an empty SQL statement.
                      Note that database specific errors categorised as ESqlDbError
                      can also be returned;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage().
		KErrPermissionDenied, the calling application does not satisfy the relevant database security policies.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None, if current RSqlStatement object represents a handle which operates on a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
@see RSqlBlobReadStream
@see RSqlBlobWriteStream
@see TSqlBlob
*/	
EXPORT_C TInt RSqlStatement::Prepare(RSqlDatabase& aDatabase, const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexStrParam, 
			&aDatabase, &aSqlStmt));
	
	return CSqlStatementImpl::New(iImpl, aDatabase.Impl(), aSqlStmt);
	}

/**
Prepares the supplied 16-bit SQL statement for execution.

An RSqlStatement object can prepare and execute a parameterised SQL statement or
an SQL statement without parameters.

The function can only deal with one SQL statement at a time, i.e. if you
supply more than one SQL statement, each separated by a ";" character, then
the function returns an error.

Note that when the statement is to be used to retrieve or write blob or text data
that is over 2Mb in size then it is recommended that the RSqlBlobReadStream and 
RSqlBlobWriteStream classes or the TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of reading and writing large 
amounts of blob or text data from a database.

@param aDatabase     A reference to the RSqlDatabase object that represents 
                     the database for which the SQL statement is being prepared.
@param aSqlStmt      A string of 16-bit wide characters containing the
                     SQL statement to be prepared.

@leave  KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the SQL statement is invalid, for example, the supplied
                      string contains more than one SQL statement, or it
                      contains an empty SQL statement.
                      Note that database specific errors categorised as ESqlDbError
                      can also be returned;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage().
		KErrPermissionDenied, the calling application does not satisfy the relevant database security policies.
                      Note that the function may leave with database specific errors categorised as ESqlDbError and
                      other system-wide error codes.

@capability None, if current RSqlStatement object represents a handle which operates on a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
@see RSqlBlobReadStream
@see RSqlBlobWriteStream
@see TSqlBlob
*/
EXPORT_C void RSqlStatement::PrepareL(RSqlDatabase& aDatabase, const TDesC& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexStrParam16, 
			&aDatabase, &aSqlStmt));
	
	__SQLLEAVE_IF_ERROR(Prepare(aDatabase, aSqlStmt));
	}
	
/**
Prepares the supplied 8-bit SQL statement for execution.

An RSqlStatement object can prepare and execute a parameterised SQL statement or
an SQL statement without parameters.

The function can only deal with one SQL statement at a time, i.e. if you
supply more than one SQL statement, each separated by a ";" character, then
the function returns an error.

Note that when the statement is to be used to retrieve or write blob or text data
that is over 2Mb in size then it is recommended that the RSqlBlobReadStream and 
RSqlBlobWriteStream classes or the TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of reading and writing large 
amounts of blob or text data from a database.

@param aDatabase     A reference to the RSqlDatabase object that represents 
                     the database for which the SQL statement is being prepared.
@param aSqlStmt      A string of 8-bit wide characters containing the
                     SQL statement to be prepared.


@leave  KErrNoMemory, an out of memory condition has occurred;
        KErrArgument, the SQL statement is invalid, for example, the supplied
                      string contains more than one SQL statement, or it
                      contains an empty SQL statement.
                      Note that database specific errors categorised as ESqlDbError
                      can also be returned;
        KSqlErrGeneral, a syntax error has occurred - text describing the problem
                        can be obtained by calling 	RSqlDatabase::LastErrorMessage().
		KErrPermissionDenied, the calling application does not satisfy the relevant database security policies.
                      Note that the function may leave with database specific errors categorised as ESqlDbError and
                      other system-wide error codes.

@capability None, if current RSqlStatement object represents a handle which operates on a non-secure database;
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement modifies a secure database schema; 
            RSqlSecurityPolicy::EReadPolicy or 
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement reads from a secure database; 
            RSqlSecurityPolicy::EWritePolicy or
            RSqlSecurityPolicy::ESchemaPolicy database policy type, if the SQL statement writes to a secure database;

@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase
@see RSqlDatabase::LastErrorMessage()
@see RSqlSecurityPolicy
@see RSqlSecurityPolicy::TPolicyType
@see RSqlBlobReadStream
@see RSqlBlobWriteStream
@see TSqlBlob
*/
EXPORT_C void RSqlStatement::PrepareL(RSqlDatabase& aDatabase, const TDesC8& aSqlStmt)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexStrParam, 
			&aDatabase, &aSqlStmt));

	__SQLLEAVE_IF_ERROR(Prepare(aDatabase, aSqlStmt));
	}


/**
Closes this SQL statement object.

The function frees memory and any allocated resources.

@see RSqlStatement::Prepare()

@capability None
*/	
EXPORT_C void RSqlStatement::Close()
	{
	SQLUTRACE_PROFILER(this);
	delete iImpl;
	iImpl = NULL;
	}

/**
Tests whether the SQL statement points to a valid record.

@return True, if the SQL statement points to a valid record, false otherwise.

@capability None
*/
EXPORT_C TBool RSqlStatement::AtRow() const
	{
	SQLUTRACE_PROFILER(this);
	return Impl().AtRow();
	}
	
/**
Resets the prepared SQL statement to its initial state and makes it ready to be
executed again.

Any SQL statement parameters that had values bound to them, retain their values.

If this object processes a parameterised SQL statement, then the parameter
values can be bound after the call to Reset().

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as of type ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().


Usage pattern 1:

@code
RSqlStatement stmt;
TInt err = stmt.Prepare(<database>, <SQL statement>);
while(<condition>)
	{
	err = stmt.Bind<parameter type>(<parameter index>, <value>);
	...
	err = stmt.Exec();
	....
	err = stmt.Reset();
	}
stmt.Close();
@endcode

Usage pattern 2:

@code
RSqlStatement stmt;
TInt err = stmt.Prepare(<database>, <SQL statement>);
while(<condition>)
	{
	err = stmt.Bind<parameter type>(<parameter index>, <value>);
	...
	while((err = stmt.Next()) == KSqlAtRow)
		{
		....
		}
	err = stmt.Reset();
	}
stmt.Close();
@endcode

@return KErrNone, the reset operation has completed successfully;			
        KSqlErrStmtExpired, the SQL statement has expired (if new functions or
                            collating sequences have been registered or if an
                            authorizer function has been added or changed)

@see TSqlRetCodeClass::ESqlDbError
@see RSqlDatabase::LastErrorMessage()

@capability None
*/	
EXPORT_C TInt RSqlStatement::Reset()
	{
	SQLUTRACE_PROFILER(this);
	return Impl().Reset();
	}
	
/**
Executes the prepared DDL/DML SQL statement. 

The function is very useful when the SQL statement contains parameters, because
the statement can be prepared once using RSqlStatement::Prepare(), and then
bound and executed many times.

Note that:
- parameter values must be bound before calling Exec().
- SQL statements that do not have parameters should use RSqlDatabase::Exec() instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as of type ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

Usage pattern:

@code
RSqlStatement stmt;
TInt err = stmt.Prepare(<database>, <SQL statement>);
while(<condition>)
	{
	err = stmt.Bind<parameter type>(<parameter index>, <value>);
	...
	err = stmt.Exec();
	....
	err = stmt.Reset();
	}
stmt.Close();
@endcode

@return >=0, The operation has completed successfully. The number of database rows that were 
			 changed/inserted/deleted by the most recently completed DDL/DML sql statement.
			 Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
			 if the operation has completed successfully (disregarding the number of the deleted rows);
        KSqlErrStmtExpired, the SQL statement has expired (if new functions or
                            collating sequences have been registered or if an
                            authorizer function has been added or changed);
    	KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE
    			  statement, try to use the reserved disk space (if there is a reserved disk space) to complete the operation.
    			  In all other cases the statement and database connection should be closed and some disk space freed 
    			  before reopening the database; 
        KErrNoMemory, an out of memory condition has occurred - the statement
                      will be reset.
                      Note that database specific errors categorised as ESqlDbError
                      can also be returned.

@see TSqlRetCodeClass::ESqlDbError
@see RSqlStatement::Prepare()
@see RSqlDatabase::Exec()
@see RSqlDatabase::LastErrorMessage()

@capability None
*/	
EXPORT_C TInt RSqlStatement::Exec()
	{
	SQLUTRACE_PROFILER(this);
	return Impl().Exec();
	}

/**
Executes the prepared DDL/DML SQL statement asynchronously to allow client to avoid being blocked 
by server activity.

No other operations can be performed on current RSqlStatement object 
until the asynchronous operation completes.

The function is very useful when the SQL statement contains parameters, because
the statement can be prepared once using RSqlStatement::Prepare(), and then
bound and executed many times.

Note that:
- parameter values must be bound before calling Exec().
- SQL statements that do not have parameters should use RSqlDatabase::Exec() instead.

If the call to this function fails because of a database-specific type error 
(i.e. the error is categorised as of type ESqlDbError), then a textual description of
the error can be obtained calling RSqlDatabase::LastErrorMessage().

@param aStatus Completion status of asynchronous request, one of the following:
	>=0, The operation has completed successfully. The number of database rows that were 
		changed/inserted/deleted by the most recently completed DDL/DML sql statement.
		Exception: If the executed statement is "DELETE FROM <table>", then the function returns 0 
		if the operation has completed successfully (disregarding the number of the deleted rows);
	KSqlErrStmtExpired, the SQL statement has expired (if new functions or 
						collating sequences have been registered or if an 
						authorizer function has been added or changed);
	KErrDiskFull, There is no available disk space to complete the operation. If the executed statement is a DELETE 
				statement, try to use the reserved disk space (if there is a reserved disk space) 
				to complete the operation.
				In all other cases the statement and database connection should be closed and some disk space freed 
				before reopening the database; 
	KErrNoMemory, an out of memory condition has occurred - the statement 
	will be reset.
	Note that aStatus may be set with database specific errors categorised as ESqlDbError,
	and other system-wide error codes.


@see TSqlRetCodeClass::ESqlDbError
@see RSqlStatement::Prepare()
@see RSqlDatabase::Exec()
@see RSqlDatabase::LastErrorMessage()

@capability None
*/	
EXPORT_C void RSqlStatement::Exec(TRequestStatus& aStatus)
	{
	SQLUTRACE_PROFILER(this);
	Impl().Exec(aStatus);
	}
	
/**
Retrieves a record.

If the prepared SQL statement is a "SELECT" statement, and is expected to
return a set of records, then this function can be used to retrieve that record data.

If the SQL statement contains parameters, then their values must be bound before
this function is called.

If the call to this function completes successfully, i.e. it returns
with KSqlAtRow, then this RSqlStatement object contains the record data, and 
this data will remain valid for access until another call is made to any
RSqlStatement function.

The record data can be accessed using the following functions:
- RSqlStatement::ColumnType()
- RSqlStatement::ColumnSize()
- RSqlStatement::ColumnInt()
- RSqlStatement::ColumnInt64()
- RSqlStatement::ColumnReal()
- RSqlStatement::ColumnTextL()
- RSqlStatement::ColumnText()
- RSqlStatement::ColumnBinary()
- RSqlStatement::ColumnBinaryL()

Note that if this call to Next() fails, as indicated by a return code value
other than KSqlAtRow, then calls to these RSqlStatement::Column...() functions
will raise a panic.

@return KSqlAtRow,      the record data is ready for processing by the caller;
        KSqlAtEnd,      there is no more record data;
        KSqlErrBusy,    the database file is locked;
        KErrNoMemory,   an out of memory condition has occurred - the statement
                        will be reset;
        KSqlErrGeneral, a run-time error has occured - this function must not
                        be called again;        
        KSqlErrMisuse,  this function has been called after a previous call
                        returned KSqlAtEnd or KSqlErrGeneral.
        KSqlErrStmtExpired, the SQL statement has expired (if new functions or
                            collating sequences have been registered or if an
                            authorizer function has been added or changed);

@capability None
*/	
EXPORT_C TInt RSqlStatement::Next()
	{
	SQLUTRACE_PROFILER(this);
	return Impl().Next();
	}

/**
Gets the index (starting from 0) of the parameter with the given name.

The function does a case insensitive parameter name search.

For example, if the parameter name is ":Prm", then the ":" prefix cannot
be omitted when you call ParameterIndex().

This function can be called at any time after the SQL statement has been prepared.

@param aParameterName The parameter name.

@return the parameter index value, if successful - this is a non-negative integer value;
        KErrNotFound, if no such parameter can be found.
        One of the other system-wide error codes may also be returned.

@capability None
*/	
EXPORT_C TInt RSqlStatement::ParameterIndex(const TDesC& aParameterName) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aParameterName));

	return Impl().ParamIndex(aParameterName);
	}

/**
Gets the number of columns that are to be returned by this SQL statement.

This function can be called at any time after the SQL statement has been prepared,
but it is useful only for SELECT statements. The column count of any other type of statement is always 0.

@return The number of columns.

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnCount() const
	{
	SQLUTRACE_PROFILER(this);
	return Impl().ColumnCount();
	}
	
/**
Gets the index (starting from 0) of the column with the given name.

The function does a case insensitive column name search.

This function can be called at any time after the SQL statement has been prepared.

@param aColumnName The column name.

@return the column index value, if successful - this is a non-negative integer value;
        KErrNotFound, if no such parameter can be found.
        One of the other system-wide error codes may also be returned.

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnIndex(const TDesC& aColumnName) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KStrParam16, 1, &aColumnName));

	return Impl().ColumnIndex(aColumnName);
	}
	
/**
Gets the runtime type of the column identified by the specified column index.

This function returns the actual runtime datatype of the specified column as 
opposed to its declared type.
  
Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return The column type.

@see KSqlAtRow
@see TSqlColumnType
@see RSqlStatement::DeclaredColumnType()
@see RSqlStatement::Next()
@see RSqlStatement::ColumnIndex()

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row.

@capability None
*/	
EXPORT_C TSqlColumnType RSqlStatement::ColumnType(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnType(aColumnIndex);
	}

/**
Gets the declared type of the column identified by the specified column index.

Note that the function can only be called when executing a SELECT query, and 
only after a successful call to Prepare(). 

This function returns the datatype that the specified column was originally declared to have.

The declared type of a column is determined according to the following rules:
@code
 - if the column type name contains the string "INT", then the declared column type is ESqlInt;
 - if the column type name contains any of the strings "CHAR, "TEXT" or "CLOB", then the declared column type is ESqlText;
 - if the column type name contains any of the strings "BLOB" or "BINARY", then the declared column type is ESqlBinary;
 - if the column type name contains any of the strings "FLOAT", "REAL" or "DOUBLE", then the declared column type is ESqlReal;
 - in all other cases the declared column type is assumed to be ESqlInt;
@endcode

@param aColumnIndex The index value identifying the column. This is 0 for the first column.
@param aColumnType	Output parameter. If the call completes successfully, aColumnType contains the type of the column, one of
					TSqlColumnType enum item values.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see TSqlColumnType
@see RSqlStatement::ColumnType()
@see RSqlStatement::ColumnIndex()

@panic SqlDb 5 Column index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::DeclaredColumnType(TInt aColumnIndex, TSqlColumnType& aColumnType) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().DeclaredColumnType(aColumnIndex, aColumnType);
	}
	
	
	
/**
Gets the length of the data for the column identified by the specified column index.

The length depends on the column type and is normally in bytes, except for
the case where the column type is ESqlText, in which case the function returns
the number of characters.

In detail, the following table shows the size associated with a column type.
Note that column types are identified by TSqlColumnType enum values.
@code
--------------------------------------------------------------
| Column type | Column Size                                 
--------------------------------------------------------------
| ESqlInt.....|.4                                             
| ESqlInt64...|.8                                             
| ESqlReal....|.8                                             
| ESqlText....|.the number of characters in the unicode string
| ESqlBinary..|.the byte length of the binary data            
| ESqlNull....|.0                                             
--------------------------------------------------------------
@endcode

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The column index value; this is 0 for the first column.

@return The size of the column, and depends on the type of column.

@see KSqlAtRow
@see TSqlColumnType
@see RSqlStatement::Next()
@see RSqlStatement::ColumnIndex()                

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnSize(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnSize(aColumnIndex);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////          Bind<Type>() implemenations           /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Sets the parameter to a NULL value.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.

@return KErrNone, the operation completed successfully.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()                

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindNull(TInt aParamIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aParamIndex));
	
	return Impl().BindNull(aParamIndex);
	}
	
/**
Sets the parameter to the specified 32-bit integer value.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aParamValue The 32-bit integer value to be assigned to the parameter.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()                

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindInt(TInt aParamIndex, TInt aParamValue)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aParamIndex));
	
	return Impl().BindInt(aParamIndex, aParamValue);
	}
	
/**
Sets the parameter to the specified 64-bit integer value.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aParamValue The 64-bit integer value to be assigned to the parameter.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()                

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindInt64(TInt aParamIndex, TInt64 aParamValue)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aParamIndex));
	
	return Impl().BindInt64(aParamIndex, aParamValue);
	}
	
/**
Sets the parameter to the specified 64-bit floating point value.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aParamValue The 64-bit floating point value to be assigned to the parameter.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()                

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindReal(TInt aParamIndex, TReal aParamValue)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aParamIndex));

	return Impl().BindReal(aParamIndex, aParamValue);
	}
	
/**
Sets the parameter to the specified 16-bit descriptor.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

Note that when the text data to be bound is over 2Mb in size then 
use of the RSqlBlobWriteStream or TSqlBlob class should be considered instead. 

These classes provide a more RAM-efficient way of writing large amounts of
text data to a database, however no conversions are performed on the text data -
it is simply stored as a stream of bytes. If the text data is part of a record 
to be inserted into a database then BindZeroBlob() should be called on the 
INSERT statement to create a placeholder for the text data, whose content 
can then be written using the above classes.

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aParamText  The 16-bit descriptor whose content is to be assigned to the parameter.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()
@see RSqlStatement::Next()                
@see RSqlStatement::Exec()  
@see RSqlStatement::BindZeroBlob()  
@see RSqlBlobWriteStream
@see TSqlBlob  

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindText(TInt aParamIndex, const TDesC& aParamText)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntSizeParam, 
			aParamIndex, aParamText.Length()));
	
	return Impl().BindText(aParamIndex, aParamText);
	}
	
/**
Sets the parameter to the specified 8-bit descriptor.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

Note that when the binary data to be bound is over 2Mb in size then it is 
recommended that the RSqlBlobWriteStream or TSqlBlob class is used instead. 

These classes provide a more RAM-efficient way of writing large amounts of
binary data to a database. If the binary data is part of a record to be inserted 
into a database then BindZeroBlob() should be called on the INSERT statement to create
a placeholder for the binary data, whose content can then be written using the above classes.

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aParamData  The 8-bit descriptor whose content is to be assigned to the parameter.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()
@see RSqlStatement::Next()                
@see RSqlStatement::Exec()
@see RSqlStatement::BindZeroBlob()  
@see RSqlBlobWriteStream
@see TSqlBlob  

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindBinary(TInt aParamIndex, const TDesC8& aParamData)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntSizeParam, 
			aParamIndex, aParamData.Length()));
	
	return Impl().BindBinary(aParamIndex, aParamData);
	}
	
/**
Binds a blob of length aBlobSize bytes that is filled with zeroes.

The parameter is identified by the specified index value.

A parameter value can be set:
- immediately after this SQL statement has been prepared
- after a call to Reset()

A zeroblob acts as a placeholder for a blob whose binary content is later written
using the RSqlBlobWriteStream or TSqlBlob class. 

Using zeroblobs provides a much more RAM-efficient way of creating large blobs than 
including the blob data in the INSERT statement and it is recommended for blobs that
are over 2Mb in size.

Note that a zeroblob should be created in a column after which there are no columns 
that contain anything other than zeroblobs or NULLs, otherwise the zeroblob must be 
allocated in full in RAM and its benefit is lost.

When creating a zeroblob it is recommended, where possible, to create the zeroblob 
and then write the blob content (using the RSqlBlobWriteStream or TSqlBlob class) 
within the same transaction. Otherwise the zeroblob will have to be journalled 
before being written to.

@param aParamIndex The index value identifying the parameter; this is 0 for the first parameter.
@param aBlobSize  The size in bytes of the blob.

@return KErrNone, the operation completed successfully;
        KErrNoMemory, an out of memory condition has occurred.
                  One of the other system-wide error codes may also be returned.

@see RSqlStatement::Prepare()
@see RSqlStatement::Reset()
@see RSqlStatement::Next()                
@see RSqlStatement::Exec()   
@see RSqlBlobWriteStream
@see TSqlBlob 

@panic SqlDb 5 Parameter index out of bounds.

@capability None
*/	
EXPORT_C TInt RSqlStatement::BindZeroBlob(TInt aParamIndex, TInt aBlobSize)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntSizeParam, 
			aParamIndex, aBlobSize));
	
	return Impl().BindZeroBlob(aParamIndex, aBlobSize);
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////          Column<Type>() implemenations           ///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Tests whether the value of the specified column is NULL.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return True, if the value of the column is NULL, false otherwise.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()                

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TBool RSqlStatement::IsNull(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnType(aColumnIndex) == ESqlNull;
	}

/**
Gets the value of the column as a 32-bit integer.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return The value of the column as a 32-bit integer.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()                

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnInt(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnInt(aColumnIndex);
	}
	
/**
Gets the value of the column as a 64-bit integer.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return The value of the column as a 64-bit integer.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()                

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt64 RSqlStatement::ColumnInt64(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnInt64(aColumnIndex);
	}
	
/**
Gets the value of the column as a 64-bit floating point value.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return The value of the column as a 64-bit floating point value.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()                

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TReal RSqlStatement::ColumnReal(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnReal(aColumnIndex);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////          ColumnText ()           ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Gets the value of the column as a 16-bit descriptor (leaves on failure).

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the text to be retrieved is over 2Mb in size then it is 
recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of text data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return A non-modifiable pointer descriptor representing the 16-bit column text.

@leave KErrNoMemory if the operation for retrieving the column value from the server
                    fails with an out of memory condition.

@see KSqlAtRow
@see RSqlStatement::ColumnText()
@see RSqlStatement::Prepare()
@see RSqlStatement::Next() 
@see RSqlBlobReadStream
@see TSqlBlob 

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/
EXPORT_C TPtrC RSqlStatement::ColumnTextL(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	TPtrC res;
	__SQLLEAVE_IF_ERROR(Impl().ColumnText(aColumnIndex, res));
	return res;
	}

/**
Gets the value of the column as a 16-bit descriptor.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the text to be retrieved is over 2Mb in size then it is 
recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of text data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.
@param aPtr         A non-modifiable pointer descriptor. On successful completion of this
                    function, the pointer descriptor represents the 16-bit column text.
                    The descriptor does not change if the function fails.

@return KErrNone, if the function completes successfully,
                  otherwise one of the other system-wide error codes.

@see KSqlAtRow
@see RSqlStatement::ColumnTextL()
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()   
@see RSqlBlobReadStream
@see TSqlBlob

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnText(TInt aColumnIndex, TPtrC& aPtr) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnText(aColumnIndex, aPtr);
	}
	
/**
Interprets the value of the column as a 16-bit descriptor, and copies the data
into a 16-bit modifiable descriptor supplied by the caller.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the text to be retrieved is over 2Mb in size then it is 
recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of text data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.
@param aDest        A 16-bit modifiable descriptor into which the column data is to be copied.

@return KErrNone,     the operation has completed successfully;
        KErrOverflow, the maximum length of the target descriptor supplied by
                      the caller (aDest) is less than the length of
                      the column text - the column data
                      is truncated to fit into the target descriptor.
        KErrNoMemory, an out of memory condition has occurred.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()   
@see RSqlBlobReadStream
@see TSqlBlob

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnText(TInt aColumnIndex, TDes& aDest) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	return Impl().ColumnText(aColumnIndex, aDest);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////          ColumnBinary ()           /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
	
/**
Gets the value of the column as an 8-bit descriptor (leaves on failure).

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the binary data to be retrieved is over 2Mb in size then it 
is recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of binary data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return A non-modifiable pointer descriptor representing the 8-bit column data.

@leave KErrNoMemory if the operation for retrieving the column value from the server
                    fails with an out of memory condition.

@see KSqlAtRow
@see RSqlStatement::ColumnBinary()
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()   
@see RSqlBlobReadStream
@see TSqlBlob

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/
EXPORT_C TPtrC8 RSqlStatement::ColumnBinaryL(TInt aColumnIndex) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

	TPtrC8 res;
	__SQLLEAVE_IF_ERROR(Impl().ColumnBinary(aColumnIndex, res));
	return res;
	}

/**
Gets the value of the column as an 8-bit descriptor.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the binary data to be retrieved is over 2Mb in size then it 
is recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of binary data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.
@param aPtr         A non-modifiable pointer descriptor. On successful completion of this
                    function, the pointer descriptor represents the 8-bit column data.
                    The descriptor does not change if the function fails.

@return KErrNone, if the function completes successfully,
                  otherwise one of the other system-wide error codes.

@see KSqlAtRow
@see RSqlStatement::ColumnBinaryL()
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()   
@see RSqlBlobReadStream
@see TSqlBlob

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnBinary(TInt aColumnIndex, TPtrC8& aPtr) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));

   	return Impl().ColumnBinary(aColumnIndex, aPtr);
	}
	
/**
Interprets the value of the column as an 8-bit descriptor, and copies the data
into an 8-bit modifiable descriptor supplied by the caller.

The column is identified by the specified index value.

Note that the function can only be called after a successful call to Next(),
i.e. after a call to Next() that has completed with a KSqlAtRow return code.
Calling this function after an unsuccessful call to Next() raises a panic.

Note that when the binary data to be retrieved is over 2Mb in size then it 
is recommended that the RSqlBlobReadStream or TSqlBlob class is used instead. 
These classes provide a more RAM-efficient way of retrieving large amounts 
of binary data from a database.

@param aColumnIndex The index value identifying the column; this is 0 for the first column.
@param aDest        An 8-bit modifiable descriptor into which the column data is to be copied.

@return KErrNone,     the operation has completed successfully;
        KErrOverflow, the maximum length of the target descriptor supplied by
                      the caller (aDest) is less than the length of
                      the column data - the column data
                      is truncated to fit into the target descriptor.
        KErrNoMemory, an out of memory condition has occurred.

@see KSqlAtRow
@see RSqlStatement::Prepare()
@see RSqlStatement::Next()   
@see RSqlBlobReadStream
@see TSqlBlob

@panic SqlDb 5 Column index out of bounds.
@panic SqlDb 11 Statement cursor not positioned on a row

@capability None
*/	
EXPORT_C TInt RSqlStatement::ColumnBinary(TInt aColumnIndex, TDes8& aDest) const
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));
	
   	return Impl().ColumnBinary(aColumnIndex, aDest);
	}

/**
Obtain the name of a column after preparing a query.

@param aColumnIndex Column index
@param aNameDest Descriptor which will be set to column name
@return KErrNone if successfull or one of the system-wide error codes on error
@capability None
*/
EXPORT_C TInt RSqlStatement::ColumnName(TInt aColumnIndex, TPtrC& aNameDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aColumnIndex));
	
	return Impl().ColumnName(aColumnIndex, aNameDest);
	}

/**
Obtain the name of a parameter after preparing a DML query.
The parameter names are returned in exactly the same form as
supplied in SQL statement. For example, if the parameter name is ":Prm", 
then the ":" prefix will not be omitted.

This function can be called at any time after the DML SQL statement has been prepared.

@param aParameterIndex Parameter index
@param aNameDest Descriptor which will be set to column name
@return KErrNone if successfull or one of the system-wide error codes on error
@capability None
*/
EXPORT_C TInt RSqlStatement::ParameterName(TInt aParameterIndex, TPtrC& aNameDest)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KIntParam, 1, aParameterIndex));
	
	return Impl().ParameterName(aParameterIndex, aNameDest);
	}

/**
Obtain the name of a parameter after preparing a DML query.
The parameter names are returned in exactly the same form as
supplied in SQL statement. For example, if the parameter name is ":Prm", 
then the ":" prefix will not be omitted.

ParamName has the same behaviour as ParameterName. It is provided to maintain
source compatibility with previous Symbian releases.

This function can be called at any time after the DML SQL statement has 
been prepared.

@param aParameterIndex Parameter index
@param aNameDest Descriptor which will be set to parameter name
@return KErrNone if successful or one of the system-wide error codes on error
@capability None
*/
EXPORT_C TInt RSqlStatement::ParamName(TInt aParameterIndex, TPtrC& aNameDest)
	{
	return ParameterName(aParameterIndex, aNameDest);
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
	
/**
Returns a reference to the implementation object of RSqlStatement - CSqlStatementImpl.

@panic SqlDb 2  The SQL statement has not been prepared,
                i.e. Prepare() has not yet been called on this RSqlStatement object.

@internalComponent
*/
CSqlStatementImpl& RSqlStatement::Impl() const
	{
	__SQLASSERT_ALWAYS(iImpl != NULL, ESqlPanicInvalidObj);
	return *iImpl;	
	}
