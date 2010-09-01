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

#ifndef UTRACESQLITE_H_
#define UTRACESQLITE_H_

#if defined SQLITE_ENABLE_TRACE

// UID3 of sqlite3.dll
#define EXECUTABLE_DEFAULT_MODULEUID 0x10285A79

//The UTF header can only be included after EXECUTABLE_DEFAULT_MODULEUID is defined  
#include <e32utf.h>

/**
Enable this macro to compile in the SQLite trace points that certain porting layer functions entry and exit. 
From the timestamps of these trace, the total time spent in these functions can be worked out. 
These traces are particularly useful for performance investigations.

@SymTraceMacro
*/
#undef SYMBIAN_TRACE_SQLITE_FUNC

/**
Enable this macro to compile in the SQLite trace points that trace the following internal events. 
	-	Extra internal information for certain porting layer functions
These traces can be used to assist performance and debug investigations

@SymTraceMacro
*/
#undef SYMBIAN_TRACE_SQLITE_EVENTS

#endif //SQLITE_ENABLE_TRACE


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                      		UTrace Related Strings                       ///////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined SQLITE_ENABLE_TRACE
#ifdef __SQLITETRACE_STRINGS__
	#define CONST_LIT8(var, val) extern const TLitC8<sizeof(val)> var = {sizeof(val) - 1, val}
	#define CONST_LIT16(var, val) extern const TLitC16<sizeof(L##val)/2> var = {sizeof(L##val)/2 - 1, L##val}
#else
	#define CONST_LIT8(var, val) extern const TLitC8<sizeof(val)> var
	#define CONST_LIT16(var, val) extern const TLitC16<sizeof(L##val)/2> var
#endif

//File I/O trace format strings
CONST_LIT8(KFileRead, 			"Sqlite: Size - %d bytes, File Offset Position - %d,");
CONST_LIT8(KFileWrite,			"Sqlite: Size - %d bytes, File Offset Position - %d,");
CONST_LIT8(KFileTruncate,		"Sqlite: Size - %d bytes");
CONST_LIT8(KFileFileCtr,		"Sqlite: Opeartion Called - %d");
CONST_LIT16(KFileOpen,			"Sqlite: Sqlite3_file - 0x%x, Filename - \"%S\"");
CONST_LIT16(KFileName,			"Sqlite: Filename - \"%S\"");

//TSqlUTraceProfiler trace format strings, used to display function entry and exit
CONST_LIT8(KProfilerBegin,			"Function Entry:\"%S\", Address - 0x%x");
CONST_LIT8(KProfilerEnd,			"Function Exit:\"%S\",  Address - 0x%x");

#endif //SQLITE_ENABLE_TRACE

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////              UTrace Related Macro Functions and Class Declarations           ////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SYMBIAN_TRACE_SQLITE_FUNC

/**
This class is used to help trace function entry and exits within sqlite3.dll
Exist only when SYMBIAN_TRACE_SQLITE_FUNC is enabled.

@internalComponent
*/
class TSqlUTraceProfiler
	{
	public:
		TSqlUTraceProfiler(const TAny* aObj, const char* aFunctionStr);
		~TSqlUTraceProfiler();
	private: 
		const TAny* iObj;
		TPtrC8 iFunctionStr;
	};

//This macro should be used to trace function entry and exits within Symbian SQL 
//Works only when SYMBIAN_TRACE_SQLITE_FUNC is enabled. Otherwise it evaluates to zero
#define SQLUTRACE_PROFILER(x) TSqlUTraceProfiler _profiler(x, __PRETTY_FUNCTION__)

#else

#define SQLUTRACE_PROFILER(x) void(0)
#endif //SYMBIAN_TRACE_SQLITE_FUNC

#ifdef SYMBIAN_TRACE_SQLITE_EVENTS

//This macro should be used to trace events occuring within Symbian SQL.
//To use the macro, insert the full UTF statement as the macro parameter
//Works only when SYMBIAN_TRACE_SQLITE_EVENTS is enabled. Otherwise it evaluates to zero
#define SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(c) c
const TPtrC8 GetFuncString(TInt aFunction);

#else

#define SYMBIAN_TRACE_SQLITE_EVENTS_ONLY(c) void(0)
#endif //SYMBIAN_TRACE_SQLITE_EVENTS
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* UTRACESQLITE_H_ */
