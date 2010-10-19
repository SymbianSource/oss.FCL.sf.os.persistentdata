// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <e32test.h>
#include "t_dbcmdlineutil.h"

static void GetCmdLine(RTest& aTest, const TDesC& aTestName, TDes& aCmdLine)
	{
	User::CommandLine(aCmdLine);
	aCmdLine.TrimAll();
	if(aCmdLine.Length() == 0)
		{
		aTest.Printf(_L("Usage: %S [/drv=<drive letter>:] [/logfile=<log file name>]\r\n"), &aTestName);
		return;
		}
	aCmdLine.Append(TChar('/'));
	}

static void ExtractCmdLineParams(TDes& aCmdLine,  RArray<TPtrC>& aPrmNames, RArray<TPtrC>& aPrmValues)
	{
	aPrmNames.Reset();	
	aPrmValues.Reset();	
	
	enum TState{EWaitPrmStart, EReadPrmName, EReadPrmValue};
	TState state = EWaitPrmStart;
	TInt startPos = -1;
	TPtr prmName(0, 0);
	TPtr prmValue(0, 0);
	
	aCmdLine.Append(TChar('/'));
	
	for(TInt i=0;i<aCmdLine.Length();++i)
		{
		switch(state)
			{
			case EWaitPrmStart:
				if(aCmdLine[i] == TChar('/'))
					{
					startPos = i + 1;
					prmName.Zero();
					state = EReadPrmName;
					}
				break;
			case EReadPrmName:
				if(aCmdLine[i] == TChar('='))
					{
					TPtr p = aCmdLine.MidTPtr(startPos, i - startPos);
					prmName.Set(p);
					prmName.TrimRight();
					startPos = i + 1;
					prmValue.Zero();
					state = EReadPrmValue;
					}
				break;
			case EReadPrmValue:
				if(aCmdLine[i] == TChar('/'))
					{
					TPtr p = aCmdLine.MidTPtr(startPos, i - startPos);
					prmValue.Set(p);
					prmValue.Trim();
					startPos = i + 1;
					aPrmNames.Append(prmName);
					aPrmValues.Append(prmValue);
					prmName.Zero();
					prmValue.Zero();
					state = EReadPrmName;
					}
				break;
			default:
				break;
			}
		}
	}

static void ExtractParamNamesAndValues(const RArray<TPtrC>& aPrmNames, const RArray<TPtrC>& aPrmValues, TCmdLineParams& aCmdLineParams)
	{
	__ASSERT_ALWAYS(aPrmNames.Count() == aPrmValues.Count(), User::Invariant());
	
	aCmdLineParams.SetDefaults();
	
	for(TInt i=0;i<aPrmNames.Count();++i)
		{
		if(aPrmNames[i].CompareF(_L("drv")) == 0)
			{
			if(aPrmValues[i].Length() == 2 && aPrmValues[i][1] == TChar(':'))
				{
				TChar ch(aPrmValues[i][0]);
				ch.LowerCase();
				if(ch >= TChar('a') && ch <= TChar('z'))
					aCmdLineParams.iDriveName.Copy(aPrmValues[i]);
				}
			}
		else if(aPrmNames[i].CompareF(_L("logfile")) == 0)
			{
			aCmdLineParams.iLogFileName.Copy(aPrmValues[i]);
			}
		}
	}

void GetCmdLineParams(RTest& aTest, const TDesC& aTestName, TCmdLineParams& aCmdLineParams)
	{
	TBuf<200> cmdLine;
	GetCmdLine(aTest, aTestName, cmdLine);
	RArray<TPtrC> prmNames;
	RArray<TPtrC> prmValues;
	ExtractCmdLineParams(cmdLine, prmNames, prmValues);
	ExtractParamNamesAndValues(prmNames, prmValues, aCmdLineParams);
	prmValues.Close();
	prmNames.Close();
	aTest.Printf(_L("--PRM--Database drive: %S\r\n"), &aCmdLineParams.iDriveName);
	if(aCmdLineParams.iLogFileName.Length() > 0)
		{
		aTest.Printf(_L("--PRM--Log file name: %S\r\n"), &aCmdLineParams.iLogFileName);
		}
	else
		{
		aTest.Printf(_L("--PRM--Test output: to screen only\r\n"));
		}
	}

void PrepareDbName(const TDesC& aDeafultDbName, const TDriveName& aDriveName, TDes& aDbName)
	{
	TParse parse;
	parse.Set(aDriveName, &aDeafultDbName, 0);
	const TDesC& dbFilePath = parse.FullName();
	aDbName.Copy(dbFilePath);
	}

void LogConfig(RFile& aLogFile, const TCmdLineParams& aCmdLineParams)
	{
	TBuf8<100> buf;
	buf.Format(_L8("Database drive:%S\r\n"), &aCmdLineParams.iDriveName);
	(void)aLogFile.Write(buf);
	buf.Format(_L8("\r\n\r\n"));
	(void)aLogFile.Write(buf);
	}
