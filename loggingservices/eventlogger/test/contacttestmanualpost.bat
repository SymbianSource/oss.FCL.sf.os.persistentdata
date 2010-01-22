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

if not exist %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC.TMP goto noTmpFiles

rem Delete new ( matching disabled ) rsc files
del /f %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC
del /f %EPOCROOT%EPOC32\RELEASE\WINSCW\UREL\Z\private\101f401d\LOGSERV.RSC
del /f %EPOCROOT%EPOC32\data\Z\Private\101f401d\logserv.rsc

rem Restore original ( matching disabled ) rsc files
rename %EPOCROOT%EPOC32\RELEASE\WINSCW\UDEB\Z\private\101f401d\LOGSERV.RSC.TMP LOGSERV.RSC
rename %EPOCROOT%EPOC32\RELEASE\WINSCW\UREL\Z\private\101f401d\LOGSERV.RSC.TMP LOGSERV.RSC
rename %EPOCROOT%EPOC32\data\Z\Private\101f401d\logserv.rsc.tmp logserv.rsc
goto :EOF

:noTmpFiles
@echo No temp files exist, pre bat file hasn't been run or %0 already called
