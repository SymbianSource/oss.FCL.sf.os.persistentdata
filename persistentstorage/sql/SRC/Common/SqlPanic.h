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

#ifndef __SQLPANIC_H__
#define __SQLPANIC_H__

#include "SqlAssert.h"

//Forward declarations
class RMessage2;

/**
TSqlPanic contains panic codes which may used by the SQL component (SqlDb.dll and SqlSrv.exe).

@publishedAll
@released
*/
enum TSqlPanic
	{
	ESqlPanicObjExists 			=   1,
	ESqlPanicInvalidObj			=   2,
	ESqlPanicBadHandle 			=   3,		
	ESqlPanicBadArgument 		=   4,		
	ESqlPanicBadColumnIndex 	=   5,
	ESqlPanicTypeMismatch 		=   6,	
	ESqlPanicInternalError		=   7,
	ESqlPanicStreamMarkInvalid  =   8,
	ESqlPanicStreamLocationInvalid= 9,
	ESqlPanicValueNotPresent  	=  10,
	ESqlPanicInvalidRow			=  11,
	ESqlPanicMisuse				=  12,
	ESqlPanicBadDescriptor		=  13
	};

/**
This function panics the thread where it is called from with aPanicCode panic code.

@param aPanicCode Panic code

@internalComponent
*/
void SqlPanic(TSqlPanic aPanicCode);

/**
This function is used inside the SQL server to panic the calling client.

@param aMessage The message which processing causes the panic.
@param aPanicCode Panic code

@leave KSqlLeavePanic

@return KErrNone

@internalComponent
*/
TInt SqlPanicClientL(const RMessage2& aMessage, TSqlPanic aPanicCode);
	
#endif //__SQLPANIC_H__
