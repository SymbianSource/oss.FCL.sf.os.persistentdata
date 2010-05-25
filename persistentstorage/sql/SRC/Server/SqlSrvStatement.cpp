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

#include <utf.h>					//CnvUtfConverter
#include <e32math.h>
#include "SqliteSymbian.h"			//sqlite3SymbianLastOsError()
#include "sqlite3.h"
#include "SqlSrvStatement.h"
#include "SqlBufIterator.h"			//TSqlBufRIterator
#include "SqlSrvResourceProfiler.h"
#include "UTraceSql.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////      local const data       ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

//This is the name prefix which will be given to the nameless parameters.
//For example, if the SQL string is:
//   SELECT * FROM A WHERE ColA1 = ? AND ColA2 = ?
//then the names which will be given to the parameters will be:
//"?0" and "?1"
_LIT(KNamelessParameter, "?");


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////                HSqlSrvStmtParamBuf                  //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Destroys the parameter buffer.

Virtual method.
*/
HSqlSrvStmtParamBuf::~HSqlSrvStmtParamBuf()
	{
	delete iBuf;
	}

/**
Binds the parameter value.
The buffer can be synch-ed if:
 - this is the first synch operation;
 - the bound statement object is still alive (not finalized);
 - the current object is alive;
 - the current object data is retrieved from an IPC stream;
 
If none of the conditions above is true, the synch operation is no-op.

Virtual method.
*/
void HSqlSrvStmtParamBuf::DoSynchL()
	{
	if(iSynchDone || !iAlive || iStatementFinalized || iBufType != HSqlSrvStmtParamBuf::EBufIpcStream)
		{
		return;	
		}
	iSynchDone = ETrue;
	TBufBuf::DoSynchL();
	iStatement.BindParamBufL(iParamIndex);
	}

/**
Destroys the HSqlSrvStmtParamBuf instance.
This method is a no-op if the statement is not finalized yet.

Virtual method.
*/
void HSqlSrvStmtParamBuf::DoRelease()
 	{
	iAlive = EFalse;
 	if(iStatementFinalized)
 		{//The bound statement has been finalized - destroy the current object then.
 		delete this;
 		}
	}

/**
This function is called by the bound statement object to notify the current HSqlSrvStmtParamBuf object that the
bound statement is about to be finalized. That means, when the "stream close" operation on the client side 
makes an attempt to synch the HSqlSrvStmtParamBuf object, no attempt should be made to bound the parameter data,
because the statement object is gone.
After this call the bound statement objects ceases to exist.

Actions, performed by this method:
 - if the buffer type is "simple bind", the buffer will be destroyed. No reason to keep it alive, there is no bound IPC
   stream object on the client side;
 - if the buffer type is an IPC stream buffer and the buffer is alive, that means: the bound statement object is about to be
   finalized, but there is a bound client side IPC stream object that is still alive. In this case the buffer won't be destroyed,
   but will be "told" that the bound statement is finalized, so when the client side IPC stream is closed, this object will get destroyed;
 - if the buffer type is an IPC stream buffer and the buffer is "dead", that means there is no bound IPC stream object on the client
   side and it is safe to destroy the buffer;

*/ 	
void HSqlSrvStmtParamBuf::NotifyStatementFinalized()
	{
	iStatementFinalized = ETrue;
	if(iBufType == HSqlSrvStmtParamBuf::EBufSimpleBind || !iAlive)
		{
		DoRelease();	
		}
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////   CSqlSrvStatement class    ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Creates a new CSqlSrvStatement instance.

The created CSqlSrvStatement instance will be placed in the cleanup stack.

@param aDbHandle The database handle
@param aSqlStmt 16-bit SQL statement, zero-terminated string
@param aColumnCount Output parameter. It will be initialized with the column count.
@param aParamCount Output parameter. It will be initialized with the parameter count.

@return A pointer to the created CSqlSrvStatement instance.

@leave KErrNoMemory, an out of memory condition has occurred;
	   KErrArgument, bad argument, for example - the SQL string contains more than one SQL statements.
                  Note that the function may also leave with some other database specific 
                  errors categorised as ESqlDbError.

@panic SqlDb 4 In _DEBUG mode if aSqlStmt is not zero-terminated string.
*/	
CSqlSrvStatement* CSqlSrvStatement::NewLC(sqlite3* aDbHandle, const TDesC16& aSqlStmt, TInt& aColumnCount, TInt& aParamCount)
	{
	__SQLASSERT(aSqlStmt.Length() > 0 ? (TInt)aSqlStmt[aSqlStmt.Length() - 1] == 0 : ETrue, ESqlPanicBadArgument);
	
	CSqlSrvStatement* self = new (ELeave) CSqlSrvStatement;
	CleanupStack::PushL(self);
	self->ConstructL(aDbHandle, aSqlStmt);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KSrvStmtCreated, self));
	aColumnCount = self->iColumnCount;
	aParamCount = self->iParamCount;
	return self;
	}
	
