@echo off
rem
rem Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

@if not exist "%1" md "%1"
@if not exist "%1" goto :InvalidFolder
@if not exist "%2" goto :MissingCert
@if not exist "%3" goto :MissingKey


makesis KS401.pkg KS401.sis
signsis -S KS401.sis KS401s.sis %2 %3
makesis KP401.pkg KP401.sis
signsis -S KP401.sis KP401s.sis %2 %3
makesis KS402.pkg KS402.sis
signsis -S KS402.sis KS402s.sis %2 %3
makesis KP402.pkg KP402.sis
signsis -S KP402.sis KP402s.sis %2 %3
makesis KS403.pkg KS403.sis
signsis -S KS403.sis KS403s.sis %2 %3
makesis KP403.pkg KP403.sis
signsis -S KP403.sis KP403s.sis %2 %3
makesis KS404.pkg KS404.sis
signsis -S KS404.sis KS404s.sis %2 %3
makesis KP404.pkg KP404.sis
signsis -S KP404.sis KP404s.sis %2 %3
makesis KS405.pkg KS405.sis
signsis -S KS405.sis KS405s.sis %2 %3
makesis KP405.pkg KP405.sis
signsis -S KP405.sis KP405s.sis %2 %3
makesis KS406.pkg KS406.sis
signsis -S KS406.sis KS406s.sis %2 %3
makesis KP406.pkg KP406.sis
signsis -S KP406.sis KP406s.sis %2 %3
makesis KS407.pkg KS407.sis
signsis -S KS407.sis KS407s.sis %2 %3
makesis KP407.pkg KP407.sis
signsis -S KP407.sis KP407s.sis %2 %3
makesis KS411.pkg KS411.sis
signsis -S KS411.sis KS411s.sis %2 %3
makesis KP411.pkg KP411.sis
signsis -S KP411.sis KP411s.sis %2 %3
makesis KS412.pkg KS412.sis
signsis -S KS412.sis KS412s.sis %2 %3
makesis KP412.pkg KP412.sis
signsis -S KP412.sis KP412s.sis %2 %3
makesis KS413.pkg KS413.sis
signsis -S KS413.sis KS413s.sis %2 %3
makesis KP413.pkg KP413.sis
signsis -S KP413.sis KP413s.sis %2 %3
makesis KS414.pkg KS414.sis
signsis -S KS414.sis KS414s.sis %2 %3
makesis KP414.pkg KP414.sis
signsis -S KP414.sis KP414s.sis %2 %3
makesis KS415.pkg KS415.sis
signsis -S KS415.sis KS415s.sis %2 %3
makesis KP415.pkg KP415.sis
signsis -S KP415.sis KP415s.sis %2 %3
makesis KS416.pkg KS416.sis
signsis -S KS416.sis KS416s.sis %2 %3
makesis KP416.pkg KP416.sis
signsis -S KP416.sis KP416s.sis %2 %3
makesis KS417.pkg KS417.sis
signsis -S KS417.sis KS417s.sis %2 %3
makesis KP417.pkg KP417.sis
signsis -S KP417.sis KP417s.sis %2 %3
makesis KS421.pkg KS421.sis
signsis -S KS421.sis KS421s.sis %2 %3
makesis KP421.pkg KP421.sis
signsis -S KP421.sis KP421s.sis %2 %3
makesis KS422.pkg KS422.sis
signsis -S KS422.sis KS422s.sis %2 %3
makesis KP422.pkg KP422.sis
signsis -S KP422.sis KP422s.sis %2 %3
makesis KS423.pkg KS423.sis
signsis -S KS423.sis KS423s.sis %2 %3
makesis KP423.pkg KP423.sis
signsis -S KP423.sis KP423s.sis %2 %3
makesis KS431.pkg KS431.sis
signsis -S KS431.sis KS431s.sis %2 %3
makesis KP431.pkg KP431.sis
signsis -S KP431.sis KP431s.sis %2 %3
makesis KS432.pkg KS432.sis
signsis -S KS432.sis KS432s.sis %2 %3
makesis KP432.pkg KP432.sis
signsis -S KP432.sis KP432s.sis %2 %3
makesis KS433.pkg KS433.sis
signsis -S KS433.sis KS433s.sis %2 %3
makesis KP433.pkg KP433.sis
signsis -S KP433.sis KP433s.sis %2 %3


echo f | XCOPY /fry *.sis %1\
del /f *.sis
@goto :EOF

:InvalidFolder
@echo ERROR: Can't create folder %1
@goto :EOF

:MissingCert
@echo ERROR: Missing certificate %2
@goto :EOF

:MissingKey
@echo ERROR: Missing certificate key %3
@goto :EOF