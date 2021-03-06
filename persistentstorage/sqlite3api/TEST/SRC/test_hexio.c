/*
** 2007 April 6
**
** Portions Copyright (c) 2008 Nokia Corporation and/or its subsidiaries. All rights reserved.
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing all sorts of SQLite interfaces.  This code
** implements TCL commands for reading and writing the binary
** database files and displaying the content of those files as
** hexadecimal.  We could, in theory, use the built-in "binary"
** command of TCL to do a lot of this, but there are some issues
** with historical versions of the "binary" command.  So it seems
** easier and safer to build our own mechanism.
**
** $Id: test_hexio.c,v 1.7 2008/05/12 16:17:42 drh Exp $
*/
#include "sqliteInt.h"
#include "tcl.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/param.h>

extern char* FullFilePath(char* aPath, const char* aFileName);

/* Symbian OS - often TCL scripts attempt to open a file just using the file name without path.
   This is not working on the hardware. The function bellow will contstruct the full file name. */
extern char* GetFullFilePath(char* aPath, const char* aFileName)
	{
	return FullFilePath(aPath, aFileName);
	}

/*
** Convert binary to hex.  The input zBuf[] contains N bytes of
** binary data.  zBuf[] is 2*n+1 bytes long.  Overwrite zBuf[]
** with a hexadecimal representation of its original binary input.
*/
void sqlite3TestBinToHex(unsigned char *zBuf, int N){
  const unsigned char zHex[] = "0123456789ABCDEF";
  int i, j;
  unsigned char c;
  i = N*2;
  zBuf[i--] = 0;
  for(j=N-1; j>=0; j--){
    c = zBuf[j];
    zBuf[i--] = zHex[c&0xf];
    zBuf[i--] = zHex[c>>4];
  }
  assert( i==-1 );
}

/*
** Convert hex to binary.  The input zIn[] contains N bytes of
** hexadecimal.  Convert this into binary and write aOut[] with
** the binary data.  Spaces in the original input are ignored.
** Return the number of bytes of binary rendered.
*/
int sqlite3TestHexToBin(const unsigned char *zIn, int N, unsigned char *aOut){
  const unsigned char aMap[] = {
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     1, 2, 3, 4, 5, 6, 7, 8,  9,10, 0, 0, 0, 0, 0, 0,
     0,11,12,13,14,15,16, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0,11,12,13,14,15,16, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  };
  int i, j;
  int hi=1;
  unsigned char c;

  for(i=j=0; i<N; i++){
    c = aMap[zIn[i]];
    if( c==0 ) continue;
    if( hi ){
      aOut[j] = (c-1)<<4;
      hi = 0;
    }else{
      aOut[j++] |= c-1;
      hi = 1;
    }
  }
  return j;
}


/*
** Usage:   hexio_read  FILENAME  OFFSET  AMT
**
** Read AMT bytes from file FILENAME beginning at OFFSET from the
** beginning of the file.  Convert that information to hexadecimal
** and return the resulting HEX string.
*/
static int hexio_read(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
  int offset;
  int amt, got;
  const char *zFile;
  unsigned char *zBuf;
  FILE *in;
  char fnamebuf[MAXPATHLEN + 1];

  if( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET AMT");
    return TCL_ERROR;
  }
  if( Tcl_GetIntFromObj(interp, objv[2], &offset) ) return TCL_ERROR;
  if( Tcl_GetIntFromObj(interp, objv[3], &amt) ) return TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zBuf = sqlite3_malloc( amt*2+1 );
  if( zBuf==0 ){
    return TCL_ERROR;
  }
  if(GetFullFilePath(fnamebuf, zFile) == 0)
    return TCL_ERROR;
  in = fopen(fnamebuf, "rb");
  if( in==0 ){
    in = fopen(fnamebuf, "r");
  }
  if( in==0 ){
    Tcl_AppendResult(interp, "cannot open input file ", fnamebuf, 0);
    return TCL_ERROR;
  }
  fseek(in, offset, SEEK_SET);
  got = fread(zBuf, 1, amt, in);
  fclose(in);
  if( got<0 ){
    got = 0;
  }
  sqlite3TestBinToHex(zBuf, got);
  Tcl_AppendResult(interp, zBuf, 0);
  sqlite3_free(zBuf);
  return TCL_OK;
}


