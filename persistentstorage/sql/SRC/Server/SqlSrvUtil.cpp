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

#include <stdlib.h>				//wchar_t
#include "SqlPanic.h"
#include "SqlSrvUtil.h"

#ifdef _NOTIFY

//Used in PrintAuthorizerArguments()
_LIT8(KCreateIndex,			"Create index");			//SQLITE_CREATE_INDEX
_LIT8(KCreateTable, 		"Create table");			//SQLITE_CREATE_TABLE
_LIT8(KCreateTempIndex, 	"Create temp index");		//SQLITE_CREATE_TEMP_INDEX
_LIT8(KCreateTempTable, 	"Create temp table");		//SQLITE_CREATE_TEMP_TABLE
_LIT8(KCreateTempTrigger, 	"Create temp trigger");		//SQLITE_CREATE_TEMP_TRIGGER
_LIT8(KCreateTempView, 		"Create temp view");		//SQLITE_CREATE_TEMP_VIEW
_LIT8(KCreateTrigger, 		"Create trigger");			//SQLITE_CREATE_TRIGGER
_LIT8(KCreateView, 			"Create view");				//SQLITE_CREATE_VIEW
_LIT8(KDelete, 				"DELETE");					//SQLITE_DELETE
_LIT8(KDropIndex, 			"Drop index");				//SQLITE_DROP_INDEX
_LIT8(KDropTable, 			"Drop table");				//SQLITE_DROP_TABLE
_LIT8(KDropTempIndex, 		"Drop temp index");			//SQLITE_DROP_TEMP_INDEX
_LIT8(KDropTempTable, 		"Drop temp table");			//SQLITE_DROP_TEMP_TABLE
_LIT8(KDropTempTrigger, 	"Drop temp trigger");		//SQLITE_DROP_TEMP_TRIGGER
_LIT8(KDropTempView, 		"Drop temp view");			//SQLITE_DROP_TEMP_VIEW
_LIT8(KDropTrigger, 		"Drop trigger");			//SQLITE_DROP_TRIGGER
_LIT8(KDropView, 			"Drop view");				//SQLITE_DROP_VIEW
_LIT8(KInsert, 				"INSERT");					//SQLITE_INSERT
_LIT8(KPragma, 				"PRAGMA");					//SQLITE_PRAGMA
_LIT8(KRead, 				"Read");					//SQLITE_READ
_LIT8(KSelect, 				"SELECT");					//SQLITE_SELECT
_LIT8(KTransaction, 		"TRANSACTION");				//SQLITE_TRANSACTION
_LIT8(KUpdate, 				"UPDATE");					//SQLITE_UPDATE
_LIT8(KAttach, 				"ATTACH");					//SQLITE_ATTACH
_LIT8(KDetach, 				"DETACH");					//SQLITE_DETACH
_LIT8(KAlterTable, 			"Alter table");				//SQLITE_ALTER_TABLE
_LIT8(KReindex, 			"Reindex");					//SQLITE_REINDEX
_LIT8(KAnalyze, 			"Analyze");					//SQLITE_ANALYZE
_LIT8(KCreateVTable, 		"Create virt.table");		//SQLITE_CREATE_VTABLE
_LIT8(KDropVTable, 			"Drop virt.table");			//SQLITE_DROP_VTABLE
_LIT8(KFunctionCall, 		"Function call");			//SQLITE_FUNCTION

_LIT8(KNull, 				"NULL");
_LIT8(KInvalid, 			"INVALID");

//Used in PrintAuthorizerArguments()
const TPtrC8 KDbOpNames[] = 
	{
	KCreateIndex(),	KCreateTable(), KCreateTempIndex(), KCreateTempTable(), KCreateTempTrigger(),
	KCreateTempView(), KCreateTrigger(), KCreateView(), KDelete(), KDropIndex(), 
	KDropTable(), KDropTempIndex(), KDropTempTable(), KDropTempTrigger(), KDropTempView(),
	KDropTrigger(), KDropView(), KInsert(), KPragma(), KRead(), 
	KSelect(), KTransaction(), KUpdate(), KAttach(), KDetach(), KAlterTable(), KReindex(), KAnalyze(),
	KCreateVTable(), KDropVTable(), KFunctionCall()
	};