/**
Creates a new CSqlSrvStatement instance.

The created CSqlSrvStatement instance will be placed in the cleanup stack.

@param aDbHandle The database handle
@param aSqlStmt 8-bit SQL statement, zero-terminated string
@param aColumnCount Output parameter. It will be initialized with the column count.
@param aParamCount Output parameter. It will be initialized with the parameter count.

@return A pointer to the created CSqlSrvStatement instance.

@leave KErrNoMemory, an out of memory condition has occurred;
	   KErrArgument, bad argument, for example - the SQL string contains more than one SQL statements.
                  Note that the function may also leave with some other database specific 
                  errors categorised as ESqlDbError.

@panic SqlDb 4 In _DEBUG mode if aSqlStmt is not zero-terminated string.
*/	
CSqlSrvStatement* CSqlSrvStatement::NewLC(sqlite3* aDbHandle, const TDesC8& aSqlStmt, TInt& aColumnCount, TInt& aParamCount)
	{
	__SQLASSERT(aSqlStmt.Length() > 0 ? (TInt)aSqlStmt[aSqlStmt.Length() - 1] == 0 : ETrue, ESqlPanicBadArgument);

	CSqlSrvStatement* self = new (ELeave) CSqlSrvStatement;
	CleanupStack::PushL(self);
	self->ConstructL(aDbHandle, aSqlStmt);
	SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KSrvStmtCreated, self));
	aColumnCount = self->iColumnCount;
	aParamCount = self->iParamCount;
	return self;
	}
	
/**
Destroys the allocated by CSqlSrvStatement instance memory and other resources.
*/	
CSqlSrvStatement::~CSqlSrvStatement()
	{
	DestroyParamBufArray();
	iBufFlat.Close();
	if(iStmtHandle)
		{
#ifdef SYMBIAN_TRACE_SQL_EVENTS
		TInt scanCount = sqlite3_stmt_status(iStmtHandle, SQLITE_STMTSTATUS_FULLSCAN_STEP, ETrue);
		TInt sortCount = sqlite3_stmt_status(iStmtHandle, SQLITE_STMTSTATUS_SORT, ETrue);
		SYMBIAN_TRACE_SQL_EVENTS_ONLY(UTF::Printf(UTF::TTraceContext(UTF::EInternals), KSrvStmtStatus, this, scanCount, sortCount));
#endif
		(void)sqlite3_finalize(iStmtHandle);
		}
	}

