// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <e32std.h>

//The file is used only to compile and include in the executable server image only a single
//copy of the trace strings.
#define __SQLTRACE_STRINGS__
#include "UTraceSql.h"

#ifdef SYMBIAN_TRACE_SQL_EVENTS

/**
Get the corresponding string to the IPC function from the TSqlSrvFunction enum

@param aFunction Function code
			
@return A pointer to the string literal associated with the TSqlSrvFunction enum identifiers

@internalComponent
 */ 
const TPtrC8 GetIPCFuncStr(TInt aFunction)
	{
	TInt function = aFunction & 0xFF;
	
	//Array of Sql Server's IPC functions.This should map to TSqlSrvFunction
	const TText8* const KSrvFunctionStr[] =
	   {
	   //Test functions	   
		_S8("ESqlSrvTestBase"),			//0x00
		_S8("ESqlSrvResourceMark"),		//0x01
		_S8("ESqlSrvResourceCheck"),	//0x02
		_S8("ESqlSrvResourceCount"),	//0x03
		_S8("ESqlSrvSetDbHeapFailure"),	//0x04
		_S8("ESqlSrvSetHeapFailure"),	//0x05
		
		//Profiling functions	
		_S8("ESqlSrvProfilerStart"),	//0x06
		_S8("ESqlSrvProfilerStop"),		//0x07
		_S8("ESqlSrvProfilerReset"),	//0x08
		_S8("ESqlSrvProfilerSetRange"),	//0x09
		_S8("ESqlSrvProfilerQuery"),	//0x0A
		
		//Not Used
		_S8(""),						//0x0B
		_S8(""),						//0x0C
		_S8(""),						//0x0D
		_S8(""),						//0x0E
		_S8(""),						//0x0F
		
		//Database functions
		_S8("ESqlSrvDbBase"),			//0x10
		_S8("ESqlSrvDbCreate"),			//0x11
		_S8("ESqlSrvDbCreateSecure"),	//0x12
		_S8("ESqlSrvDbOpen"),			//0x13
		_S8("ESqlSrvDbOpenFromHandle"),	//0x14
		_S8("ESqlSrvDbClose"),			//0x15	
		_S8("ESqlSrvDbCopy"),			//0x16
		_S8("ESqlSrvDbDelete"),			//0x17
		
		//All operations with opcode > ESqlSrvDbDelete require valid database object (on the server side)	
		_S8("ESqlSrvLastErrorMsg"),			//0x18
		_S8("ESqlSrvDbExec8"),				//0x19
		_S8("ESqlSrvDbExec16"),				//0x1A
		_S8("ESqlSrvDbRowIdExec16"),		//0x1B
		_S8("ESqlSrvDbSetIsolationLevel"),	//0x1C
		_S8("ESqlSrvDbGetSecurityPolicy"),	//0x1D
		_S8("ESqlSrvDbAttach"),				//0x1E
		_S8("ESqlSrvDbAttachFromHandle"),	//0x1F
		_S8("ESqlSrvDbDetach"),				//0x20
		_S8("ESqlSrvDbScalarFullSelect8"),	//0x21
		_S8("ESqlSrvDbScalarFullSelect16"),	//0x22
		_S8("ESqlSrvDbInTransaction"),		//0x23
		_S8("ESqlSrvDbSize"),				//0x24
		_S8("ESqlSrvDbSize2"),				//0x25
		_S8("ESqlSrvDbBlobSource"),			//0x26
		_S8("ESqlSrvDbLastInsertedRowId"),	//0x27
		_S8("ESqlSrvDbCompact"),			//0x28

		//Database - reserved drive space management
		_S8("ESqlSrvDbReserveDriveSpace"), 		//0x29
		_S8("ESqlSrvDbFreeReservedSpace"),		//0x2A
		_S8("ESqlSrvDbGetReserveAccess"),		//0x2B
		_S8("ESqlSrvDbReleaseReserveAccess"),	//0x2C
		
		//Not Used
		_S8(""),						//0x2D
		_S8(""),						//0x2E
		_S8(""),						//0x2F
		_S8(""),						//0x30
		_S8(""),						//0x31
		_S8(""),						//0x32
		_S8(""),						//0x33
		_S8(""),						//0x34
		_S8(""),						//0x35
		_S8(""),						//0x36
		_S8(""),						//0x37	
		_S8(""),						//0x38
		_S8(""),						//0x39
		_S8(""),						//0x3A
		_S8(""),						//0x3B
		_S8(""),						//0x3C
		_S8(""),						//0x3D
		_S8(""),						//0x3E
		_S8(""),						//0x3F
		_S8(""),						//0x40
		_S8(""),						//0x41
		_S8(""),						//0x42
		_S8(""),						//0x43
		_S8(""),						//0x44
		_S8(""),						//0x45
		_S8(""),						//0x46
		_S8(""),						//0x47
		_S8(""),						//0x48
		_S8(""),						//0x49
		_S8(""),						//0x4A
		_S8(""),						//0x4B
		_S8(""),						//0x4C
		_S8(""),						//0x4D
		_S8(""),						//0x4E
		_S8(""),						//0x4F
		
		//Statement functions	
		_S8("ESqlSrvStmtBase"),				//0x50
		_S8("ESqlSrvStmtPrepare8"),			//0x51
		_S8("ESqlSrvStmtPrepare16"),		//0x52	
		_S8("ESqlSrvStmtClose"),			//0x53
		_S8("ESqlSrvStmtReset"),			//0x54
		_S8("ESqlSrvStmtExec"),				//0x55
		_S8("ESqlSrvStmtAsyncExec"),		//0x56
		_S8("ESqlSrvStmtBindExec"),			//0x57
		_S8("ESqlSrvStmtBindExecRowId"),	//0x58
		_S8("ESqlSrvStmtAsyncBindExec"),	//0x59
		_S8("ESqlSrvStmtNext"),				//0x5A
		_S8("ESqlSrvStmtBindNext"),			//0x5B
		_S8("ESqlSrvStmtColumnNames"),		//0x5C
		_S8("ESqlSrvStmtParamNames"),		//0x5D
		_S8("ESqlSrvStmtColumnSource"),		//0x5E
		_S8("ESqlSrvStmtBinParamSink"),		//0x5F
		_S8("ESqlSrvStmtTxtParamSink8"),	//0x60
		_S8("ESqlSrvStmtTxtParamSink16"),	//0x61
		_S8("ESqlSrvStmtBufFlat"),			//0x62	
		_S8("ESqlSrvStmtColumnValue"),		//0x63
		_S8("ESqlSrvStmtDeclColumnTypes"),	//0x64
		_S8("ESqlSrvStmtEvaluateAll"),		//0x65
		_S8("ESqlSrvStmtGoto"),				//0x66
		
		//Not Used
		_S8(""),						//0x67
		_S8(""),						//0x68
		_S8(""),						//0x69
		_S8(""),						//0x6A
		_S8(""),						//0x6B
		_S8(""),						//0x6C
		_S8(""),						//0x6D
		_S8(""),						//0x6E
		_S8(""),						//0x6F

		//Stream functions
		_S8("ESqlSrvStreamBase"),		//0x70
		_S8("ESqlSrvStreamRead"),		//0x71
		_S8("ESqlSrvStreamWrite"),		//0x72
		_S8("ESqlSrvStreamSize"),		//0x73
		_S8("ESqlSrvStreamSynch"),		//0x74
		_S8("ESqlSrvStreamClose")		//0x75
	   };

	const TInt KMaxSrvFunctions = sizeof(KSrvFunctionStr) / sizeof(KSrvFunctionStr[0]);
	
	if (function <= KMaxSrvFunctions)
		return KSrvFunctionStr[function];
	else 
		return _S8("Unknown");
	}