const TInt KMaxOpCodes = sizeof(KDbOpNames) / sizeof(KDbOpNames[0]);

_LIT(KFormatStr, "!!Authorize: %20.20S, %40.40S, %10.10S, %10.10S, %10.10S\r\n"); 

/**
This function has a defined implementaion only in _DEBUG mode and is used to print the authorizer arguments.

@internalComponent
*/
void PrintAuthorizerArguments(TInt aDbOpType, const char* aDbObjName1, const char* aDbObjName2, 
							  const char* aDbName, const char* aTrgOrViewName)
	{
	__SQLASSERT(aDbOpType > 0 && aDbOpType <= KMaxOpCodes, ESqlPanicInternalError);
	
	//TPtrC8 objects cannot be used for the function arguments, because the arguments may not be 16-bit aligned!!!	

	TBuf<20> opName;
	opName.Copy(KDbOpNames[aDbOpType - 1]);

	TBuf<64> dbObjName1; 
	dbObjName1.Copy(KNull);
	if(aDbObjName1)
		{
		dbObjName1.Copy(KInvalid);
		if(User::StringLength((const TUint8*)aDbObjName1) <= dbObjName1.MaxLength())
			{
			dbObjName1.Copy(TPtrC8(reinterpret_cast <const TUint8*> (aDbObjName1)));
			}
		}
	TBuf<64> dbObjName2; 
	dbObjName2.Copy(KNull);
	if(aDbObjName2)
		{
		dbObjName2.Copy(KInvalid);
		if(User::StringLength((const TUint8*)aDbObjName2) <= dbObjName2.MaxLength())
			{
			dbObjName2.Copy(TPtrC8(reinterpret_cast <const TUint8*> (aDbObjName2)));
			}
		}
	TBuf<64> dbName; 
	dbName.Copy(KNull);
	if(aDbName)
		{
		dbName.Copy(KInvalid);
		if(User::StringLength((const TUint8*)aDbName) <= dbName.MaxLength())
			{
			dbName.Copy(TPtrC8(reinterpret_cast <const TUint8*> (aDbName)));
			}
		}
	TBuf<64> trgOrViewName; 
	trgOrViewName.Copy(KNull);
	if(aTrgOrViewName)
		{
		trgOrViewName.Copy(KInvalid);
		if(User::StringLength((const TUint8*)aTrgOrViewName) <= trgOrViewName.MaxLength())
			{
			trgOrViewName.Copy(TPtrC8(reinterpret_cast <const TUint8*> (aTrgOrViewName)));
			}
		}
		
	RDebug::Print(KFormatStr, &opName, &dbObjName1, &dbObjName2, &dbName, &trgOrViewName);
	}
#endif//_NOTIFY

/**
Converts a UTF16 encoded descriptor to a UTF8 encoded descriptor.
Note: the function works only for input descriptors with length less or equal than KMaxFileName.

@param aIn  The input UTF16 encoded descriptor,
@param aOut The output buffer where the converted input descriptor will be stored. 
@return True if the conversion was successful, false otherwise.

@panic SqlDb 4 In _DEBUG mode if aIn length is bigger than KMaxFileName.
@panic SqlDb 4 In _DEBUG mode if aOut max length is less than KMaxFileName.

@internalComponent
*/
TBool UTF16ToUTF8(const TDesC& aIn, TDes8& aOut)
	{
	__SQLASSERT(aIn.Length() <= KMaxFileName, ESqlPanicBadArgument);
	__SQLASSERT(aOut.MaxLength() >= KMaxFileName, ESqlPanicBadArgument);
	if(aIn.Length() > KMaxFileName || aOut.MaxLength() < KMaxFileName)
		{
		return EFalse;	
		}
	TBuf16<KMaxFileName + 1> des;
	des.Copy(aIn);
	des.Append(TChar(0));
	TInt len = wcstombs((char*)aOut.Ptr(), (const wchar_t*)des.Ptr(), KMaxFileName);
	//Check the file name length. If it is longer than KMaxFileName characters, then the file name is not valid.
	if((TUint)len <= KMaxFileName)
		{
		aOut.SetLength(len);
		return ETrue;
		}
	return EFalse;
	}