/**
Sets SQL statement parameter values.

Only parameters, whose values are set by the client, will be processed.

@param aParamBuf Flat buffer with parameter values.

@leave KErrArgument, unknown parameter type;
	   KSqlErrStmtExpired, statement handle expired.
                  Note that the function may also leave with some other database specific 
                  errors categorised as ESqlDbError.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
*/	
void CSqlSrvStatement::BindL(const RSqlBufFlat& aParamBuf)
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	
	(void)sqlite3SymbianLastOsError();//clear last OS error

	TSqlBufRIterator it;
	it.Set(aParamBuf);
	TInt prmIdx = 0;
	
	TInt err = SQLITE_OK;
	while(it.Next() && err == SQLITE_OK)
		{
		++prmIdx;//the first SQLITE parameter index is 1
		if(it.IsPresent())
			{
			switch(it.Type())
 				{
				case ESqlInt:
					err = sqlite3_bind_int(iStmtHandle, prmIdx, it.Int());
					break;
				case ESqlInt64:
					err = sqlite3_bind_int64(iStmtHandle, prmIdx, it.Int64());
					break;
				case ESqlReal: 
					err = sqlite3_bind_double(iStmtHandle, prmIdx, it.Real());
					break;
				case ESqlText: 
					//SQLITE_STATIC is used as an argument, because the text data will be kept and can be used by the next bind call
					{
					TPtrC text = it.Text();
					TPtrC8 prmDataCopy(reinterpret_cast <const TUint8*> (text.Ptr()), text.Length() * sizeof(TUint16));
					prmDataCopy.Set(CopyAndStoreParamL(prmIdx - 1, HSqlSrvStmtParamBuf::EText16, prmDataCopy));
					err = sqlite3_bind_text16(iStmtHandle, prmIdx, prmDataCopy.Ptr(), prmDataCopy.Length(), SQLITE_STATIC);
					}
					break;
				case ESqlBinary:
					//SQLITE_STATIC is used as an argument, because the blob data will be kept and can be used by the next bind call
					{
					TPtrC8 prmDataCopy = CopyAndStoreParamL(prmIdx - 1, HSqlSrvStmtParamBuf::EBinary, it.Binary());
					err = sqlite3_bind_blob(iStmtHandle, prmIdx, prmDataCopy.Ptr(), prmDataCopy.Length(), SQLITE_STATIC);
					}
					break;
				case ESqlNull:
					err = sqlite3_bind_null(iStmtHandle, prmIdx);
					break;
				case ESqlZeroBlob:
					err = sqlite3_bind_zeroblob(iStmtHandle, prmIdx, it.Int());
					break;
				default:
					__SQLLEAVE(KErrArgument);//unknown parameter type
					break;
				}
			}//end of - if(it.IsPresent())
		}//end of - while(it.Next() && err == SQLITE_OK)
	err = ::Sql2OsErrCode(err, sqlite3SymbianLastOsError());
	__SQLLEAVE_IF_ERROR(err);
	}

/**
Collects column names in a flat buffer and returns a reference to the buffer. 

@return A const reference to a flat buffer containing the column names.

@leave KErrNoMemory, an out of memory condition has occurred.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
*/	
const RSqlBufFlat& CSqlSrvStatement::ColumnNamesL()
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	iBufFlatType = static_cast <TSqlBufFlatType> (-1);
	__SQLLEAVE_IF_ERROR(iBufFlat.SetCount(iColumnCount));
	TSqlBufWIterator it;
	it.Set(iBufFlat);
	TInt colIdx = -1;
	while(it.Next())
		{
		++colIdx;//the first SQLITE column index is 0
		const TUint16* name = reinterpret_cast <const TUint16*> (__SQLLEAVE_IF_NULL(const_cast <void*> (sqlite3_column_name16(iStmtHandle, colIdx))));
		TPtrC ptr(name, User::StringLength(name));
		__SQLLEAVE_IF_ERROR(it.SetText(ptr));
		}
	iBufFlatType = ESqlColumnNamesBuf;
	SQLPROFILER_REPORT_ALLOC(iBufFlat.MaxSize());
	return iBufFlat;
	}

