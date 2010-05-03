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

#ifndef UTRACESQL_H
#define UTRACESQL_H

#if defined  SQLSRV_ENABLE_TRACE
//If this header is included from Sqlsrv.exe then use the UID3 from that process
#define EXECUTABLE_DEFAULT_MODULEUID 0x10281E17
#define SQL_ENABLE_TRACE

#elif defined SQLDB_ENABLE_TRACE  
//If this header is included from Sqldb.dll then use the UID3 from that process
#define EXECUTABLE_DEFAULT_MODULEUID 0x10281E18
#define SQL_ENABLE_TRACE

#elif defined SQLITELIB_ENABLE_TRACE
//If this header is included from Sqlite.lib then use the UID3 from that process
#define EXECUTABLE_DEFAULT_MODULEUID 0x10281E19
#define SQL_ENABLE_TRACE

#endif //SQLSRV_ENABLE_TRACE, SQLDB_ENABLE_TRACE, SQLITELIB_ENABLE_TRACE 

#ifdef SQL_ENABLE_TRACE

//The UTF header can only be included after EXECUTABLE_DEFAULT_MODULEUID is defined  
#include <e32utf.h>

/**
Enable this macro to compile in the SQL trace points that trace function leaves and panics.  
These traces can be used to assist debugging in client applications and the Symbian SQL.

@SymTraceMacro
*/
//#define SYMBIAN_TRACE_SQL_ERR
/**
Enable this macro to compile in the SQL trace points that trace exported functions and certain 
porting layer functions entry and exit. From the timestamps of these trace, the total time spent 
in these functions can be worked out. 
These traces are particularly useful for performance investigations.

@SymTraceMacro
*/
//#define SYMBIAN_TRACE_SQL_FUNC
/**
Enable this macro to compile in the SQL trace points that trace the following internal events. 
	-	IPC calls send to and serviced by the SQL Server
	- 	SQL Server's startup and close
	- 	No. of full table scans and sort operation performed by a RSqlStatement object
	-	Extra internal information for certain exported and porting layer functions
These traces can be used to assist performance and debug investigations

@SymTraceMacro
*/
//#define SYMBIAN_TRACE_SQL_EVENTS
#endif //SQL_ENABLE_TRACE	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////                      		UTrace Related Strings                      //////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef SQL_ENABLE_TRACE

#ifdef __SQLTRACE_STRINGS__
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

//Input pararmeter format strings 
CONST_LIT8(KStrParam,			"SqlDb: Parameter %d  - \"%S\"");
CONST_LIT8(KHexStrParam,		"SqlDb: Parameter 1 - 0x%x, Parameter 2 - \"%S\"");
CONST_LIT8(KHexIntParam,		"SqlDb: Parameter 1 - 0x%x, Parameter 2 - %d");
CONST_LIT8(KIntParam,			"SqlDb: Parameter 1  - %d");
CONST_LIT8(KIntSizeParam,		"SqlDb: Parameter 1 - %d, Parameter %d  - %d bytes");

CONST_LIT16(KStrParam16,		"SqlDb: Parameter %d  - \"%S\"");
CONST_LIT16(KStrStrParam16,		"SqlDb: Parameter 1 - \"%S\", Parameter 2 - \"%S\"");
CONST_LIT16(KHexStrParam16,		"SqlDb: Parameter 1 - 0x%x, Parameter 2 - \"%S\"");
CONST_LIT16(KSizeStrParam16,	"SqlDb: Parameter %d - %d bytes, Parameter %d - \"%S\"");
CONST_LIT16(KRSqlBlobParam16,	"SqlDb: Parameter 1 - 0x%x, Parameter 2 - \"%S\", Parameter 3 - \"%S\", Parameter 4 - %d, Parameter 5 - \"%S\"");	

//IPC request trace format strings
CONST_LIT8(KDbMsgStr, 			"SqlDb:  Request - %S Sent to SQL Server");
CONST_LIT8(KSrvMsgStr, 			"SqlSrv: Request - %S Recieved by SQL Server");
CONST_LIT8(KSrvStmtCreated, 	"SqlSrv: sqlite3_stmt Object 0x%x Created");
CONST_LIT8(KSrvStmtStatus,		"SqlSrv: sqlite3_stmt Object 0x%x Destroyed, %d Full Table Scan Performed, %d Sort Performed");

//Server startup and close trace format strings
CONST_LIT8(KSqlSrvStart,		"SqlSrv: SQL Server Startup");
CONST_LIT8(KSqlSrvClose,		"SqlSrv: SQL Server Close");

//TSqlUTraceProfiler trace format strings, used to display function entry and exit
CONST_LIT8(KProfilerBegin,			"Function Entry:\"%S\", Address - 0x%x");
CONST_LIT8(KProfilerEnd,			"Function Exit:\"%S\",  Address - 0x%x");

//Panic and leave trace format strings
CONST_LIT16(KSqlPanic,			"Sql: Panic %d at %S(%d)");
CONST_LIT16(KSqlPanicClient,		"Sql: Panic client %d at %S(%d)");
CONST_LIT16(KSqlLeave,			"Sql: Leave error %d at %S(%d)");
CONST_LIT8(KSqlSrvPanicClient, 	"SqlSrv ServiceError: Panic Client %d");
CONST_LIT8(KSqlSrvError, 		"SqlSrv ServiceError: Error %d");

#endif //SQL_ENABLE_TRACE	

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////              UTrace Related Macro Functions and Class Declarations           ////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined SYMBIAN_TRACE_SQL_ERR && defined SQL_ENABLE_TRACE

//This macro should be used to trace errors occurring within Symbian SQL.
//To use the macro, insert the full UTF statement as the macro parameter
//Works only when SYMBIAN_TRACE_SQL_ERR is enabled. Otherwise it evaluates to zero
#define SYMBIAN_TRACE_SQL_ERR_ONLY(c) c

#else

#define SYMBIAN_TRACE_SQL_ERR_ONLY(c) do {} while(0)
#endif //SYMBIAN_TRACE_SQL_ERR


#if defined SYMBIAN_TRACE_SQL_FUNC && defined SQL_ENABLE_TRACE

/**
This class is used to help trace function entry and exits within Symbian SQL 
Exist only when SYMBIAN_TRACE_SQL_FUNC is enabled.

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
//Works only when SYMBIAN_TRACE_SQL_FUNC is enabled. Otherwise it evaluates to zero
#define SQLUTRACE_PROFILER(x) TSqlUTraceProfiler _profiler(x, __PRETTY_FUNCTION__)

#else

#define SQLUTRACE_PROFILER(x) do {} while(0)
#endif //SYMBIAN_TRACE_SQL_FUNC

#if defined SYMBIAN_TRACE_SQL_EVENTS && defined SQL_ENABLE_TRACE

//This macro should be used to trace events occurring within Symbian SQL.
//To use the macro, insert the full UTF statement as the macro parameter
//Works only when SYMBIAN_TRACE_SQL_EVENTS is enabled. Otherwise it evaluates to zero
#define SYMBIAN_TRACE_SQL_EVENTS_ONLY(c) c
const TPtrC8 GetIPCFuncStr(TInt aFunction);

#else

#define SYMBIAN_TRACE_SQL_EVENTS_ONLY(c) do {} while(0)
#endif //SYMBIAN_TRACE_SQL_EVENTS
////////////////////////////////////////////////////////////////////////////////////////////////////////////	
#endif // UTRACESQL_H
