// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "sqlite3.h"
#include "SqlPanic.h"
#include "SqlSrvBlob.h"
#include "SqliteSymbian.h"
#include "SqlSrvUtil.h"
#include "SqlSrvStrings.h"
#include "SqlDb.h"

/**
Creates a new HBlobBuf instance.

@param aDb 			 The database handle
@param aDbName 		 The database name, zero-terminated. If the blob belongs to one of the attached databases, 
					 then the attached database name should be used as the value of aDbName, otherwise the
					 value should be "main"
@param aTableName    The name of the table to which the blob belongs, zero-terminated.
@param aColumnName   The name of the blob column, zero-terminated.
@param aRowId        The ROWID of the row to which the blob belongs
@param aMode		 Specifies the blob access mode, either HBlobBuf::EReadOnly or HBlobBuf::EReadWrite

@leave KErrNoMemory, An out of memory condition has occurred;
                     Note that the function may also leave with some other system wide errors or 
                     database specific errors categorised as ESqlDbError.

@return A pointer to the created HBlobBuf instance
                     
@panic SqlDb 4 In _DEBUG mode. NULL aDb parameter.
@panic SqlDb 4 In _DEBUG mode. Negative or zero aRowId.
@panic SqlDb 4 In _DEBUG mode. aMode parameter is not HBlobBuf::EReadOnly or HBlobBuf::EReadWrite.
*/
HBlobBuf* HBlobBuf::NewL(sqlite3* aDb, const TDesC8& aDbName, const TDesC8& aTableName, const TDesC8& aColumnName, TInt64 aRowId, TMode aMode)
	{
	__SQLASSERT(aDb != NULL, ESqlPanicBadArgument);
	__SQLASSERT(aRowId > 0, ESqlPanicBadArgument);
	__SQLASSERT(aMode == HBlobBuf::EReadOnly || aMode == HBlobBuf::EReadWrite, ESqlPanicBadArgument);
	
	HBlobBuf* self = new (ELeave) HBlobBuf;
	CleanupStack::PushL(self);
	self->ConstructL(aDb, aDbName, aTableName, aColumnName, aRowId, aMode);
	CleanupStack::Pop(self);
	return self;
	}

/**
Initializes HBlobBuf data members with their default values.
*/
HBlobBuf::HBlobBuf() :
	iBlobHandle(NULL),
	iWrPos(KMinTInt),
	iRdPos(KMinTInt)
	{
	}

/**
Initializes a new HBlobBuf instance.

@param aDb 			 The database handle
@param aDbName 		 The database name, zero-terminated. If the blob belongs to one of the attached databases, 
					 then the attached database name should be used as a value of aDbName, otherwise the
					 value should be "main"
@param aTableName    The name of the table to which the blob belongs, zero-terminated.
@param aColumnName   The name of the blob column, zero-terminated.
@param aRowId        The ROWID of the row to which the blob belongs
@param aMode		 Specifies the blob access mode, either HBlobBuf::EReadOnly or HBlobBuf::EReadWrite

@leave KErrNoMemory, 	An out of memory condition has occurred;
                     	Note that the function may also leave with some other system wide errors or 
                    	database specific errors categorised as ESqlDbError.
                     
@panic SqlDb 4 In _DEBUG mode. NULL aDb parameter.
@panic SqlDb 4 In _DEBUG mode. Negative or zero aRowId.
@panic SqlDb 4 In _DEBUG mode. aMode parameter is not HBlobBuf::EReadOnly or HBlobBuf::EReadWrite.
@panic SqlDb 7 In _DEBUG mode. NULL blob handle.
*/
void HBlobBuf::ConstructL(sqlite3* aDb, const TDesC8& aDbName, const TDesC8& aTableName, const TDesC8& aColumnName, TInt64 aRowId, TMode aMode)
	{
	__SQLASSERT(aDb != NULL, ESqlPanicBadArgument);
	__SQLASSERT(aRowId > 0, ESqlPanicBadArgument);
	__SQLASSERT(aMode == HBlobBuf::EReadOnly || aMode == HBlobBuf::EReadWrite, ESqlPanicBadArgument);
	
	(void)sqlite3SymbianLastOsError();//clear last OS error
			
	TInt err = sqlite3_blob_open(aDb, (const char*)aDbName.Ptr(), (const char*)aTableName.Ptr(), (const char*)aColumnName.Ptr(), 
								 aRowId, aMode & HBlobBuf::EReadWrite, &iBlobHandle);
	__SQLLEAVE_IF_ERROR(::Sql2OsErrCode(err, sqlite3SymbianLastOsError()));
	__SQLASSERT(iBlobHandle != NULL, ESqlPanicInternalError);
	iBlobSize = sqlite3_blob_bytes(iBlobHandle);
	iWrPos = iRdPos = 0;
	}

/**
Closes the blob handle as part of the HBlobBuf object being destroyed.
*/
void HBlobBuf::DoRelease()
	{
	TRAP_IGNORE(DoSynchL());
	}
	
/**
Closes the blob handle.
Any buffered data is delivered to the stream.
*/	
void HBlobBuf::DoSynchL()
	{
	if(iBlobHandle)
		{
		TInt err = sqlite3_blob_close(iBlobHandle);
		iBlobHandle = NULL; // the close is unconditional, even if an error occurs
		__SQLLEAVE_IF_ERROR(::Sql2OsErrCode(err, sqlite3SymbianLastOsError()));
		}	
	}