/**
Collects parameter names in a flat buffer and returns a reference to the buffer. 

@return A const reference to a flat buffer containing the parameter names.

@leave KErrNoMemory, an out of memory condition has occurred.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
*/	
const RSqlBufFlat& CSqlSrvStatement::ParamNamesL()
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	iBufFlatType = static_cast <TSqlBufFlatType> (-1);
	__SQLLEAVE_IF_ERROR(iBufFlat.SetCount(iParamCount));
	TSqlBufWIterator it;
	it.Set(iBufFlat);
	TInt prmIdx = 0;
	while(it.Next())
		{
		++prmIdx;//the first SQLITE parameter index is 1
		const TUint8* name8 = reinterpret_cast <const TUint8*> (sqlite3_bind_parameter_name(iStmtHandle, prmIdx));
		if(name8)
			{
			HBufC* name = CnvUtfConverter::ConvertToUnicodeFromUtf8L(TPtrC8(name8, User::StringLength(name8)));
			TInt err = it.SetText(name->Des());
			delete name;
			__SQLLEAVE_IF_ERROR(err);
			}
		else //nameless parameter case
			{
			//The parameter name in this case will be formatted as "?<num>", where <num> is the parameter index.
			TBuf<5> prmName;
			prmName.Append(KNamelessParameter);
			prmName.AppendNum((TInt64)(prmIdx - 1));
			__SQLLEAVE_IF_ERROR(it.SetText(prmName));
			}
		}
	iBufFlatType = ESqlParamNamesBuf;
	SQLPROFILER_REPORT_ALLOC(iBufFlat.MaxSize());
	return iBufFlat;
	}
	
/**
Collects the column values in a flat buffer and returns a reference to the buffer. 

@leave KErrNoMemory, an out of memory condition has occurred.

@return A const reference to a flat buffer containing the column values.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object
*/	
const RSqlBufFlat& CSqlSrvStatement::ColumnValuesL()
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	
	iBufFlatType = static_cast <TSqlBufFlatType> (-1);
	iBufFlat.SetCount(iColumnCount);
	TSqlBufWIterator it;
	it.Set(iBufFlat);
	TInt colIdx = -1;
	
	while(it.Next())
		{
		++colIdx;//the first SQLITE column index is 0
		TInt colType = sqlite3_column_type(iStmtHandle, colIdx);
		switch(colType)
			{
			case SQLITE_INTEGER:
				{
				TInt64 val = sqlite3_column_int64(iStmtHandle, colIdx);
				__SQLLEAVE_IF_ERROR(val == TInt64(TInt32(val)) ? it.SetInt(static_cast <TInt> (val)) : it.SetInt64(val));
				}
				break;
			case SQLITE_FLOAT:
				__SQLLEAVE_IF_ERROR(it.SetReal(sqlite3_column_double(iStmtHandle, colIdx)));
				break;
			case SQLITE_TEXT:
				{
				TInt charLength = (TUint)sqlite3_column_bytes16(iStmtHandle, colIdx) / sizeof(TUint16);
                //"charLength == 0" - this might be an indication of an "out of memory" problem, if the column text is in UTF8 format. 
                //(sqlite3_column_bytes16() may allocate memory for UTF8->UTF16 conversion)
				if(charLength == 0 && sqlite3_errcode(sqlite3_db_handle(iStmtHandle)) == SQLITE_NOMEM)
                    {
                    __SQLLEAVE(KErrNoMemory);
                    }
				if(charLength >= KSqlMaxDesLen)
					{
					it.SetAsNotPresent(ESqlText, charLength);
					}
				else
					{//sqlite3_column_bytes16() already allocated the needed memory if a UTF8->UTF16 conversion
                     //had to be performed. The sqlite3_column_text16() on the next line is guaranteed to succeed.
					const TUint16* text = reinterpret_cast <const TUint16*> (sqlite3_column_text16(iStmtHandle, colIdx));
					__SQLASSERT(text != NULL, ESqlPanicInternalError);
					__SQLLEAVE_IF_ERROR(it.SetText(TPtrC16(text, charLength)));
					}
				}
				break;
			case SQLITE_BLOB:
				{
				TInt byteLength = sqlite3_column_bytes(iStmtHandle, colIdx);
				if(byteLength >= KSqlMaxDesLen)
					{
					it.SetAsNotPresent(ESqlBinary, byteLength);
					}
				else
					{
					__SQLLEAVE_IF_ERROR(it.SetBinary(TPtrC8(reinterpret_cast <const TUint8*> (sqlite3_column_blob(iStmtHandle, colIdx)), byteLength)));
					}
				}
				break;
			case SQLITE_NULL:
				it.SetNull();
				break;
			default:
				__SQLASSERT(EFalse, ESqlPanicInternalError);
				break;
			}//end of switch(...)
		}//end of - while(it.Next())
	iBufFlatType = ESqlColumnValuesBuf;
	SQLPROFILER_REPORT_ALLOC(iBufFlat.MaxSize());
	return iBufFlat;
	}

