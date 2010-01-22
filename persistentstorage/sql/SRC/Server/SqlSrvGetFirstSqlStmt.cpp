// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include "sqlite3.h"

//This function searches aString argument for ';' occurences.
//Every time when it finds a ';' character, the function places a 0 right after the ';' and
//tests the just created, zero-terminated substring if it is a comlpete SQL statement.
//
//If it is a SQL statement, the function replaces the found ';' character with 0 and returns the just created
//zero-terminated substring.Also the function modifies aString argument to point right after the found
//SQL string. If it is not SQL statement, the function will continue the searching.
//
//If there is no ';' inside aString argument, the function returns the same string as a return result and
//modifies aString argument - sets it to TPtr(NULL, 0, 0).
//
//The function expects aString argument to be zero-terminated.
TPtrC GetFirstSqlStmt(TPtr& aString)
	{
	const TChar KDelimitier(';');
	TPtr str(const_cast <TUint16*> (aString.Ptr()), aString.Length(), aString.Length());
	TInt afterDelimitierPos = 0;
	TInt pos;
	while((pos = str.Locate(KDelimitier) + 1) > 0 && pos < str.Length())
		{
		//There is a possibility that the string which terminates with the found ';' is SQL statement.
		//Zero terminate the string placing a zero right after ';' character and test it using sqlite3_complete16()
		//call. If it is not SQL string, restore the original character and continue searching.
		afterDelimitierPos += pos;
		TChar ch = aString[afterDelimitierPos];
		aString[afterDelimitierPos] = 0;
		TInt res = sqlite3_complete16(aString.Ptr());
		aString[afterDelimitierPos] = ch;
		if(res)
			{
			str.Set(const_cast <TUint16*> (aString.Ptr()), afterDelimitierPos, afterDelimitierPos);	
			//Replace the found ';' character with 0.
			str[afterDelimitierPos - 1] = 0;
			aString.Set(const_cast <TUint16*> (aString.Ptr()) + afterDelimitierPos, aString.Length() - afterDelimitierPos, aString.Length() - afterDelimitierPos);
			return str;
			}
		str.Set(const_cast <TUint16*> (str.Ptr()) + pos, str.Length() - pos, str.Length() - pos);	
		}
	//aString argument does not contain valid SQL statement or there is no ';' character inside aString.
	//Set aString to TPtr(NULL, 0, 0) and return the original string.
	aString.Set(NULL, 0, 0);
	return str;
	}