/**
Reads a data block from the blob with the specified length.

@param aPtr Pointer to the location where the blob data will be copied
@param aMaxLength The length of the data block to be read

@leave KErrNoMemory,  An out of memory condition has occurred;
       KErrEof, 	  An attempt has been made to read beyond the end of the blob;
       KErrBadHandle, NULL blob handle;
                      Note that the function may also leave with some other system 
                      wide errors or database specific errors categorised as ESqlDbError.

@return The number of bytes read.

@panic SqlDb 4 In _DEBUG mode. NULL aPtr parameter.
@panic SqlDb 4 In _DEBUG mode. Negative aMaxLength parameter.
@panic SqlDb 2 In _DEBUG mode. NULL iBlobHandle.
*/
TInt HBlobBuf::DoReadL(TAny* aPtr, TInt aMaxLength)
	{
	__SQLASSERT(aPtr != NULL, ESqlPanicBadArgument);
	__SQLASSERT(aMaxLength >= 0, ESqlPanicBadArgument);
	__SQLASSERT(iBlobHandle != NULL, ESqlPanicInvalidObj);
	
	if(aMaxLength <= 0)
		{
		return 0;	
		}
	
	(void)sqlite3SymbianLastOsError();//clear last OS error
	
	if(iRdPos >= iBlobSize)
		{
		__SQLLEAVE(KErrEof);	
		}
	if((aMaxLength + iRdPos) > iBlobSize)
		{
		aMaxLength = iBlobSize - iRdPos;
		}
	TInt err = sqlite3_blob_read(BlobHandleL(), aPtr, aMaxLength, iRdPos);
	__SQLLEAVE_IF_ERROR(::Sql2OsErrCode(err, sqlite3SymbianLastOsError()));
	iRdPos += aMaxLength;
	return aMaxLength;
	}

/**
Writes a data block with the specified length to the blob.

@param aPtr Pointer to the location with the blob data to be written
@param aLength The length of the data block to be written

@leave KErrNoMemory,  An out of memory condition has occurred;
       KErrEof,       An attempt has been made to write beyond the end of the blob;
       KErrBadHandle, NULL blob handle;
                      Note that the function may also leave with some other system 
                      wide errors or database specific errors categorised as ESqlDbError.

@panic SqlDb 4 In _DEBUG mode. NULL aPtr parameter.
@panic SqlDb 4 In _DEBUG mode. Negative aLength parameter.
@panic SqlDb 2 In _DEBUG mode. NULL iBlobHandle.
*/
void HBlobBuf::DoWriteL(const TAny* aPtr, TInt aLength)
	{
	__SQLASSERT(aPtr != NULL, ESqlPanicBadArgument);
	__SQLASSERT(aLength >= 0, ESqlPanicBadArgument);
	__SQLASSERT(iBlobHandle != NULL, ESqlPanicInvalidObj);
	
	if(aLength <= 0)
		{
		return;
		}
		
	if((iWrPos + aLength) > iBlobSize)
		{
		__SQLLEAVE(KErrEof);
		}

	(void)sqlite3SymbianLastOsError();//clear last OS error
	TInt err = sqlite3_blob_write(BlobHandleL(), aPtr, aLength, iWrPos);
	err = ::Sql2OsErrCode(err, sqlite3SymbianLastOsError());
	
	__SQLLEAVE_IF_ERROR(err);
	iWrPos += aLength;
	}

/**
Positions the mark(s) indicated by aMark at aOffset from aLocation.

@leave KErrEof,       An attempt has been made to seek beyond the end of the blob;
	   KErrBadHandle, NULL blob handle;
                      Note that the function may also leave with some other system wide errors or 
                      database specific errors categorised as ESqlDbError.

@return The new stream position (read or write)

@panic SqlDb 2 In _DEBUG mode. NULL iBlobHandle.
@panic SqlDb 4 In _DEBUG mode. Negative aOffset parameter.
@panic SqlDb 8 In _DEBUG mode. Invalid aMark parameter.
@panic SqlDb 9 In _DEBUG mode. Invalid aLocation parameter.
*/
TStreamPos HBlobBuf::DoSeekL(MStreamBuf::TMark aMark, TStreamLocation aLocation, TInt aOffset)
	{
	__SQLASSERT_ALWAYS(!(aMark & ~(ERead | EWrite)), ESqlPanicStreamMarkInvalid);
	__SQLASSERT(aOffset >= 0, ESqlPanicBadArgument);
	__SQLASSERT(iBlobHandle != NULL, ESqlPanicInvalidObj);
	
	TInt newPos = 0;
	switch(aLocation)
		{
	case EStreamBeginning:
		newPos = aOffset;
		break;
	case EStreamMark:
		newPos = (aMark & MStreamBuf::EWrite ? iWrPos : iRdPos) + aOffset;
		break;
	case EStreamEnd:
		newPos = iBlobSize + aOffset;
		break;
	default:
		__SQLASSERT(0, ESqlPanicStreamLocationInvalid);
		newPos = -1;
		break;
		}
	if(newPos < 0 || newPos > iBlobSize)
		{
		__SQLLEAVE(KErrEof);
		}
	if(aMark & MStreamBuf::EWrite)
		{
		iWrPos = newPos;
		}
	else if(aMark & MStreamBuf::ERead)
		{
		iRdPos = newPos;	
		}
	return TStreamPos(newPos);
	}

/**
Returns the blob handle, if it is not NULL.

@leave KErrBadHandle, The blob handle is NULL.

@return The blob handle
*/
sqlite3_blob* HBlobBuf::BlobHandleL()
	{
	if(!iBlobHandle)
		{
		__SQLLEAVE(KErrBadHandle);
		}
	return iBlobHandle;
	}