/**
This method sets aColumnSource parameter to point to the column data.

@param aColumnIndex Column Index, zero based. 
@param aColumnSource Output parameter. It is set to point to the column data.

@return KErrNone, the operation completed successfully;
		KErrArgument, the refered by aColumnIndex index column is not a binary or text column.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
*/	
TInt CSqlSrvStatement::ColumnSource(TInt aColumnIndex, TPtrC8& aColumnSource) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	TInt colType = sqlite3_column_type(iStmtHandle, aColumnIndex);
	if(colType == SQLITE_TEXT)
        {//Since the first called function after the Next() operation is always CSqlSrvStatement::ColumnValuesL(), then
         //sqlite3_column_bytes16() (called from  ColumnValuesL()) already allocated the needed memory if a UTF8->UTF16 
         //conversion had to be performed. The sqlite3_column_text16() on the next line is guaranteed to succeed.
		const void* text = sqlite3_column_text16(iStmtHandle, aColumnIndex);
        __SQLASSERT(text != NULL, ESqlPanicInternalError);
		TInt length  = sqlite3_column_bytes16(iStmtHandle, aColumnIndex);
		aColumnSource.Set(reinterpret_cast <const TUint8*> (text), length);
		}
	else if(colType == SQLITE_BLOB)
		{
		const void* data = sqlite3_column_blob(iStmtHandle, aColumnIndex);
		TInt length  = sqlite3_column_bytes(iStmtHandle, aColumnIndex);
		aColumnSource.Set(reinterpret_cast <const TUint8*> (data), length);
		}
	else
		{
		return KErrArgument;	
		}
	return KErrNone;
	}

/**
Retrieves from the SQLITE library columns and parameters count.

@panic SqlDb 4 In _DEBUG mode. aDbHandle is NULL.
*/	
void CSqlSrvStatement::DoCommonConstructL()
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	iColumnCount = sqlite3_column_count(iStmtHandle);
	iParamCount = sqlite3_bind_parameter_count(iStmtHandle);
	__SQLLEAVE_IF_ERROR(iBufFlat.SetCount(Max(iColumnCount, iParamCount)));
	}

/**
Destroys the parameter buffer array (used for text or binary parameters).
Before the array destruction, each array member is notified that the statement is about to be finalized.
*/
void CSqlSrvStatement::DestroyParamBufArray()
	{
	TInt idx = iParamBufArray.Count();
	while(--idx >= 0)
		{
		if(iParamBufArray[idx])
			{
			iParamBufArray[idx]->NotifyStatementFinalized();	
			}
		}
	iParamBufArray.Close();
	}
	