/*
** Usage:   hexio_write  FILENAME  OFFSET  DATA
**
** Write DATA into file FILENAME beginning at OFFSET from the
** beginning of the file.  DATA is expressed in hexadecimal.
*/
static int hexio_write(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
  int offset;
  int nIn, nOut, written;
  const char *zFile;
  const unsigned char *zIn;
  unsigned char *aOut;
  FILE *out;
  char fnamebuf[MAXPATHLEN + 1];

  if( objc!=4 ){
    Tcl_WrongNumArgs(interp, 1, objv, "FILENAME OFFSET HEXDATA");
    return TCL_ERROR;
  }
  if( Tcl_GetIntFromObj(interp, objv[2], &offset) ) return TCL_ERROR;
  zFile = Tcl_GetString(objv[1]);
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[3], &nIn);
  aOut = sqlite3_malloc( nIn/2 );
  if( aOut==0 ){
    return TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, nIn, aOut);
  if(GetFullFilePath(fnamebuf, zFile) == 0)
    return TCL_ERROR;
  out = fopen(fnamebuf, "r+b");
  if( out==0 ){
    out = fopen(fnamebuf, "r+");
  }
  if( out==0 ){
    Tcl_AppendResult(interp, "cannot open output file ", fnamebuf, 0);
    return TCL_ERROR;
  }
  fseek(out, offset, SEEK_SET);
  written = fwrite(aOut, 1, nOut, out);
  sqlite3_free(aOut);
  fclose(out);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(written));
  return TCL_OK;
}

/*
** USAGE:   hexio_get_int   HEXDATA
**
** Interpret the HEXDATA argument as a big-endian integer.  Return
** the value of that integer.  HEXDATA can contain between 2 and 8
** hexadecimal digits.
*/
static int hexio_get_int(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
  int val;
  int nIn, nOut;
  const unsigned char *zIn;
  unsigned char *aOut;
  unsigned char aNum[4];

  if( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEXDATA");
    return TCL_ERROR;
  }
  zIn = (const unsigned char *)Tcl_GetStringFromObj(objv[1], &nIn);
  aOut = sqlite3_malloc( nIn/2 );
  if( aOut==0 ){
    return TCL_ERROR;
  }
  nOut = sqlite3TestHexToBin(zIn, nIn, aOut);
  if( nOut>=4 ){
    memcpy(aNum, aOut, 4);
  }else{
    memset(aNum, 0, sizeof(aNum));
    memcpy(&aNum[4-nOut], aOut, nOut);
  }
  sqlite3_free(aOut);
  val = (aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
  Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
  return TCL_OK;
}


/*
** USAGE:   hexio_render_int16   INTEGER
**
** Render INTEGER has a 16-bit big-endian integer in hexadecimal.
*/
static int hexio_render_int16(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
  int val;
  unsigned char aNum[10];

  if( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    return TCL_ERROR;
  }
  if( Tcl_GetIntFromObj(interp, objv[1], &val) ) return TCL_ERROR;
  aNum[0] = val>>8;
  aNum[1] = val;
  sqlite3TestBinToHex(aNum, 2);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 4));
  return TCL_OK;
}


/*
** USAGE:   hexio_render_int32   INTEGER
**
** Render INTEGER has a 32-bit big-endian integer in hexadecimal.
*/
static int hexio_render_int32(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
  int val;
  unsigned char aNum[10];

  if( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "INTEGER");
    return TCL_ERROR;
  }
  if( Tcl_GetIntFromObj(interp, objv[1], &val) ) return TCL_ERROR;
  aNum[0] = val>>24;
  aNum[1] = val>>16;
  aNum[2] = val>>8;
  aNum[3] = val;
  sqlite3TestBinToHex(aNum, 4);
  Tcl_SetObjResult(interp, Tcl_NewStringObj((char*)aNum, 8));
  return TCL_OK;
}

/*
** USAGE:  utf8_to_utf8  HEX
**
** The argument is a UTF8 string represented in hexadecimal.
** The UTF8 might not be well-formed.  Run this string through
** sqlite3Utf8to8() convert it back to hex and return the result.
*/
static int utf8_to_utf8(
  void * clientData,
  Tcl_Interp *interp,
  int objc,
  Tcl_Obj *CONST objv[]
){
#ifdef SQLITE_DEBUG
  int n;
  int nOut;
  const unsigned char *zOrig;
  unsigned char *z;
  if( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "HEX");
    return TCL_ERROR;
  }
  zOrig = (unsigned char *)Tcl_GetStringFromObj(objv[1], &n);
  z = sqlite3_malloc( n+3 );
  n = sqlite3TestHexToBin(zOrig, n, z);
  z[n] = 0;
  nOut = sqlite3Utf8To8(z);
  sqlite3TestBinToHex(z,nOut);
  Tcl_AppendResult(interp, (char*)z, 0);
  sqlite3_free(z);
#endif
  return TCL_OK;
}


/*
** Register commands with the TCL interpreter.
*/
int Sqlitetest_hexio_Init(Tcl_Interp *interp){
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
  } aObjCmd[] = {
     { "hexio_read",                   hexio_read            },
     { "hexio_write",                  hexio_write           },
     { "hexio_get_int",                hexio_get_int         },
     { "hexio_render_int16",           hexio_render_int16    },
     { "hexio_render_int32",           hexio_render_int32    },
     { "utf8_to_utf8",                 utf8_to_utf8          },
  };
  int i;
  for(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, 0, 0);
  }
  return TCL_OK;
}