/**
Converts a zero-terminated, UTF16 encoded file name to a zero-terminated, UTF8 encoded file name.
@param aFileName The input file name buffer. aFileName argument is expected to point to UTF16 encoded, 
				 zero terminated string,
@param aFileNameDestBuf The output file name buffer where the converted input file name will be stored. 
						The output file name buffer max length should be at least KMaxFileName + 1.
@return True if the conversion was successful, false otherwise.

@panic SqlDb 4 In _DEBUG mode if aFileName length is bigger than KMaxFileName + 1.
@panic SqlDb 4 In _DEBUG mode if aFileName is not zero-terminated or if aFileNameDestBuf max length is less than KMaxFileName + 1.

@internalComponent
*/
TBool UTF16ZToUTF8Z(const TDesC& aFileName, TDes8& aFileNameDestBuf)
	{
	__SQLASSERT(aFileName.Length() <= (KMaxFileName + 1), ESqlPanicBadArgument);
	__SQLASSERT(aFileName[aFileName.Length() - 1] == 0, ESqlPanicBadArgument);
	__SQLASSERT(aFileNameDestBuf.MaxLength() >= (KMaxFileName + 1), ESqlPanicBadArgument);
	const wchar_t* src = reinterpret_cast <const wchar_t*> (aFileName.Ptr());
	TInt len = wcstombs((char*)aFileNameDestBuf.Ptr(), src, KMaxFileName);
	//Check the file name length. If it is longer than KMaxFileName characters, then the file name is not valid.
	if((TUint)len <= KMaxFileName)
		{
		aFileNameDestBuf.SetLength(len + 1);
		aFileNameDestBuf[len] = 0;
		return ETrue;
		}
	return EFalse;
	}

/**
Converts a UTF16 encoded file name to a zero-terminated, UTF8 encoded file name.
@param aFileName The input file name buffer. aFileName argument is expected to point to UTF16 encoded string,
@param aFileNameDestBuf The output file name buffer where the converted input file name will be stored. 
						The output file name buffer max length should be at least KMaxFileName + 1.
@return True if the conversion was successful, false otherwise.

@panic SqlDb 4 In _DEBUG mode if aFileName length is bigger than KMaxFileName.
@panic SqlDb 4 In _DEBUG mode if aFileNameDestBuf max length is less than KMaxFileName + 1.

@internalComponent
*/
TBool UTF16ToUTF8Z(const TDesC& aFileName, TDes8& aFileNameDestBuf)
	{
	__SQLASSERT(aFileName.Length() <= KMaxFileName, ESqlPanicBadArgument);
	__SQLASSERT(aFileNameDestBuf.MaxLength() >= (KMaxFileName + 1), ESqlPanicBadArgument);
	TBool rc = ::UTF16ToUTF8(aFileName, aFileNameDestBuf);
	if(rc)
		{
		aFileNameDestBuf.Append(0);
		}
	return rc;
	}

//Returns true if aDbFileName is a read-only file
TBool IsReadOnlyFileL(RFs& aFs, const TDesC& aDbFileName)
	{
	TEntry entry;
	TInt err = aFs.Entry(aDbFileName, entry);
	if(err == KErrNotFound)
		{//Non-existing file
		return EFalse;	
		}
	__SQLLEAVE_IF_ERROR(err);
	return entry.IsReadOnly();
	}