/**
Binds a streamed text or binary parameter value.

@param aParamIndex The text/binary parameter index

@leave KErrNoMemory, an out of memory condition has occurred;
	   KSqlErrStmtExpired, statement handle has expired.
                  Note that the function may also leave with some other database specific 
                  errors categorised as ESqlDbError.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. No parameter buffer has been created yet for this parameter.
@panic SqlDb 4 In _DEBUG mode. Parameter index out of bounds.
*/
void CSqlSrvStatement::BindParamBufL(TInt aParamIndex)
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT(aParamIndex >= 0 && aParamIndex < sqlite3_bind_parameter_count(iStmtHandle), ESqlPanicBadArgument);
	__SQLASSERT(aParamIndex < iParamBufArray.Count(), ESqlPanicBadArgument);
	__SQLASSERT(iParamBufArray[aParamIndex] != NULL, ESqlPanicBadArgument);
	(void)sqlite3SymbianLastOsError();//clear last OS error
	//Bind the parameter value.
	//SQLITE_STATIC is used as an argument, because the text/blob data will be kept and can be used by the next bind call
	HSqlSrvStmtParamBuf& paramBuf = *iParamBufArray[aParamIndex];
	const TPtrC8 paramData(paramBuf.Data());
	SQLPROFILER_REPORT_ALLOC(paramData.Length());
	TInt err = KErrNone;
	++aParamIndex;//SQLite uses positive parameter indexes, the SQL server - parameter indexes begin from 0
	switch(paramBuf.DataType())
		{
		case HSqlSrvStmtParamBuf::EText16:
			//sqlite3_bind_text16() expects 4-th argument to be the bytes count, not the characters count.
			err = sqlite3_bind_text16(iStmtHandle, aParamIndex, paramData.Ptr(), paramData.Length(), SQLITE_STATIC);
			break;
		case HSqlSrvStmtParamBuf::EBinary:
		default:
			err = sqlite3_bind_blob(iStmtHandle, aParamIndex, paramData.Ptr(), paramData.Length(), SQLITE_STATIC);
			break;
		}
	err = ::Sql2OsErrCode(err, sqlite3SymbianLastOsError());
	__SQLLEAVE_IF_ERROR(err);
	}

/**
@return Represents the content of the column identified by aColIdx as integer value.
		If the current column type does not refer to an integer, then 
		the function will do a data conversion as described in the table which can be found
		in SqlDb.h file.
@see RSqlStatement

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Invalid aColIdx value.
*/
TInt CSqlSrvStatement::ColumnInt(TInt aColIdx) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT((TUint)aColIdx < iColumnCount, ESqlPanicBadArgument);
	TInt colType = sqlite3_column_type(iStmtHandle, aColIdx);
	switch(colType)
		{
		case SQLITE_FLOAT:
			{
			TReal roundVal;
			TInt err = Math::Round(roundVal, sqlite3_column_double(iStmtHandle, aColIdx), 0);
			if(err != KErrNone)
				{
				return KMinTInt;
				}
			TRealX val(roundVal);
			return static_cast <TInt> (val);
			}
		case SQLITE_NULL:
		case SQLITE_TEXT:
		case SQLITE_BLOB:
			return 0;
		default:			//int, int64
			{
			TInt64 val = sqlite3_column_int64(iStmtHandle, aColIdx);
			return val == (TInt)val ? (TInt)val : (val < KMinTInt ? KMinTInt : KMaxTInt);
			}
		}
	}
	
/**
@return Represents the content of the column identified by aColIdx as 64-bit integer value.
		If the current column type does not refer to a 64-bit integer, then 
		the function will do a data conversion as described in the table which can be found
		in SqlDb.h file.
@see RSqlStatement

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Invalid aColIdx value.
*/
TInt64 CSqlSrvStatement::ColumnInt64(TInt aColIdx) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT((TUint)aColIdx < iColumnCount, ESqlPanicBadArgument);
	TInt colType = sqlite3_column_type(iStmtHandle, aColIdx);
	switch(colType)
		{
		case SQLITE_FLOAT:
			{
			TReal roundVal;
			TInt err = Math::Round(roundVal, sqlite3_column_double(iStmtHandle, aColIdx), 0);
			if(err != KErrNone)
				{
				return KMinTInt64;
				}
			TRealX val(roundVal);
			return static_cast <TInt64> (val);
			}
		case SQLITE_NULL:
		case SQLITE_TEXT:
		case SQLITE_BLOB:
			return 0;
		default:			//int, int64
			return sqlite3_column_int64(iStmtHandle, aColIdx);
		}
	}
	
/**
@return Represents the content of the column identified by aColIdx as real value.
		If the current column type does not refer to a real, then 
		the function will do a data conversion as described in the table which can be found
		in SqlDb.h file.
@see RSqlStatement

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Invalid aColIdx value.
*/
TReal CSqlSrvStatement::ColumnReal(TInt aColIdx) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT((TUint)aColIdx < iColumnCount, ESqlPanicBadArgument);
	TInt colType = sqlite3_column_type(iStmtHandle, aColIdx);
	switch(colType)
		{
		case SQLITE_INTEGER:
			{
			TRealX val(sqlite3_column_int64(iStmtHandle, aColIdx));	
			return static_cast <TReal> (val);
			}
		case SQLITE_NULL:
		case SQLITE_TEXT:
		case SQLITE_BLOB:
			return 0.0;
		default:
			return sqlite3_column_double(iStmtHandle, aColIdx);
		}
	}
	
