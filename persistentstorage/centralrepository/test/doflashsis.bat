@echo off
rem For flash test stub (centrepswiteststub.sis) see testexecute/SWI/data/dosis.bat
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
@echo on

rem Make and sign the RF1 package.
makesis RF1.pkg
signsis -S RF1.sis RF1.sis testexecute\SWI\data\certstore\centreproot.pem testexecute\SWI\data\certstore\centreproot.key

makesis RF2.pkg
signsis -S RF2.sis RF2.sis testexecute\SWI\data\certstore\centreproot.pem testexecute\SWI\data\certstore\centreproot.key

makesis RF3.pkg
signsis -S RF3.sis RF3.sis testexecute\SWI\data\certstore\centreproot.pem testexecute\SWI\data\certstore\centreproot.key

makesis RF4.pkg
signsis -S RF4.sis RF4.sis testexecute\SWI\data\certstore\centreproot.pem testexecute\SWI\data\certstore\centreproot.key