#endif //SYMBIAN_TRACE_SQL_EVENTS

#ifdef SYMBIAN_TRACE_SQL_FUNC

/**
Create the TSqlUTraceProfiler object which inserts a UTF trace, used to signal a function entry

@param aFunctionStr 	A "const char" pointer describing the function to be profiled.
						Currently this is the type signature of the function returned by 
						the __PRETTY_FUNCTION__ macro.
@param aObj  			A object pointer used to provide context for the function.

@internalComponent
*/
TSqlUTraceProfiler::TSqlUTraceProfiler(const TAny* aObj, const char* aFunctionStr):
iObj(aObj), iFunctionStr(reinterpret_cast<const TUint8*>(aFunctionStr))
	{
	UTF::Printf(UTF::TTraceContext(UTF::ESystemCharacteristicMetrics), KProfilerBegin, &iFunctionStr, iObj);
	}

/**
Destroys TSqlUTraceProfiler object which inserts a UTF trace, used to signal a function exit

@internalComponent
*/
TSqlUTraceProfiler::~TSqlUTraceProfiler()
	{
	UTF::Printf(UTF::TTraceContext(UTF::ESystemCharacteristicMetrics), KProfilerEnd, &iFunctionStr, iObj);
	}

#endif //SYMBIAN_TRACE_SQL_FUNC