/**
Represents the content of the column identified by aColIdx as text (16 bit) descriptor.
If the current column type does not refer to a text block of data, then 
the function will do a data conversion as described in the table which can be found
in SqlDb.h file.
		
@see RSqlStatement

@leave KErrNoMemory, an out of memory condition has occurred.

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Invalid aColIdx value.
*/
TPtrC CSqlSrvStatement::ColumnTextL(TInt aColIdx) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT((TUint)aColIdx < iColumnCount, ESqlPanicBadArgument);
	TPtrC res;
	TInt colType = sqlite3_column_type(iStmtHandle, aColIdx);
	if(colType == SQLITE_TEXT)
		{
		TInt charLength = (TUint)sqlite3_column_bytes16(iStmtHandle, aColIdx) / sizeof(TUint16);
        //"charLength == 0" - this might be an indication of an "out of memory" problem, if the column text is in UTF8 format. 
        //(sqlite3_column_bytes16() may allocate memory for UTF8->UTF16 conversion)
        if(charLength == 0 && sqlite3_errcode(sqlite3_db_handle(iStmtHandle)) == SQLITE_NOMEM)
            {
            __SQLLEAVE(KErrNoMemory);
            }
        //sqlite3_column_bytes16() already allocated the needed memory if a UTF8->UTF16 conversion
        //had to be performed. The sqlite3_column_text16() on the next line is guaranteed to succeed.
        const TUint16* text = reinterpret_cast <const TUint16*> (sqlite3_column_text16(iStmtHandle, aColIdx));
        __SQLASSERT(text != NULL, ESqlPanicInternalError);
		res.Set(text, charLength);
		}
	return res;
	}
	
/**
Represents the content of the column identified by aColIdx as binary (8 bit) descriptor.
If the current column type does not refer to a binary block of data, then 
the function will do a data conversion as described in the table which can be found
in SqlDb.h file.
                      
@see RSqlStatement

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Invalid aColIdx value.
*/
TPtrC8 CSqlSrvStatement::ColumnBinary(TInt aColIdx) const
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT((TUint)aColIdx < iColumnCount, ESqlPanicBadArgument);
	TPtrC8 res;
	TInt colType = sqlite3_column_type(iStmtHandle, aColIdx);
	if(colType == SQLITE_BLOB)
		{
		TInt byteLength = sqlite3_column_bytes(iStmtHandle, aColIdx);
		res.Set(reinterpret_cast <const TUint8*> (sqlite3_column_blob(iStmtHandle, aColIdx)), byteLength);
		}
	return res;
	}

/**
Retrieves the declared column types using the SQLITE library storing in a 
flat buffer and returns a reference to the buffer. 

@return A const reference to a flat buffer containing the declared column type names.

@leave KErrNoMemory, an out of memory condition has occurred;

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
*/
const RSqlBufFlat& CSqlSrvStatement::GetDeclColumnTypesL()
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	iBufFlatType = static_cast <TSqlBufFlatType> (-1);
	__SQLLEAVE_IF_ERROR(iBufFlat.SetCount(iColumnCount));
	TSqlBufWIterator it;
	it.Set(iBufFlat);
	TInt colIdx = -1;
	while(it.Next())
		{
		++colIdx;//the first SQLITE column index is 0
		const TUint16* declTypeTxt = reinterpret_cast <const TUint16*> (sqlite3_column_decltype16(iStmtHandle, colIdx));
		TPtrC ptr(KNullDesC);
        if(declTypeTxt)
            {
            ptr.Set(declTypeTxt, User::StringLength(declTypeTxt));
            }
        else
            {
            //If sqlite3_column_decltype16() returns NULL but sqlite3_column_decltype() doesn't, then it is an "out of memory" condition 
            if(sqlite3_column_decltype(iStmtHandle, colIdx))
                {
                __SQLLEAVE(KErrNoMemory);
                }
            }
		__SQLLEAVE_IF_ERROR(it.SetText(ptr));
		}
	iBufFlatType = ESqlDeclColumnTypesBuf;
	return iBufFlat;	
	}


