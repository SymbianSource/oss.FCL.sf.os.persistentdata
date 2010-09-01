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

#include <sqldb.h>					//RSqlColumnReadStream, RSqlParamWriteStream
#include "SqlStatementImpl.h"		//CSqlStatementImpl

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////         RSqlColumnReadStream              ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Gives access to column data as a read-only stream of characters,

The function can only be used for text and binary column types.

@param aStmt        The RSqlStatement object to which the referred column belongs.
@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return KErrNone, the text column data stream has been opened successfully;
        KErrNoMemory, an out of memory condition occurred;
        KErrArgument, the column type is neither text nor binary.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C TInt RSqlColumnReadStream::ColumnText(RSqlStatement& aStmt, TInt aColumnIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aColumnIndex));
	
	TRAPD(err, Attach(aStmt.Impl().ColumnSourceL(aColumnIndex)));
	return err;
	}
	
/**
Gives access to column data as a read-only stream of bytes.

The function can only be used for text and binary column types.

@param aStmt        The RSqlStatement object to which the referred column belongs.
@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@return KErrNone, the text column data stream has been opened successfully;
        KErrNoMemory, an out of memory condition occurred;
        KErrArgument, the column type is neither text nor binary.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C TInt RSqlColumnReadStream::ColumnBinary(RSqlStatement& aStmt, TInt aColumnIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aColumnIndex));
	
	TRAPD(err, Attach(aStmt.Impl().ColumnSourceL(aColumnIndex)));
	return err;
	}

/**
Gives access to column data as a read-only stream of characters,

The function can only be used for text and binary column types.

@param aStmt        The RSqlStatement object to which the referred column belongs.
@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@leave  KErrNoMemory, an out of memory condition occurred;
        KErrArgument, the column type is neither text nor binary.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C void RSqlColumnReadStream::ColumnTextL(RSqlStatement& aStmt, TInt aColumnIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aColumnIndex));
	
	Attach(aStmt.Impl().ColumnSourceL(aColumnIndex));
	}
	
/**
Gives access to column data as a read-only stream of bytes.

The function can only be used for text and binary column types.

@param aStmt        The RSqlStatement object to which the referred column belongs.
@param aColumnIndex The index value identifying the column; this is 0 for the first column.

@leave  KErrNoMemory, an out of memory condition occurred;
        KErrArgument, the column type is neither text nor binary.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@capability None
*/
EXPORT_C void RSqlColumnReadStream::ColumnBinaryL(RSqlStatement& aStmt, TInt aColumnIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aColumnIndex));

	Attach(aStmt.Impl().ColumnSourceL(aColumnIndex));
	}

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////         RSqlParamWriteStream              ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Gives access to parameter data as a stream of characters.

NB You need to close the stream after the execution of the statement operation for
which the parameter is set (RSqlStatement::Next() or RSqlStatement::Exec()).

@param aStmt           The RSqlStatement object to which the referred parameter belongs.
@param aParameterIndex The index value identifying the parameter; this is 0 for the first parameter.

@return KErrNone, the binary parameter data stream has been opened successfully;
        KErrNoMemory, an out of memory condition occurred.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see RSqlStatement::Next()
@see RSqlStatement::Exec()

@capability None
*/
EXPORT_C TInt RSqlParamWriteStream::BindText(RSqlStatement& aStmt, TInt aParameterIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aParameterIndex));

	TRAPD(err, Attach(aStmt.Impl().ParamSinkL(ESqlSrvStmtTxtParamSink16, aParameterIndex)));
	return err;
	}
	
/**
Gives access to parameter data as a stream of bytes.

NB You need to close the stream after the execution of the statement operation for
which the parameter is set (RSqlStatement::Next() or RSqlStatement::Exec()).

@param aStmt           The RSqlStatement object to which the referred parameter belongs.
@param aParameterIndex The index value identifying the parameter; this is 0 for the first parameter.

@return KErrNone, the binary parameter data stream has been opened successfully;
        KErrNoMemory, an out of memory condition occurred.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see RSqlStatement::Next()
@see RSqlStatement::Exec()

@capability None
*/
EXPORT_C TInt RSqlParamWriteStream::BindBinary(RSqlStatement& aStmt, TInt aParameterIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aParameterIndex));

	TRAPD(err, Attach(aStmt.Impl().ParamSinkL(ESqlSrvStmtBinParamSink, aParameterIndex)));
	return err;
	}

/**
Gives access to parameter data as a stream of characters.

NB You need to close the stream after the execution of the statement operation for
which the parameter is set (RSqlStatement::Next() or RSqlStatement::Exec()).

@param aStmt           The RSqlStatement object to which the referred parameter belongs.
@param aParameterIndex The index value identifying the parameter; this is 0 for the first parameter.

@leave KErrNoMemory, an out of memory condition occurred.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see RSqlStatement::Next()
@see RSqlStatement::Exec()

@capability None
*/
EXPORT_C void RSqlParamWriteStream::BindTextL(RSqlStatement& aStmt, TInt aParameterIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aParameterIndex));

	Attach(aStmt.Impl().ParamSinkL(ESqlSrvStmtTxtParamSink16, aParameterIndex));
	}
	
/**
Gives access to parameter data as a stream of bytes.

NB You need to close the stream after the execution of the statement operation for
which the parameter is set (RSqlStatement::Next() or RSqlStatement::Exec()).

@param aStmt           The RSqlStatement object to which the referred parameter belongs.
@param aParameterIndex The index value identifying the parameter; this is 0 for the first parameter.

@leave KErrNoMemory, an out of memory condition occurred.
                      Note that database specific errors categorised as ESqlDbError, and
                      other system-wide error codes may also be returned.

@see RSqlStatement::Next()
@see RSqlStatement::Exec()

@capability None
*/
EXPORT_C void RSqlParamWriteStream::BindBinaryL(RSqlStatement& aStmt, TInt aParameterIndex)
	{
	SQLUTRACE_PROFILER(this);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KHexIntParam, &aStmt, aParameterIndex));

	Attach(aStmt.Impl().ParamSinkL(ESqlSrvStmtBinParamSink, aParameterIndex));
	}

