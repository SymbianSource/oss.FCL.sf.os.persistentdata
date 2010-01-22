@echo off
rem
rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
rem All rights reserved.
rem This component and the accompanying materials are made available
rem under the terms of "Eclipse Public License v1.0"
rem which accompanies this distribution, and is available
rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
rem
rem Initial Contributors:
rem Nokia Corporation - initial contribution.
rem
rem Contributors:
rem
rem Description:
rem

@rem This call is so that the batch file can be used unaltered by the ONB
@if exist \textshell.bat call \textshell.bat

@if not exist %EPOCROOT%EPOC32\DATA\Z\SYSTEM\TEST\LOGSERVCNTMATCH.RSC goto notBuilt
@if exist %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC.TMP goto TmpFilesExist

@rem Save original ( matching disabled ) rsc files
@rename %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC LOGSERV.RSC.TMP
@rename %EPOCROOT%EPOC32\RELEASE\WINSCW\UREL\Z\private\101f401d\LOGSERV.RSC LOGSERV.RSC.TMP
@rename %EPOCROOT%EPOC32\data\Z\Private\101f401d\logserv.rsc logserv.rsc.tmp

@rem Copy over new ( matching enabled ) rsc file
@copy /y %EPOCROOT%EPOC32\DATA\Z\SYSTEM\TEST\LOGSERVCNTMATCH.RSC %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC
@copy /y %EPOCROOT%EPOC32\DATA\Z\SYSTEM\TEST\LOGSERVCNTMATCH.RSC %EPOCROOT%EPOC32\RELEASE\WINSCW\UREL\Z\private\101f401d\LOGSERV.RSC
@copy /y %EPOCROOT%EPOC32\DATA\Z\SYSTEM\TEST\LOGSERVCNTMATCH.RSC %EPOCROOT%EPOC32\data\Z\Private\101f401d\logserv.rsc
@goto :EOF

:TmpFilesExist
@echo Tmp files exist %0 already called
@goto :EOF

:notBuilt
@echo %EPOCROOT%EPOC32\DATA\Z\SYSTEM\TEST\LOGSERVCNTMATCH.RSC doesn't exist, test files not exported.