/**
Creates a new HSqlSrvStmtParamBuf object for the parameter with index "aParamIndex" or
reuses the existing one.

@param aParamIndex Parameter index, zero based.
@param aDataType Parameter value type - binary, text8 or text16.
@param aIsStreamBuf True if the param data will be retrieved from an IPC stream

@return A pointer to the created HSqlSrvStmtParamBuf instance.

@leave KErrNoMemory, an out of memory condition has occurred;
*/
HSqlSrvStmtParamBuf* CSqlSrvStatement::GetParamBufL(TInt aParamIndex, HSqlSrvStmtParamBuf::TDataType aDataType, 
													HSqlSrvStmtParamBuf::TBufType aBufType)
	{
	__SQLASSERT(aParamIndex >= 0 && aParamIndex < sqlite3_bind_parameter_count(iStmtHandle), ESqlPanicBadArgument);
	ExtendParamBufArrayL(aParamIndex);
	HSqlSrvStmtParamBuf*& paramBuf = iParamBufArray[aParamIndex];
	if(paramBuf)
		{//Reset and reuse the existing buffer
		__SQLASSERT(paramBuf->ParamIndex() == aParamIndex, ESqlPanicInternalError);
		paramBuf->Reset(aDataType, aBufType);	
		}
	else
		{
		paramBuf = HSqlSrvStmtParamBuf::NewL(*this, aParamIndex, aDataType, aBufType);
		}
	return paramBuf;
	}

/**
This function will extend the iParamBufArray array (filling the new array items with NULL), if it is needed - 
to ensure that there is enough place in the buffer for the parameter identified by aParamIndex.

@param aParamIndex The parameter index

@leave KErrNoMemory, an out of memory condition has occurred;

@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Parameter index out of bounds.
*/
void CSqlSrvStatement::ExtendParamBufArrayL(TInt aParamIndex)
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT(aParamIndex >= 0 && aParamIndex < sqlite3_bind_parameter_count(iStmtHandle), ESqlPanicBadArgument);
	TInt ext = aParamIndex - iParamBufArray.Count() + 1;
	while(ext-- > 0)
		{
		__SQLLEAVE_IF_ERROR(iParamBufArray.Append(NULL));
		}
	}

/**
This function will create a copy of the aParamValue and store it in the iParamBufArray array for later use.
The reason: once bound, the parameter value can be used multiple times by the SQLite if it is not set explicitly again.

@param aParamIndex The parameter index
@param aDataType Parameter value type - binary, text8 or text16.
@param aParamValue The parameter value

@return 8-bit descriptor to the stored parameter value

@leave KErrNoMemory, an out of memory condition has occurred;
                     
@panic SqlDb 2 In _DEBUG mode. Invalid (not created) CSqlSrvStatement object.
@panic SqlDb 4 In _DEBUG mode. Parameter index out of bounds.
*/
TPtrC8 CSqlSrvStatement::CopyAndStoreParamL(TInt aParamIndex, HSqlSrvStmtParamBuf::TDataType aDataType, const TDesC8& aParamValue)
	{
	__SQLASSERT(iStmtHandle != NULL, ESqlPanicInvalidObj);
	__SQLASSERT(aParamIndex >= 0 && aParamIndex < sqlite3_bind_parameter_count(iStmtHandle), ESqlPanicBadArgument);
	HSqlSrvStmtParamBuf* paramBuf = GetParamBufL(aParamIndex, aDataType, HSqlSrvStmtParamBuf::EBufSimpleBind);
	return paramBuf->SetDataL(aParamValue);
	}
