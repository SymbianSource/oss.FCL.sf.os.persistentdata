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
@echo on
rem The stub..
makesis -s centrepswiteststub.pkg

rem Make and sign the KS12 package. This modifies the KS1.2 keyspace, which
rem is a user mod on the ROM keyspace K1.
makesis KS12.pkg
signsis -S KS12.sis KS12s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KS14 package.
makesis KS14.pkg
signsis -S KS14.sis KS14s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KS2 package. This creates the KS2 keyspace, which
rem is new.
makesis KS2.pkg
signsis -S KS2.sis KS2s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KS22 package. This creates the KS22 keyspace, which
rem upgrades KS21 (user modified KS2).
makesis KS22.pkg
signsis -S KS22.sis KS22s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KS3 package. This package will fail to install
rem because the referenced keyspace file is not in the ROM stub.
makesis KS3.pkg
signsis -S KS3.sis KS3s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KS4 package. This keyspace contains various capabilities
rem which we'll test don't change on a further install.
makesis KS4.pkg
signsis -S KS4.sis KS4s.sis certstore\centreproot.pem certstore\centreproot.key

rem Installs on top of the above, you might think the capabilities in the
rem above would be lost, but no - they are persisted before the caps in this
rem package can splat them.
makesis KS41.pkg
signsis -S KS41.sis KS41s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP4 package. This keyspace contains various capabilities
rem which we'll test don't change on a further install.
makesis KP4.pkg
signsis -S KP4.sis KP4s.sis certstore\centreproot.pem certstore\centreproot.key

rem Installs on top of the above, you might think the capabilities in the
rem above would be lost, but no - they are persisted before the caps in this
rem package can splat them.
makesis KP41.pkg
signsis -S KP41.sis KP41s.sis certstore\centreproot.pem certstore\centreproot.key

rem SP filenull for uninstalling KP4.
makesis KPS4.pkg
signsis -S KPS4.sis KPS4s.sis certstore\centreproot.pem certstore\centreproot.key

rem Installs on top of the ROM (51551554)
makesis KS51.pkg
signsis -S KS51.sis KS51s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP2 package. This creates the KP2 keyspace, which
rem is new.
makesis KP2.pkg
signsis -S KP2.sis KP2s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP22 package. This creates the KP22 keyspace, which
rem upgrades KP21 (user modified KP2).
makesis KP22.pkg
signsis -S KP22.sis KP22s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP23 package.
makesis KP23.pkg
signsis -S KP23.sis KP23s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP12 package.
makesis KP12.pkg
signsis -S KP12.sis KP12s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP14 package.
makesis KP14.pkg
signsis -S KP14.sis KP14s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KPS14 standard patch, this SP is installed with a
rem file-null so the 51551551 keyspace file can be removed.
makesis KPS14.pkg
signsis -S KPS14.sis KPS14s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP3 package. This package will fail to install
rem because the referenced keyspace file is not in the ROM stub.
makesis KP3.pkg
signsis -S KP3.sis KP3s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP62 package.
makesis KP62.pkg
signsis -S KP62.sis KP62s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KP62 package.
makesis KS62.pkg
signsis -S KS62.sis KS62s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KPS64 package.
makesis KPS64.pkg
signsis -S KPS64.sis KPS64s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the KSX package. This is the embedded sis that goes in AppX.sis
makesis KSX.pkg
signsis -S KSX.sis KSXs.sis certstore\centreproot.pem certstore\centreproot.key

rem Make the AppX package for the application. This is the sis which contains the 
rem embedded sis that has the application repository as a Centrep patch. This sis
rem is self-signed and conditonally installs the exe file depending on the environment
makesis APPX.pkg
signsis -S APPX.sis APPXs.sis certstore\appx.cer certstore\appx.key

rem Make and sign the KSD package. This contains the corrupt KSD keyspace.
makesis KSD.pkg
signsis -S KSD.sis KSDs.sis certstore\centreproot.pem certstore\centreproot.key

rem Make the AppY,1,2 packages for the application. These sis files contain the 
rem embedded sis that has the application repository as a Centrep patch. This sis
rem is self-signed and conditonally installs the exe file depending on the environment
rem These SIS files are used to test a standard installation

rem The following packages are used for the APP-SP test

rem Make and sign the KSY package. This is the embedded PU sis that goes in AppY.sis
makesis KSY.pkg
signsis -S KSY.sis KSYs.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the APPY package. This package will install an APP exe
rem a keyspace in an embedded PU package and an SP package to be used for the uninstall
makesis APPY.pkg
signsis -S APPY.sis APPYs.sis certstore\appx.cer certstore\appx.key


rem Make and sign the KSY1 package. This is the embedded PU sis that goes in AppY1.sis
makesis KSY1.pkg
signsis -S KSY1.sis KSY1s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the APPY1 package. This package will install the APP exe
rem a new version of the keyspace in an embedded PU package and an SP package 
rem to be used for the uninstall
makesis APPY1.pkg
signsis -S APPY1.sis APPY1s.sis certstore\appx.cer certstore\appx.key


rem Make and sign the KSY2 package. This is the embedded PU sis that goes in AppY2.sis
makesis KSY2.pkg
signsis -S KSY2.sis KSY2s.sis certstore\centreproot.pem certstore\centreproot.key

rem Make and sign the APPY2 package. This package will install the APP exe
rem a new version of the keyspace in an embedded PU package and an SP package 
rem to be used for the uninstall
makesis APPY2.pkg
signsis -S APPY2.sis APPY2s.sis certstore\appx.cer certstore\appx.key

