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
// NTT DOCOMO, INC - Fix for Bug 1915 "SQL server panics when using long column type strings"
//
// Description:
//

#include <s32mem.h>
#include "SqlStmtSession.h"		//RSqlStatementSession

/**
Sends a request to the server to close the statement handle.
Closes the session object.
*/
void RSqlStatementSession::Close()
	{
	if(iDbSession && iHandle > 0)
		{
		(void)iDbSession->SendReceive(::MakeMsgCode(ESqlSrvStmtClose, ESqlSrvStatementHandle, iHandle));
		}
	iDbSession = NULL;
	iHandle = -1;
	}

/**
Binds the statement parameters and sends a request to the SQL server to move to the next record which satisfies the 
condition of the prepared SQL statement.
If there is a valid next record, the method transfers the column values from the server.

@param aParamBuf  It references RSqlBufFlat object where the parameter values are stored.
@param aColumnBuf It references RSqlBufFlat object where the column values will be stored.

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
*/
TInt RSqlStatementSession::BindNext(const RSqlBufFlat& aParamBuf, RSqlBufFlat& aColumnBuf)
	{
	TPtrC8 prmData(aParamBuf.BufDes());
	TIpcArgs ipcArgs;
	ipcArgs.Set(0, prmData.Length());
	ipcArgs.Set(1, &prmData);
	return DoBindNext(ESqlSrvStmtBindNext, ipcArgs, aColumnBuf);
	}

/**
Implements RSqlStatementSession::Next() and RSqlStatementSession::BindNext().
Sends a "Next" command to the server combined with optional "Bind" command.
In a single IPC call the statement parameters will be bound and the current row columns - returned.
If the client side flat buffer is not big enough, a second IPC call will be made after reallocating the buffer.

Usage of the IPC call arguments:
Arg 0: [out]		parameter buffer length in bytes
Arg 1: [out]		parameter buffer
Arg 2: [out]		column buffer length in bytes
Arg 3: [in/out]		column buffer

@see RSqlStatementSession::Next()
@see RSqlStatementSession::BindNext()
*/
TInt RSqlStatementSession::DoBindNext(TSqlSrvFunction aFunction, TIpcArgs& aIpcArgs, RSqlBufFlat& aColumnBuf)
	{
	aColumnBuf.Reset();
	aIpcArgs.Set(2, aColumnBuf.MaxSize());
	aIpcArgs.Set(3, &aColumnBuf.BufPtr());
	TInt err = DbSession().SendReceive(::MakeMsgCode(aFunction, ESqlSrvStatementHandle, iHandle), aIpcArgs);
	if(err > KSqlClientBufOverflowCode)
		{
		err = Retry(aColumnBuf, err - KSqlClientBufOverflowCode, ESqlColumnValuesBuf);
		if(err == KErrNone)
			{
			err = KSqlAtRow;	
			}
		}
	return err;
	}

/**
Sends a command to the server for retrieving parameter names or column names.

Usage of the IPC call arguments:
Arg 0: [out]		buffer length in bytes
Arg 1: [in/out]		buffer
*/	
TInt RSqlStatementSession::GetNames(TSqlSrvFunction aFunction, RSqlBufFlat& aNameBuf)
	{
	aNameBuf.Reset();
	TPtr8& ptr = aNameBuf.BufPtr();
	TInt err = DbSession().SendReceive(::MakeMsgCode(aFunction, ESqlSrvStatementHandle, iHandle), TIpcArgs(ptr.MaxLength(), &ptr));
	if(err > KSqlClientBufOverflowCode)
		{
		err = Retry(aNameBuf, err - KSqlClientBufOverflowCode, aFunction == ESqlSrvStmtColumnNames ? ESqlColumnNamesBuf : ESqlParamNamesBuf);
		}
	return err;
	}

/**
Sends a command to the server for retrieving specified data (aWhat parameter).

Usage of the IPC call arguments:
Arg 0: [out]		The type of the data to be retrieved
Arg 1: [in/out]		Data buffer
*/	
TInt RSqlStatementSession::Retry(RSqlBufFlat& aBufFlat, TInt aSize, TSqlBufFlatType aWhat)
	{
	aBufFlat.Reset();
	TInt err = aBufFlat.ReAlloc(aSize);
	if(err == KErrNone)
		{
		TPtr8& ptr = aBufFlat.BufPtr();
		err = DbSession().SendReceive(::MakeMsgCode(ESqlSrvStmtBufFlat, ESqlSrvStatementHandle, iHandle), TIpcArgs(aWhat, &ptr));
		}
	return err;	
	}

/**
Sends a command to the server for retrieving the declared types of columns

Usage of the IPC call arguments:
Arg 0: [out]		buffer length in bytes
Arg 1: [in/out]		buffer
*/	 
TInt RSqlStatementSession::GetDeclColumnTypes(RSqlBufFlat& aDeclColumnTypeBuf)
	{
	aDeclColumnTypeBuf.Reset();
	TPtr8& ptr = aDeclColumnTypeBuf.BufPtr();
	TInt err = DbSession().SendReceive(::MakeMsgCode(ESqlSrvStmtDeclColumnTypes, ESqlSrvStatementHandle, iHandle), TIpcArgs(ptr.MaxLength(), &ptr));	
	if(err > KSqlClientBufOverflowCode)
		{
		err = Retry(aDeclColumnTypeBuf, err - KSqlClientBufOverflowCode, ESqlDeclColumnTypesBuf);
		}
	return err;	
	}
