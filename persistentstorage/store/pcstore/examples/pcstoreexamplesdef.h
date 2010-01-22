// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#if !defined(__PCSTOREEXAMPLESDEF_H__)
#define __PCSTOREEXAMPLESDEF_H__

using namespace PCStore;

const TInt8 KRefTInt8_1 = -52;
const TReal64 KRefTReal64 = 101255681.866786;
const TInt8 KRefTInt8_2 = -99;
const TInt16 KRefTInt16 = -6745;
const TReal32 KRefTReal32 = -3.14159f;
const TUint8 KRefTUint8 = 171u;
const TInt32 KRefTInt32 = -5415642;
const TUint16 KRefTUint16 = 52714u;
const TUint32 KRefTUint32 = 3168716372u;
const TUid KRefTUid={65};

// Characters to form the 8-bit descriptor.
const TUint8 RefData8[] =
{
0xFE, 0x45, 0x6E, 0x67, 0x6C, 0x69, 0x73, 0x68, 0x3A, 0x20, 0x4A, 0x61, 0x63, 0x6B, 0x64, 0x61,  // .English: Jackda
0x77, 0x73, 0x20, 0x6C, 0x6F, 0x76, 0x65, 0x20, 0x6D, 0x79, 0x20, 0x62, 0x69, 0x67, 0x20, 0x73,  // ws love my big s
0x70, 0x68, 0x69, 0x6E, 0x78, 0x20, 0x6F, 0x66, 0x20, 0x71, 0x75, 0x61, 0x72, 0x74, 0x7A, 0x2E,  // phinx of quartz.
0x20, 0x50, 0x61, 0x63, 0x6B, 0x20, 0x6D, 0x79, 0x20, 0x62, 0x6F, 0x78, 0x20, 0x77, 0x69, 0x74,  //  Pack my box wit
0x68, 0x20, 0x66, 0x69, 0x76, 0x65, 0x20, 0x64, 0x6F, 0x7A, 0x65, 0x6E, 0x20, 0x6C, 0x69, 0x71,  // h five dozen liq
0x75, 0x6F, 0x72, 0x20, 0x6A, 0x75, 0x67, 0x73, 0x2E, 0x06, 0x06, 0x47, 0x65, 0x72, 0x6D, 0x61,  // uor jugs...Germa
0x6E, 0x3A, 0x20, 0x44, 0x61, 0xDF, 0x20, 0x43, 0x2B, 0x2B, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20,  // n: Da� C++ sich 
0x73, 0x6F, 0x20, 0x73, 0x74, 0x61, 0x72, 0x6B, 0x20, 0x76, 0x65, 0x72, 0x62, 0x72, 0x65, 0x69,  // so stark verbrei
0x74, 0x65, 0x74, 0x20, 0x68, 0x61, 0x74, 0x2C, 0x20, 0x6D, 0x61, 0x67, 0x20, 0x76, 0x6F, 0x72,  // tet hat, mag vor
0x20, 0x61, 0x6C, 0x6C, 0x65, 0x6E, 0x20, 0x44, 0x69, 0x6E, 0x67, 0x65, 0x6E, 0x20, 0x7A, 0x77,  //  allen Dingen zw
0x65, 0x69, 0x20, 0x47, 0x72, 0xFC, 0x6E, 0x64, 0x65, 0x20, 0x68, 0x61, 0x62, 0x65, 0x6E, 0x3A,  // ei Gr�nde haben:
0x06, 0x44, 0x69, 0x65, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69, 0x65, 0x6E,  // .Die objektorien
0x74, 0x69, 0x65, 0x72, 0x74, 0x65, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69,  // tierte Programmi
0x65, 0x72, 0x75, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x74, 0x20, 0x73, 0x69, 0x63, 0x68, 0x65, 0x72,  // erung ist sicher
0x20, 0x65, 0x69, 0x6E, 0x65, 0x20, 0x61, 0x75, 0x73, 0x67, 0x65, 0x7A, 0x65, 0x69, 0x63, 0x68,  //  eine ausgezeich
0x6E, 0x65, 0x74, 0x65, 0x20, 0x41, 0x6C, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x74, 0x69, 0x76, 0x65,  // nete Alternative
0x06, 0x7A, 0x75, 0x20, 0x64, 0x65, 0x6E, 0x20, 0x74, 0x72, 0x61, 0x64, 0x69, 0x74, 0x69, 0x6F,  // .zu den traditio
0x6E, 0x65, 0x6C, 0x6C, 0x65, 0x6E, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69,  // nellen Programmi
0x65, 0x72, 0x74, 0x65, 0x63, 0x68, 0x6E, 0x69, 0x6B, 0x65, 0x6E, 0x2C, 0x20, 0x6D, 0xF6, 0x67,  // ertechniken, m�g
0x6C, 0x69, 0x63, 0x68, 0x65, 0x72, 0x77, 0x65, 0x69, 0x73, 0x65, 0x20, 0x66, 0xFC, 0x72, 0x20,  // licherweise f�r 
0x67, 0x72, 0x6F, 0xDF, 0x65, 0x20, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x2D, 0x50,  // gro�e Software-P
0x72, 0x6F, 0x6A, 0x65, 0x6B, 0x74, 0x65, 0x06, 0x67, 0x65, 0x67, 0x65, 0x6E, 0x77, 0xE4, 0x72,  // rojekte.gegenw�r
0x74, 0x69, 0x67, 0x20, 0x64, 0x69, 0x65, 0x20, 0x65, 0x69, 0x6E, 0x7A, 0x69, 0x67, 0x65, 0x20,  // tig die einzige 
0x4D, 0xF6, 0x67, 0x6C, 0x69, 0x63, 0x68, 0x6B, 0x65, 0x69, 0x74, 0x2C, 0x20, 0x64, 0x69, 0x65,  // M�glichkeit, die
0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x65, 0x20, 0x73, 0x6F, 0x77, 0x6F, 0x68,  //  Programme sowoh
0x6C, 0x20, 0x65, 0x66, 0x66, 0x65, 0x6B, 0x74, 0x69, 0x76, 0x20, 0x73, 0x63, 0x68, 0x72, 0x65,  // l effektiv schre
0x69, 0x62, 0x65, 0x6E, 0x20, 0x61, 0x6C, 0x73, 0x20, 0x61, 0x75, 0x63, 0x68, 0x20, 0x77, 0x61,  // iben als auch wa
0x72, 0x74, 0x65, 0x6E, 0x06, 0x7A, 0x75, 0x20, 0x6B, 0xF6, 0x6E, 0x6E, 0x65, 0x6E, 0x20, 0x28,  // rten.zu k�nnen (
0x75, 0x6E, 0x64, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69, 0x65, 0x6E, 0x74,  // und objektorient
0x69, 0x65, 0x72, 0x74, 0x65, 0x20, 0x56, 0x6F, 0x72, 0x67, 0x65, 0x68, 0x65, 0x6E, 0x73, 0x77,  // ierte Vorgehensw
0x65, 0x69, 0x73, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x6E, 0x64, 0x20, 0x61, 0x75, 0x63, 0x68, 0x20,  // eisen sind auch 
0x69, 0x6E, 0x20, 0x76, 0x69, 0x65, 0x6C, 0x65, 0x6E, 0x20, 0x61, 0x6E, 0x64, 0x65, 0x72, 0x65,  // in vielen andere
0x6E, 0x06, 0x57, 0x69, 0x73, 0x73, 0x65, 0x6E, 0x73, 0x63, 0x68, 0x61, 0x66, 0x74, 0x73, 0x62,  // n.Wissenschaftsb
0x65, 0x72, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x65, 0x72, 0x66, 0x6F, 0x6C, 0x67, 0x72,  // ereichen erfolgr
0x65, 0x69, 0x63, 0x68, 0x29, 0x2E, 0x20, 0x5A, 0x75, 0x6D, 0x20, 0x61, 0x6E, 0x64, 0x65, 0x72,  // eich). Zum ander
0x65, 0x6E, 0x20, 0x65, 0x6E, 0x74, 0x68, 0xE4, 0x6C, 0x74, 0x20, 0x43, 0x2B, 0x2B, 0x20, 0x64,  // en enth�lt C++ d
0x65, 0x6E, 0x20, 0x6B, 0x6F, 0x6D, 0x70, 0x6C, 0x65, 0x74, 0x74, 0x65, 0x6E, 0x06, 0x53, 0x70,  // en kompletten.Sp
0x72, 0x61, 0x63, 0x68, 0x75, 0x6D, 0x66, 0x61, 0x6E, 0x67, 0x20, 0x76, 0x6F, 0x6E, 0x20, 0x43,  // rachumfang von C
0x2C, 0x20, 0x73, 0x6F, 0x20, 0x64, 0x61, 0xDF, 0x20, 0x66, 0xFC, 0x72, 0x20, 0x64, 0x69, 0x65,  // , so da� f�r die
0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69, 0x65, 0x72, 0x65, 0x72, 0x20, 0x65,  //  Programmierer e
0x69, 0x6E, 0x20, 0x22, 0x67, 0x6C, 0x65, 0x69, 0x74, 0x65, 0x6E, 0x64, 0x65, 0x72, 0x20, 0xDC,  // in "gleitender �
0x62, 0x65, 0x72, 0x67, 0x61, 0x6E, 0x67, 0x22, 0x20, 0x6D, 0xF6, 0x67, 0x6C, 0x69, 0x63, 0x68,  // bergang" m�glich
0x20, 0x69, 0x73, 0x74, 0x06, 0x75, 0x6E, 0x64, 0x20, 0x61, 0x6C, 0x6C, 0x65, 0x20, 0x43, 0x2D,  //  ist.und alle C-
0x52, 0x6F, 0x75, 0x74, 0x69, 0x6E, 0x65, 0x6E, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x43, 0x2D, 0x42,  // Routinen und C-B
0x69, 0x62, 0x6C, 0x69, 0x6F, 0x74, 0x68, 0x65, 0x6B, 0x65, 0x6E, 0x20, 0x77, 0x65, 0x69, 0x74,  // ibliotheken weit
0x65, 0x72, 0x20, 0x76, 0x65, 0x72, 0x77, 0x65, 0x6E, 0x64, 0x65, 0x74, 0x20, 0x77, 0x65, 0x72,  // er verwendet wer
0x64, 0x65, 0x6E, 0x20, 0x6B, 0xF6, 0x6E, 0x6E, 0x65, 0x6E, 0x2E, 0x06, 0x06, 0x53, 0x77, 0x65,  // den k�nnen...Swe
0x64, 0x69, 0x73, 0x68, 0x3A, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x20, 0xE4, 0x72, 0x20, 0x75, 0x74,  // dish: Java �r ut
0x76, 0x65, 0x63, 0x6B, 0x6C, 0x61, 0x64, 0x20, 0x61, 0x76, 0x20, 0x53, 0x75, 0x6E, 0x2E, 0x20,  // vecklad av Sun. 
0x46, 0x72, 0xE5, 0x6E, 0x20, 0x62, 0xF6, 0x72, 0x6A, 0x61, 0x6E, 0x20, 0x76, 0x61, 0x72, 0x20,  // Fr�n b�rjan var 
0x64, 0x65, 0x74, 0x20, 0x74, 0xE4, 0x6E, 0x6B, 0x74, 0x20, 0x61, 0x74, 0x74, 0x20, 0x73, 0x70,  // det t�nkt att sp
0x72, 0xE5, 0x6B, 0x65, 0x74, 0x20, 0x73, 0x6B, 0x75, 0x6C, 0x6C, 0x65, 0x20, 0x61, 0x6E, 0x76,  // r�ket skulle anv
0xE4, 0x6E, 0x64, 0x61, 0x73, 0x20, 0x69, 0x6E, 0x6F, 0x6D, 0x20, 0x65, 0x6C, 0x65, 0x6B, 0x74,  // �ndas inom elekt
0x72, 0x6F, 0x6E, 0x69, 0x6B, 0x20, 0x74, 0x69, 0x6C, 0x6C, 0x06, 0x74, 0x20, 0x65, 0x78, 0x20,  // ronik till.t ex 
0x62, 0x72, 0xF6, 0x64, 0x72, 0x6F, 0x73, 0x74, 0x61, 0x72, 0x20, 0x65, 0x6C, 0x6C, 0x65, 0x72,  // br�drostar eller
0x20, 0x74, 0x76, 0xE4, 0x74, 0x74, 0x6D, 0x61, 0x73, 0x6B, 0x69, 0x6E, 0x65, 0x72, 0x2E, 0x20,  //  tv�ttmaskiner. 
0x55, 0x74, 0x76, 0x65, 0x63, 0x6B, 0x6C, 0x61, 0x72, 0x6E, 0x61, 0x20, 0x69, 0x6E, 0x73, 0xE5,  // Utvecklarna ins�
0x67, 0x20, 0x73, 0x6E, 0x61, 0x62, 0x62, 0x74, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74,  // g snabbt att det
0x20, 0x6B, 0x72, 0xE4, 0x76, 0x64, 0x65, 0x73, 0x20, 0x65, 0x74, 0x74, 0x20, 0x70, 0x6C, 0x61,  //  kr�vdes ett pla
0x74, 0x74, 0x66, 0x6F, 0x72, 0x6D, 0x73, 0x6F, 0x62, 0x65, 0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65,  // ttformsoberoende
0x06, 0x73, 0x70, 0x72, 0xE5, 0x6B, 0x20, 0x66, 0xF6, 0x72, 0x20, 0x61, 0x74, 0x74, 0x20, 0x6B,  // .spr�k f�r att k
0x75, 0x6E, 0x6E, 0x61, 0x20, 0x74, 0x69, 0x6C, 0x6C, 0x67, 0x6F, 0x64, 0x6F, 0x73, 0x65, 0x20,  // unna tillgodose 
0x64, 0x65, 0x20, 0x6F, 0x6C, 0x69, 0x6B, 0x61, 0x20, 0x62, 0x65, 0x68, 0x6F, 0x76, 0x65, 0x6E,  // de olika behoven
0x20, 0x70, 0xE5, 0x20, 0x65, 0x6C, 0x65, 0x6B, 0x74, 0x72, 0x6F, 0x6E, 0x69, 0x6B, 0x6D, 0x61,  //  p� elektronikma
0x72, 0x6B, 0x6E, 0x61, 0x64, 0x65, 0x6E, 0x2E, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x73, 0x20, 0x73,  // rknaden. Javas s
0x6B, 0x61, 0x70, 0x61, 0x72, 0x65, 0x20, 0x4A, 0x61, 0x6D, 0x65, 0x73, 0x20, 0x47, 0x6F, 0x73,  // kapare James Gos
0x6C, 0x69, 0x6E, 0x67, 0x06, 0x62, 0xF6, 0x72, 0x6A, 0x61, 0x64, 0x65, 0x20, 0x61, 0x72, 0x62,  // ling.b�rjade arb
0x65, 0x74, 0x61, 0x20, 0x75, 0x74, 0x69, 0x66, 0x72, 0xE5, 0x6E, 0x20, 0x43, 0x2B, 0x2B, 0x20,  // eta utifr�n C++ 
0x6B, 0x6F, 0x6D, 0x70, 0x69, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x72, 0x6E, 0x2E, 0x20, 0x45, 0x66,  // kompilatiorn. Ef
0x74, 0x65, 0x72, 0x20, 0x65, 0x6E, 0x20, 0x74, 0x69, 0x64, 0x20, 0x69, 0x6E, 0x73, 0xE5, 0x67,  // ter en tid ins�g
0x20, 0x68, 0x61, 0x6E, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74, 0x06, 0x62, 0xE4, 0x73,  //  han att det.b�s
0x74, 0x61, 0x20, 0x76, 0x6F, 0x72, 0x65, 0x20, 0x61, 0x74, 0x74, 0x20, 0x73, 0x6B, 0x61, 0x70,  // ta vore att skap
0x61, 0x20, 0x65, 0x74, 0x74, 0x20, 0x6E, 0x79, 0x74, 0x74, 0x20, 0x73, 0x70, 0x72, 0xE5, 0x6B,  // a ett nytt spr�k
0x20, 0x73, 0x6F, 0x6D, 0x20, 0x76, 0x61, 0x72, 0x20, 0x6E, 0xE4, 0x72, 0x61, 0x20, 0x62, 0x65,  //  som var n�ra be
0x73, 0x6C, 0xE4, 0x6B, 0x74, 0x61, 0x74, 0x20, 0x6D, 0x65, 0x64, 0x20, 0x43, 0x2B, 0x2B, 0x2E,  // sl�ktat med C++.
0x20, 0x44, 0x65, 0x74, 0x20, 0x66, 0xF6, 0x72, 0x73, 0x74, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x6E,  //  Det f�rsta namn
0x65, 0x74, 0x20, 0x73, 0x6F, 0x6D, 0x20, 0x74, 0x65, 0x73, 0x74, 0x61, 0x64, 0x65, 0x73, 0x20,  // et som testades 
0x66, 0xF6, 0x72, 0x06, 0x70, 0x61, 0x74, 0x65, 0x6E, 0x74, 0x20, 0x76, 0x61, 0x72, 0x20, 0x4F,  // f�r.patent var O
0x61, 0x6B, 0x2C, 0x20, 0x64, 0x65, 0x74, 0x74, 0x61, 0x20, 0x6D, 0x69, 0x73, 0x73, 0x6C, 0x79,  // ak, detta missly
0x63, 0x6B, 0x61, 0x64, 0x65, 0x73, 0x20, 0x2D, 0x20, 0x6F, 0x63, 0x68, 0x20, 0x64, 0x65, 0x74,  // ckades - och det
0x20, 0x6E, 0x79, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x6E, 0x65, 0x74, 0x20, 0x62, 0x6C, 0x65, 0x76,  //  nya namnet blev
0x20, 0x4A, 0x61, 0x76, 0x61, 0x2E, 0x06, 0x53, 0x70, 0x72, 0xE5, 0x6B, 0x65, 0x74, 0x20, 0x69,  //  Java..Spr�ket i
0x6E, 0x74, 0x72, 0x6F, 0x64, 0x75, 0x63, 0x65, 0x72, 0x61, 0x64, 0x65, 0x73, 0x20, 0x69, 0x20,  // ntroducerades i 
0x31, 0x39, 0x39, 0x35, 0x2E, 0x20, 0x53, 0x70, 0x72, 0xE5, 0x6B, 0x65, 0x74, 0x73, 0x20, 0x66,  // 1995. Spr�kets f
0x72, 0x61, 0x6D, 0x67, 0xE5, 0x6E, 0x67, 0x20, 0x62, 0x65, 0x72, 0x6F, 0x72, 0x20, 0x6D, 0x79,  // ramg�ng beror my
0x63, 0x6B, 0x65, 0x74, 0x20, 0x70, 0xE5, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74, 0x20,  // cket p� att det 
0xE4, 0x72, 0x06, 0x70, 0x6C, 0x61, 0x74, 0x74, 0x66, 0x6F, 0x72, 0x6D, 0x73, 0x6F, 0x62, 0x65,  // �r.plattformsobe
0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65, 0x2E, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x20, 0x6B, 0x61, 0x6E,  // roende. Java kan
0x20, 0x62, 0x65, 0x73, 0x6B, 0x72, 0x69, 0x76, 0x61, 0x73, 0x20, 0x73, 0x6F, 0x6D, 0x20, 0x65,  //  beskrivas som e
0x6E, 0x6B, 0x65, 0x6C, 0x74, 0x2C, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69,  // nkelt, objektori
0x65, 0x6E, 0x74, 0x65, 0x72, 0x61, 0x74, 0x2C, 0x20, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62,  // enterat, distrib
0x75, 0x65, 0x72, 0x61, 0x74, 0x2C, 0x20, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x70, 0x72, 0x65, 0x74,  // uerat, interpret
0x65, 0x72, 0x61, 0x6E, 0x64, 0x65, 0x2C, 0x06, 0x72, 0x6F, 0x62, 0x75, 0x73, 0x74, 0x2C, 0x20,  // erande,.robust, 
0x73, 0xE4, 0x6B, 0x65, 0x72, 0x74, 0x2C, 0x20, 0x70, 0x6C, 0x61, 0x74, 0x74, 0x66, 0x6F, 0x72,  // s�kert, plattfor
0x6D, 0x73, 0x6F, 0x62, 0x65, 0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65, 0x2C, 0x20, 0x70, 0x6F, 0x72,  // msoberoende, por
0x74, 0x61, 0x62, 0x65, 0x6C, 0x74, 0x20, 0x6F, 0x63, 0x68, 0x20, 0x64, 0x79, 0x6E, 0x61, 0x6D,  // tabelt och dynam
0x69, 0x73, 0x6B, 0x74, 0x20, 0x73, 0x70, 0x72, 0xE5, 0x6B, 0x2E, 0x06, 0x06, 0x52, 0x75, 0x73,  // iskt spr�k...Rus
0x73, 0x69, 0x61, 0x6E, 0x3A, 0x20, 0x15, 0x32, 0x40, 0x4E, 0x4F, 0x40, 0x2C, 0x20, 0x4F, 0x40,  // sian: ......, ..
}; // phew!


// Unicode characters to form the 16-bit descriptor.
const TUint16 RefDataUnicode[] =
{
0xFEFF, 0x45, 0x6E, 0x67, 0x6C, 0x69, 0x73, 0x68, 0x3A, 0x20, 0x4A, 0x61, 0x63, 0x6B, 0x64, 0x61,  // .English: Jackda
0x77, 0x73, 0x20, 0x6C, 0x6F, 0x76, 0x65, 0x20, 0x6D, 0x79, 0x20, 0x62, 0x69, 0x67, 0x20, 0x73,  // ws love my big s
0x70, 0x68, 0x69, 0x6E, 0x78, 0x20, 0x6F, 0x66, 0x20, 0x71, 0x75, 0x61, 0x72, 0x74, 0x7A, 0x2E,  // phinx of quartz.
0x20, 0x50, 0x61, 0x63, 0x6B, 0x20, 0x6D, 0x79, 0x20, 0x62, 0x6F, 0x78, 0x20, 0x77, 0x69, 0x74,  //  Pack my box wit
0x68, 0x20, 0x66, 0x69, 0x76, 0x65, 0x20, 0x64, 0x6F, 0x7A, 0x65, 0x6E, 0x20, 0x6C, 0x69, 0x71,  // h five dozen liq
0x75, 0x6F, 0x72, 0x20, 0x6A, 0x75, 0x67, 0x73, 0x2E, 0x06, 0x06, 0x47, 0x65, 0x72, 0x6D, 0x61,  // uor jugs...Germa
0x6E, 0x3A, 0x20, 0x44, 0x61, 0xDF, 0x20, 0x43, 0x2B, 0x2B, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20,  // n: Da� C++ sich 
0x73, 0x6F, 0x20, 0x73, 0x74, 0x61, 0x72, 0x6B, 0x20, 0x76, 0x65, 0x72, 0x62, 0x72, 0x65, 0x69,  // so stark verbrei
0x74, 0x65, 0x74, 0x20, 0x68, 0x61, 0x74, 0x2C, 0x20, 0x6D, 0x61, 0x67, 0x20, 0x76, 0x6F, 0x72,  // tet hat, mag vor
0x20, 0x61, 0x6C, 0x6C, 0x65, 0x6E, 0x20, 0x44, 0x69, 0x6E, 0x67, 0x65, 0x6E, 0x20, 0x7A, 0x77,  //  allen Dingen zw
0x65, 0x69, 0x20, 0x47, 0x72, 0xFC, 0x6E, 0x64, 0x65, 0x20, 0x68, 0x61, 0x62, 0x65, 0x6E, 0x3A,  // ei Gr�nde haben:
0x06, 0x44, 0x69, 0x65, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69, 0x65, 0x6E,  // .Die objektorien
0x74, 0x69, 0x65, 0x72, 0x74, 0x65, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69,  // tierte Programmi
0x65, 0x72, 0x75, 0x6E, 0x67, 0x20, 0x69, 0x73, 0x74, 0x20, 0x73, 0x69, 0x63, 0x68, 0x65, 0x72,  // erung ist sicher
0x20, 0x65, 0x69, 0x6E, 0x65, 0x20, 0x61, 0x75, 0x73, 0x67, 0x65, 0x7A, 0x65, 0x69, 0x63, 0x68,  //  eine ausgezeich
0x6E, 0x65, 0x74, 0x65, 0x20, 0x41, 0x6C, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x74, 0x69, 0x76, 0x65,  // nete Alternative
0x06, 0x7A, 0x75, 0x20, 0x64, 0x65, 0x6E, 0x20, 0x74, 0x72, 0x61, 0x64, 0x69, 0x74, 0x69, 0x6F,  // .zu den traditio
0x6E, 0x65, 0x6C, 0x6C, 0x65, 0x6E, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69,  // nellen Programmi
0x65, 0x72, 0x74, 0x65, 0x63, 0x68, 0x6E, 0x69, 0x6B, 0x65, 0x6E, 0x2C, 0x20, 0x6D, 0xF6, 0x67,  // ertechniken, m�g
0x6C, 0x69, 0x63, 0x68, 0x65, 0x72, 0x77, 0x65, 0x69, 0x73, 0x65, 0x20, 0x66, 0xFC, 0x72, 0x20,  // licherweise f�r 
0x67, 0x72, 0x6F, 0xDF, 0x65, 0x20, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x2D, 0x50,  // gro�e Software-P
0x72, 0x6F, 0x6A, 0x65, 0x6B, 0x74, 0x65, 0x06, 0x67, 0x65, 0x67, 0x65, 0x6E, 0x77, 0xE4, 0x72,  // rojekte.gegenw�r
0x74, 0x69, 0x67, 0x20, 0x64, 0x69, 0x65, 0x20, 0x65, 0x69, 0x6E, 0x7A, 0x69, 0x67, 0x65, 0x20,  // tig die einzige 
0x4D, 0xF6, 0x67, 0x6C, 0x69, 0x63, 0x68, 0x6B, 0x65, 0x69, 0x74, 0x2C, 0x20, 0x64, 0x69, 0x65,  // M�glichkeit, die
0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x65, 0x20, 0x73, 0x6F, 0x77, 0x6F, 0x68,  //  Programme sowoh
0x6C, 0x20, 0x65, 0x66, 0x66, 0x65, 0x6B, 0x74, 0x69, 0x76, 0x20, 0x73, 0x63, 0x68, 0x72, 0x65,  // l effektiv schre
0x69, 0x62, 0x65, 0x6E, 0x20, 0x61, 0x6C, 0x73, 0x20, 0x61, 0x75, 0x63, 0x68, 0x20, 0x77, 0x61,  // iben als auch wa
0x72, 0x74, 0x65, 0x6E, 0x06, 0x7A, 0x75, 0x20, 0x6B, 0xF6, 0x6E, 0x6E, 0x65, 0x6E, 0x20, 0x28,  // rten.zu k�nnen (
0x75, 0x6E, 0x64, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69, 0x65, 0x6E, 0x74,  // und objektorient
0x69, 0x65, 0x72, 0x74, 0x65, 0x20, 0x56, 0x6F, 0x72, 0x67, 0x65, 0x68, 0x65, 0x6E, 0x73, 0x77,  // ierte Vorgehensw
0x65, 0x69, 0x73, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x6E, 0x64, 0x20, 0x61, 0x75, 0x63, 0x68, 0x20,  // eisen sind auch 
0x69, 0x6E, 0x20, 0x76, 0x69, 0x65, 0x6C, 0x65, 0x6E, 0x20, 0x61, 0x6E, 0x64, 0x65, 0x72, 0x65,  // in vielen andere
0x6E, 0x06, 0x57, 0x69, 0x73, 0x73, 0x65, 0x6E, 0x73, 0x63, 0x68, 0x61, 0x66, 0x74, 0x73, 0x62,  // n.Wissenschaftsb
0x65, 0x72, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x65, 0x72, 0x66, 0x6F, 0x6C, 0x67, 0x72,  // ereichen erfolgr
0x65, 0x69, 0x63, 0x68, 0x29, 0x2E, 0x20, 0x5A, 0x75, 0x6D, 0x20, 0x61, 0x6E, 0x64, 0x65, 0x72,  // eich). Zum ander
0x65, 0x6E, 0x20, 0x65, 0x6E, 0x74, 0x68, 0xE4, 0x6C, 0x74, 0x20, 0x43, 0x2B, 0x2B, 0x20, 0x64,  // en enth�lt C++ d
0x65, 0x6E, 0x20, 0x6B, 0x6F, 0x6D, 0x70, 0x6C, 0x65, 0x74, 0x74, 0x65, 0x6E, 0x06, 0x53, 0x70,  // en kompletten.Sp
0x72, 0x61, 0x63, 0x68, 0x75, 0x6D, 0x66, 0x61, 0x6E, 0x67, 0x20, 0x76, 0x6F, 0x6E, 0x20, 0x43,  // rachumfang von C
0x2C, 0x20, 0x73, 0x6F, 0x20, 0x64, 0x61, 0xDF, 0x20, 0x66, 0xFC, 0x72, 0x20, 0x64, 0x69, 0x65,  // , so da� f�r die
0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x69, 0x65, 0x72, 0x65, 0x72, 0x20, 0x65,  //  Programmierer e
0x69, 0x6E, 0x20, 0x22, 0x67, 0x6C, 0x65, 0x69, 0x74, 0x65, 0x6E, 0x64, 0x65, 0x72, 0x20, 0xDC,  // in "gleitender �
0x62, 0x65, 0x72, 0x67, 0x61, 0x6E, 0x67, 0x22, 0x20, 0x6D, 0xF6, 0x67, 0x6C, 0x69, 0x63, 0x68,  // bergang" m�glich
0x20, 0x69, 0x73, 0x74, 0x06, 0x75, 0x6E, 0x64, 0x20, 0x61, 0x6C, 0x6C, 0x65, 0x20, 0x43, 0x2D,  //  ist.und alle C-
0x52, 0x6F, 0x75, 0x74, 0x69, 0x6E, 0x65, 0x6E, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x43, 0x2D, 0x42,  // Routinen und C-B
0x69, 0x62, 0x6C, 0x69, 0x6F, 0x74, 0x68, 0x65, 0x6B, 0x65, 0x6E, 0x20, 0x77, 0x65, 0x69, 0x74,  // ibliotheken weit
0x65, 0x72, 0x20, 0x76, 0x65, 0x72, 0x77, 0x65, 0x6E, 0x64, 0x65, 0x74, 0x20, 0x77, 0x65, 0x72,  // er verwendet wer
0x64, 0x65, 0x6E, 0x20, 0x6B, 0xF6, 0x6E, 0x6E, 0x65, 0x6E, 0x2E, 0x06, 0x06, 0x53, 0x77, 0x65,  // den k�nnen...Swe
0x64, 0x69, 0x73, 0x68, 0x3A, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x20, 0xE4, 0x72, 0x20, 0x75, 0x74,  // dish: Java �r ut
0x76, 0x65, 0x63, 0x6B, 0x6C, 0x61, 0x64, 0x20, 0x61, 0x76, 0x20, 0x53, 0x75, 0x6E, 0x2E, 0x20,  // vecklad av Sun. 
0x46, 0x72, 0xE5, 0x6E, 0x20, 0x62, 0xF6, 0x72, 0x6A, 0x61, 0x6E, 0x20, 0x76, 0x61, 0x72, 0x20,  // Fr�n b�rjan var 
0x64, 0x65, 0x74, 0x20, 0x74, 0xE4, 0x6E, 0x6B, 0x74, 0x20, 0x61, 0x74, 0x74, 0x20, 0x73, 0x70,  // det t�nkt att sp
0x72, 0xE5, 0x6B, 0x65, 0x74, 0x20, 0x73, 0x6B, 0x75, 0x6C, 0x6C, 0x65, 0x20, 0x61, 0x6E, 0x76,  // r�ket skulle anv
0xE4, 0x6E, 0x64, 0x61, 0x73, 0x20, 0x69, 0x6E, 0x6F, 0x6D, 0x20, 0x65, 0x6C, 0x65, 0x6B, 0x74,  // �ndas inom elekt
0x72, 0x6F, 0x6E, 0x69, 0x6B, 0x20, 0x74, 0x69, 0x6C, 0x6C, 0x06, 0x74, 0x20, 0x65, 0x78, 0x20,  // ronik till.t ex 
0x62, 0x72, 0xF6, 0x64, 0x72, 0x6F, 0x73, 0x74, 0x61, 0x72, 0x20, 0x65, 0x6C, 0x6C, 0x65, 0x72,  // br�drostar eller
0x20, 0x74, 0x76, 0xE4, 0x74, 0x74, 0x6D, 0x61, 0x73, 0x6B, 0x69, 0x6E, 0x65, 0x72, 0x2E, 0x20,  //  tv�ttmaskiner. 
0x55, 0x74, 0x76, 0x65, 0x63, 0x6B, 0x6C, 0x61, 0x72, 0x6E, 0x61, 0x20, 0x69, 0x6E, 0x73, 0xE5,  // Utvecklarna ins�
0x67, 0x20, 0x73, 0x6E, 0x61, 0x62, 0x62, 0x74, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74,  // g snabbt att det
0x20, 0x6B, 0x72, 0xE4, 0x76, 0x64, 0x65, 0x73, 0x20, 0x65, 0x74, 0x74, 0x20, 0x70, 0x6C, 0x61,  //  kr�vdes ett pla
0x74, 0x74, 0x66, 0x6F, 0x72, 0x6D, 0x73, 0x6F, 0x62, 0x65, 0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65,  // ttformsoberoende
0x06, 0x73, 0x70, 0x72, 0xE5, 0x6B, 0x20, 0x66, 0xF6, 0x72, 0x20, 0x61, 0x74, 0x74, 0x20, 0x6B,  // .spr�k f�r att k
0x75, 0x6E, 0x6E, 0x61, 0x20, 0x74, 0x69, 0x6C, 0x6C, 0x67, 0x6F, 0x64, 0x6F, 0x73, 0x65, 0x20,  // unna tillgodose 
0x64, 0x65, 0x20, 0x6F, 0x6C, 0x69, 0x6B, 0x61, 0x20, 0x62, 0x65, 0x68, 0x6F, 0x76, 0x65, 0x6E,  // de olika behoven
0x20, 0x70, 0xE5, 0x20, 0x65, 0x6C, 0x65, 0x6B, 0x74, 0x72, 0x6F, 0x6E, 0x69, 0x6B, 0x6D, 0x61,  //  p� elektronikma
0x72, 0x6B, 0x6E, 0x61, 0x64, 0x65, 0x6E, 0x2E, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x73, 0x20, 0x73,  // rknaden. Javas s
0x6B, 0x61, 0x70, 0x61, 0x72, 0x65, 0x20, 0x4A, 0x61, 0x6D, 0x65, 0x73, 0x20, 0x47, 0x6F, 0x73,  // kapare James Gos
0x6C, 0x69, 0x6E, 0x67, 0x06, 0x62, 0xF6, 0x72, 0x6A, 0x61, 0x64, 0x65, 0x20, 0x61, 0x72, 0x62,  // ling.b�rjade arb
0x65, 0x74, 0x61, 0x20, 0x75, 0x74, 0x69, 0x66, 0x72, 0xE5, 0x6E, 0x20, 0x43, 0x2B, 0x2B, 0x20,  // eta utifr�n C++ 
0x6B, 0x6F, 0x6D, 0x70, 0x69, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x72, 0x6E, 0x2E, 0x20, 0x45, 0x66,  // kompilatiorn. Ef
0x74, 0x65, 0x72, 0x20, 0x65, 0x6E, 0x20, 0x74, 0x69, 0x64, 0x20, 0x69, 0x6E, 0x73, 0xE5, 0x67,  // ter en tid ins�g
0x20, 0x68, 0x61, 0x6E, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74, 0x06, 0x62, 0xE4, 0x73,  //  han att det.b�s
0x74, 0x61, 0x20, 0x76, 0x6F, 0x72, 0x65, 0x20, 0x61, 0x74, 0x74, 0x20, 0x73, 0x6B, 0x61, 0x70,  // ta vore att skap
0x61, 0x20, 0x65, 0x74, 0x74, 0x20, 0x6E, 0x79, 0x74, 0x74, 0x20, 0x73, 0x70, 0x72, 0xE5, 0x6B,  // a ett nytt spr�k
0x20, 0x73, 0x6F, 0x6D, 0x20, 0x76, 0x61, 0x72, 0x20, 0x6E, 0xE4, 0x72, 0x61, 0x20, 0x62, 0x65,  //  som var n�ra be
0x73, 0x6C, 0xE4, 0x6B, 0x74, 0x61, 0x74, 0x20, 0x6D, 0x65, 0x64, 0x20, 0x43, 0x2B, 0x2B, 0x2E,  // sl�ktat med C++.
0x20, 0x44, 0x65, 0x74, 0x20, 0x66, 0xF6, 0x72, 0x73, 0x74, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x6E,  //  Det f�rsta namn
0x65, 0x74, 0x20, 0x73, 0x6F, 0x6D, 0x20, 0x74, 0x65, 0x73, 0x74, 0x61, 0x64, 0x65, 0x73, 0x20,  // et som testades 
0x66, 0xF6, 0x72, 0x06, 0x70, 0x61, 0x74, 0x65, 0x6E, 0x74, 0x20, 0x76, 0x61, 0x72, 0x20, 0x4F,  // f�r.patent var O
0x61, 0x6B, 0x2C, 0x20, 0x64, 0x65, 0x74, 0x74, 0x61, 0x20, 0x6D, 0x69, 0x73, 0x73, 0x6C, 0x79,  // ak, detta missly
0x63, 0x6B, 0x61, 0x64, 0x65, 0x73, 0x20, 0x2D, 0x20, 0x6F, 0x63, 0x68, 0x20, 0x64, 0x65, 0x74,  // ckades - och det
0x20, 0x6E, 0x79, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x6E, 0x65, 0x74, 0x20, 0x62, 0x6C, 0x65, 0x76,  //  nya namnet blev
0x20, 0x4A, 0x61, 0x76, 0x61, 0x2E, 0x06, 0x53, 0x70, 0x72, 0xE5, 0x6B, 0x65, 0x74, 0x20, 0x69,  //  Java..Spr�ket i
0x6E, 0x74, 0x72, 0x6F, 0x64, 0x75, 0x63, 0x65, 0x72, 0x61, 0x64, 0x65, 0x73, 0x20, 0x69, 0x20,  // ntroducerades i 
0x31, 0x39, 0x39, 0x35, 0x2E, 0x20, 0x53, 0x70, 0x72, 0xE5, 0x6B, 0x65, 0x74, 0x73, 0x20, 0x66,  // 1995. Spr�kets f
0x72, 0x61, 0x6D, 0x67, 0xE5, 0x6E, 0x67, 0x20, 0x62, 0x65, 0x72, 0x6F, 0x72, 0x20, 0x6D, 0x79,  // ramg�ng beror my
0x63, 0x6B, 0x65, 0x74, 0x20, 0x70, 0xE5, 0x20, 0x61, 0x74, 0x74, 0x20, 0x64, 0x65, 0x74, 0x20,  // cket p� att det 
0xE4, 0x72, 0x06, 0x70, 0x6C, 0x61, 0x74, 0x74, 0x66, 0x6F, 0x72, 0x6D, 0x73, 0x6F, 0x62, 0x65,  // �r.plattformsobe
0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65, 0x2E, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x20, 0x6B, 0x61, 0x6E,  // roende. Java kan
0x20, 0x62, 0x65, 0x73, 0x6B, 0x72, 0x69, 0x76, 0x61, 0x73, 0x20, 0x73, 0x6F, 0x6D, 0x20, 0x65,  //  beskrivas som e
0x6E, 0x6B, 0x65, 0x6C, 0x74, 0x2C, 0x20, 0x6F, 0x62, 0x6A, 0x65, 0x6B, 0x74, 0x6F, 0x72, 0x69,  // nkelt, objektori
0x65, 0x6E, 0x74, 0x65, 0x72, 0x61, 0x74, 0x2C, 0x20, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62,  // enterat, distrib
0x75, 0x65, 0x72, 0x61, 0x74, 0x2C, 0x20, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x70, 0x72, 0x65, 0x74,  // uerat, interpret
0x65, 0x72, 0x61, 0x6E, 0x64, 0x65, 0x2C, 0x06, 0x72, 0x6F, 0x62, 0x75, 0x73, 0x74, 0x2C, 0x20,  // erande,.robust, 
0x73, 0xE4, 0x6B, 0x65, 0x72, 0x74, 0x2C, 0x20, 0x70, 0x6C, 0x61, 0x74, 0x74, 0x66, 0x6F, 0x72,  // s�kert, plattfor
0x6D, 0x73, 0x6F, 0x62, 0x65, 0x72, 0x6F, 0x65, 0x6E, 0x64, 0x65, 0x2C, 0x20, 0x70, 0x6F, 0x72,  // msoberoende, por
0x74, 0x61, 0x62, 0x65, 0x6C, 0x74, 0x20, 0x6F, 0x63, 0x68, 0x20, 0x64, 0x79, 0x6E, 0x61, 0x6D,  // tabelt och dynam
0x69, 0x73, 0x6B, 0x74, 0x20, 0x73, 0x70, 0x72, 0xE5, 0x6B, 0x2E, 0x06, 0x06, 0x52, 0x75, 0x73,  // iskt spr�k...Rus
0x73, 0x69, 0x61, 0x6E, 0x3A, 0x20, 0x415, 0x432, 0x440, 0x43E, 0x43F, 0x430, 0x2C, 0x20, 0x41F, 0x440,  // sian: ......, ..
0x43E, 0x433, 0x440, 0x430, 0x43C, 0x43C, 0x43D, 0x43E, 0x435, 0x20, 0x43E, 0x431, 0x435, 0x441, 0x43F, 0x435, 
0x447, 0x435, 0x43D, 0x438, 0x435, 0x20, 0x2B, 0x20, 0x418, 0x43D, 0x442, 0x435, 0x440, 0x43D, 0x435, 0x442,  // ..... + ........
0x3A, 0x06, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x20, 0x432, 0x20, 0x433, 0x43B, 0x43E, 0x431,  // :.Unicode . ....
0x430, 0x43B, 0x44C, 0x43D, 0x44B, 0x445, 0x20, 0x43C, 0x430, 0x441, 0x448, 0x442, 0x430, 0x431, 0x430, 0x445, 
0x20, 0x20, 0x06, 0x417, 0x430, 0x440, 0x435, 0x433, 0x438, 0x441, 0x442, 0x440, 0x438, 0x440, 0x443, 0x439, 
0x442, 0x435, 0x441, 0x44C, 0x20, 0x441, 0x435, 0x439, 0x447, 0x430, 0x441, 0x20, 0x43D, 0x430, 0x20, 0x414, 
0x435, 0x441, 0x44F, 0x442, 0x443, 0x44E, 0x20, 0x41C, 0x435, 0x436, 0x434, 0x443, 0x43D, 0x430, 0x440, 0x43E, 
0x434, 0x43D, 0x443, 0x44E, 0x20, 0x41A, 0x43E, 0x43D, 0x444, 0x435, 0x440, 0x435, 0x43D, 0x446, 0x438, 0x44E, 
0x20, 0x43F, 0x43E, 0x20, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x2C, 0x20, 0x43A, 0x43E, 0x442,  //  .. Unicode, ...
0x43E, 0x440, 0x430, 0x44F, 0x06, 0x441, 0x43E, 0x441, 0x442, 0x43E, 0x438, 0x442, 0x441, 0x44F, 0x20, 0x31,  // .............. 1
0x30, 0x2D, 0x31, 0x32, 0x20, 0x43C, 0x430, 0x440, 0x442, 0x430, 0x20, 0x31, 0x39, 0x39, 0x37, 0x20,  // 0-12 ..... 1997 
0x433, 0x43E, 0x434, 0x430, 0x20, 0x432, 0x20, 0x41C, 0x430, 0x439, 0x43D, 0x446, 0x435, 0x20, 0x432, 0x20, 
0x413, 0x435, 0x440, 0x43C, 0x430, 0x43D, 0x438, 0x438, 0x2E, 0x20, 0x41A, 0x43E, 0x43D, 0x444, 0x435, 0x440,  // ......... ......
0x435, 0x43D, 0x446, 0x438, 0x44F, 0x20, 0x441, 0x43E, 0x431, 0x435, 0x440, 0x435, 0x442, 0x20, 0x448, 0x438, 
0x440, 0x43E, 0x43A, 0x438, 0x439, 0x20, 0x43A, 0x440, 0x443, 0x433, 0x06, 0x44D, 0x43A, 0x441, 0x43F, 0x435, 
0x440, 0x442, 0x43E, 0x432, 0x20, 0x43F, 0x43E, 0x20, 0x432, 0x43E, 0x43F, 0x440, 0x43E, 0x441, 0x430, 0x43C, 
0x20, 0x433, 0x43B, 0x43E, 0x431, 0x430, 0x43B, 0x44C, 0x43D, 0x43E, 0x433, 0x43E, 0x20, 0x418, 0x43D, 0x442, 
0x435, 0x440, 0x43D, 0x435, 0x442, 0x430, 0x20, 0x438, 0x20, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65,  // ...... . Unicode
0x2C, 0x20, 0x43B, 0x43E, 0x43A, 0x430, 0x43B, 0x438, 0x437, 0x430, 0x446, 0x438, 0x438, 0x20, 0x438, 0x20,  // , ........... . 
0x438, 0x43D, 0x442, 0x435, 0x440, 0x43D, 0x430, 0x446, 0x438, 0x43E, 0x43D, 0x430, 0x43B, 0x438, 0x437, 0x430, 
0x446, 0x438, 0x438, 0x2C, 0x06, 0x432, 0x43E, 0x43F, 0x43B, 0x43E, 0x449, 0x435, 0x43D, 0x438, 0x44E, 0x20,  // ...,........... 
0x438, 0x20, 0x43F, 0x440, 0x438, 0x43C, 0x435, 0x43D, 0x435, 0x43D, 0x438, 0x44E, 0x20, 0x55, 0x6E, 0x69,  // . .......... Uni
0x63, 0x6F, 0x64, 0x65, 0x20, 0x432, 0x20, 0x440, 0x430, 0x437, 0x43B, 0x438, 0x447, 0x43D, 0x44B, 0x445,  // code . .........
0x20, 0x43E, 0x43F, 0x435, 0x440, 0x430, 0x446, 0x438, 0x43E, 0x43D, 0x43D, 0x44B, 0x445, 0x20, 0x441, 0x438, 
0x441, 0x442, 0x435, 0x43C, 0x430, 0x445, 0x20, 0x438, 0x20, 0x43F, 0x440, 0x43E, 0x433, 0x440, 0x430, 0x43C, 
0x43C, 0x43D, 0x44B, 0x445, 0x06, 0x43F, 0x440, 0x438, 0x43B, 0x43E, 0x436, 0x435, 0x43D, 0x438, 0x44F, 0x445, 
0x2C, 0x20, 0x448, 0x440, 0x438, 0x444, 0x442, 0x430, 0x445, 0x2C, 0x20, 0x432, 0x435, 0x440, 0x441, 0x442,  // , ......., .....
0x43A, 0x435, 0x20, 0x438, 0x20, 0x43C, 0x43D, 0x43E, 0x433, 0x43E, 0x44F, 0x437, 0x44B, 0x447, 0x43D, 0x44B, 
0x445, 0x20, 0x43A, 0x43E, 0x43C, 0x43F, 0x44C, 0x44E, 0x442, 0x435, 0x440, 0x43D, 0x44B, 0x445, 0x20, 0x441, 
0x438, 0x441, 0x442, 0x435, 0x43C, 0x430, 0x445, 0x2E, 0x20, 0x20, 0x06, 0x41A, 0x43E, 0x433, 0x434, 0x430,  // ........  ......
0x20, 0x43C, 0x438, 0x440, 0x20, 0x436, 0x435, 0x43B, 0x430, 0x435, 0x442, 0x20, 0x43E, 0x431, 0x449, 0x430, 
0x442, 0x44C, 0x441, 0x44F, 0x2C, 0x20, 0x43E, 0x43D, 0x20, 0x43E, 0x431, 0x449, 0x430, 0x435, 0x442, 0x441,  // ...., .. .......
0x44F, 0x20, 0x43D, 0x430, 0x20, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x20, 0x20, 0x06, 0x06,  // . .. Unicode  ..
0x41, 0x72, 0x61, 0x62, 0x69, 0x63, 0x3A, 0x20, 0x623, 0x648, 0x631, 0x648, 0x628, 0x627, 0x2C, 0x20,  // Arabic: ......, 
0x628, 0x631, 0x645, 0x62C, 0x64A, 0x627, 0x62A, 0x20, 0x627, 0x644, 0x62D, 0x627, 0x633, 0x648, 0x628, 0x20, 
0x2B, 0x20, 0x627, 0x646, 0x62A, 0x631, 0x646, 0x64A, 0x62A, 0x20, 0x3A, 0x06, 0x62A, 0x635, 0x628, 0x62D,  // + ....... :.....
0x20, 0x639, 0x627, 0x644, 0x645, 0x64A, 0x627, 0x20, 0x645, 0x639, 0x20, 0x64A, 0x648, 0x646, 0x64A, 0x643, 
0x648, 0x62F, 0x20, 0x20, 0x06, 0x62A, 0x633, 0x62C, 0x651, 0x644, 0x20, 0x627, 0x644, 0x622, 0x646, 0x20, 
0x644, 0x62D, 0x636, 0x648, 0x631, 0x20, 0x627, 0x644, 0x645, 0x624, 0x62A, 0x645, 0x631, 0x20, 0x627, 0x644, 
0x62F, 0x648, 0x644, 0x64A, 0x20, 0x627, 0x644, 0x639, 0x627, 0x634, 0x631, 0x20, 0x644, 0x64A, 0x648, 0x646, 
0x64A, 0x643, 0x648, 0x62F, 0x2C, 0x20, 0x627, 0x644, 0x630, 0x64A, 0x20, 0x633, 0x64A, 0x639, 0x642, 0x62F,  // ...., .... .....
0x20, 0x641, 0x64A, 0x20, 0x31, 0x30, 0x2D, 0x31, 0x32, 0x06, 0x622, 0x630, 0x627, 0x631, 0x20, 0x31,  //  .. 10-12..... 1
0x39, 0x39, 0x37, 0x20, 0x628, 0x645, 0x62F, 0x64A, 0x646, 0x629, 0x20, 0x645, 0x627, 0x64A, 0x646, 0x62A,  // 997 ...... .....
0x633, 0x2C, 0x20, 0x623, 0x644, 0x645, 0x627, 0x646, 0x64A, 0x627, 0x2E, 0x20, 0x648, 0x633, 0x64A, 0x62C,  // ., ........ ....
0x645, 0x639, 0x20, 0x627, 0x644, 0x645, 0x624, 0x62A, 0x645, 0x631, 0x20, 0x628, 0x64A, 0x646, 0x20, 0x62E, 
0x628, 0x631, 0x627, 0x621, 0x20, 0x645, 0x646, 0x20, 0x643, 0x627, 0x641, 0x629, 0x20, 0x642, 0x637, 0x627, 
0x639, 0x627, 0x62A, 0x06, 0x627, 0x644, 0x635, 0x646, 0x627, 0x639, 0x629, 0x20, 0x639, 0x644, 0x649, 0x20, 
0x627, 0x644, 0x634, 0x628, 0x643, 0x629, 0x20, 0x627, 0x644, 0x639, 0x627, 0x644, 0x645, 0x64A, 0x629, 0x20, 
0x627, 0x646, 0x62A, 0x631, 0x646, 0x64A, 0x62A, 0x20, 0x648, 0x64A, 0x648, 0x646, 0x64A, 0x643, 0x648, 0x62F, 
0x2C, 0x20, 0x62D, 0x64A, 0x62B, 0x20, 0x633, 0x62A, 0x62A, 0x645, 0x2C, 0x20, 0x639, 0x644, 0x649, 0x20,  // , ... ...., ... 
0x627, 0x644, 0x635, 0x639, 0x64A, 0x62F, 0x64A, 0x646, 0x06, 0x627, 0x644, 0x62F, 0x648, 0x644, 0x64A, 0x20, 
0x648, 0x627, 0x644, 0x645, 0x62D, 0x644, 0x64A, 0x20, 0x639, 0x644, 0x649, 0x20, 0x62D, 0x62F, 0x20, 0x633, 
0x648, 0x627, 0x621, 0x20, 0x645, 0x646, 0x627, 0x642, 0x634, 0x629, 0x20, 0x633, 0x628, 0x644, 0x20, 0x627, 
0x633, 0x62A, 0x62E, 0x62F, 0x627, 0x645, 0x20, 0x64A, 0x648, 0x646, 0x643, 0x648, 0x62F, 0x20, 0x641, 0x64A, 
0x20, 0x627, 0x644, 0x646, 0x638, 0x645, 0x20, 0x627, 0x644, 0x642, 0x627, 0x626, 0x645, 0x629, 0x06, 0x648, 
0x641, 0x64A, 0x645, 0x627, 0x20, 0x64A, 0x62E, 0x635, 0x20, 0x627, 0x644, 0x62A, 0x637, 0x628, 0x64A, 0x642, 
0x627, 0x62A, 0x20, 0x627, 0x644, 0x62D, 0x627, 0x633, 0x648, 0x628, 0x64A, 0x629, 0x2C, 0x20, 0x627, 0x644,  // .. ........., ..
0x62E, 0x637, 0x648, 0x637, 0x2C, 0x20, 0x62A, 0x635, 0x645, 0x64A, 0x645, 0x20, 0x627, 0x644, 0x646, 0x635,  // ...., ..... ....
0x648, 0x635, 0x20, 0x648, 0x627, 0x644, 0x62D, 0x648, 0x633, 0x628, 0x629, 0x20, 0x645, 0x62A, 0x639, 0x62F, 
0x62F, 0x629, 0x20, 0x627, 0x644, 0x644, 0x63A, 0x627, 0x62A, 0x2E, 0x20, 0x20, 0x06, 0x639, 0x646, 0x62F,  // .. .......  ....
0x645, 0x627, 0x20, 0x64A, 0x631, 0x64A, 0x62F, 0x20, 0x627, 0x644, 0x639, 0x627, 0x644, 0x645, 0x20, 0x623, 
0x646, 0x20, 0x64A, 0x62A, 0x643, 0x644, 0x651, 0x645, 0x2C, 0x20, 0x641, 0x647, 0x648, 0x20, 0x64A, 0x62A,  // . ......, ... ..
0x62D, 0x62F, 0x651, 0x62B, 0x20, 0x628, 0x644, 0x63A, 0x629, 0x20, 0x64A, 0x648, 0x646, 0x64A, 0x643, 0x648, 
0x62F, 0x2E, 0x20, 0x20, 0x06, 0x06, 0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x28, 0x73,  // ..  ..Chinese (s
0x69, 0x6D, 0x70, 0x6C, 0x69, 0x66, 0x69, 0x65, 0x64, 0x29, 0x3B, 0x20, 0x6B27, 0x6D32, 0xFF0C, 0x8F6F,  // implified); ....
0x4EF6, 0xFF0B, 0x4E92, 0x8054, 0x7F51, 0x06, 0x7528, 0x7EDF, 0x4E00, 0x7801, 0x20, 0x28, 0x55, 0x6E, 0x69, 0x63,  // .......... (Unic
0x6F, 0x64, 0x65, 0x29, 0x20, 0x8D70, 0x904D, 0x4E16, 0x754C, 0x20, 0x20, 0x06, 0x5C06, 0x4E8E, 0x31, 0x39,  // ode) ....  ...19
0x39, 0x37, 0x5E74, 0x20, 0x33, 0x20, 0x6708, 0x31, 0x30, 0x65E5, 0xFF0D, 0x31, 0x32, 0x65E5, 0x5728, 0x5FB7,  // 97. 3 .10..12...
0x56FD, 0x20, 0x4D, 0x61, 0x69, 0x6E, 0x7A, 0x20, 0x5E02, 0x4E3E, 0x884C, 0x7684, 0x7B2C, 0x5341, 0x5C4A, 0x7EDF,  // . Mainz ........
0x4E00, 0x7801, 0x56FD, 0x9645, 0x7814, 0x8BA8, 0x4F1A, 0x73B0, 0x5728, 0x5F00, 0x59CB, 0x6CE8, 0x518C, 0x3002, 0x06, 0x672C, 
0x6B21, 0x4F1A, 0x8BAE, 0x5C06, 0x6C47, 0x96C6, 0x5404, 0x65B9, 0x9762, 0x7684, 0x4E13, 0x5BB6, 0x3002, 0x6D89, 0x53CA, 0x7684, 
0x9886, 0x57DF, 0x5305, 0x62EC, 0xFF1A, 0x56FD, 0x9645, 0x4E92, 0x8054, 0x7F51, 0x548C, 0x7EDF, 0x4E00, 0x7801, 0xFF0C, 0x56FD, 
0x9645, 0x5316, 0x548C, 0x672C, 0x5730, 0x5316, 0xFF0C, 0x06, 0x7EDF, 0x4E00, 0x7801, 0x5728, 0x64CD, 0x4F5C, 0x7CFB, 0x7EDF, 
0x548C, 0x5E94, 0x7528, 0x8F6F, 0x4EF6, 0x4E2D, 0x7684, 0x5B9E, 0x73B0, 0xFF0C, 0x5B57, 0x578B, 0xFF0C, 0x6587, 0x672C, 0x683C, 
0x5F0F, 0x4EE5, 0x53CA, 0x591A, 0x6587, 0x79CD, 0x8BA1, 0x7B97, 0x7B49, 0x3002, 0x20, 0x20, 0x06, 0x5F53, 0x4E16, 0x754C, 
0x9700, 0x8981, 0x6C9F, 0x901A, 0x65F6, 0xFF0C, 0x8BF7, 0x7528, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0xFF01,  // ........Unicode.
0x20, 0x20, 0x06, 0x06, 0x43, 0x68, 0x69, 0x6E, 0x65, 0x73, 0x65, 0x20, 0x28, 0x74, 0x72, 0x61,  //   ..Chinese (tra
0x64, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x29, 0x20, 0x6B50, 0x6D32, 0xFF0C, 0x8EDF, 0x9AD4, 0x53CA,  // ditional) ......
0x7DB2, 0x969B, 0x7DB2, 0x8DEF, 0xFF1A, 0x06, 0x8B93, 0x7D71, 0x4E00, 0x78BC, 0xFF08, 0x55, 0x6E, 0x69, 0x63, 0x6F,  // ...........Unico
0x64, 0x65, 0xFF09, 0x9818, 0x4F60, 0x9032, 0x5165, 0x5168, 0x4E16, 0x754C, 0x20, 0x20, 0x06, 0x4F60, 0x73FE, 0x5728,  // de........  ....
0x5C31, 0x61C9, 0x5831, 0x540D, 0x5C07, 0x5728, 0xFF11, 0xFF19, 0xFF19, 0xFF17, 0x5E74, 0xFF13, 0x6708, 0xFF11, 0xFF10, 0x81F3, 
0xFF11, 0xFF12, 0x65E5, 0x65BC, 0x5FB7, 0x570B, 0x7F8E, 0x59FF, 0x57CE, 0xFF08, 0x4D, 0x61, 0x69, 0x6E, 0x7A, 0xFF09,  // ..........Mainz.
0x06, 0x53EC, 0x958B, 0x7684, 0x7B2C, 0x5341, 0x5C46, 0x570B, 0x969B, 0x7D71, 0x4E00, 0x78BC, 0x7814, 0x8A0E, 0x6703, 0x3002, 
0x672C, 0x6B21, 0x7814, 0x8A0E, 0x6703, 0x5C07, 0x9080, 0x8ACB, 0x591A, 0x4F4D, 0x696D, 0x754C, 0x5C08, 0x5BB6, 0x7814, 0x8A0E, 
0x95DC, 0x65BC, 0x5168, 0x7403, 0x7DB2, 0x969B, 0x7DB2, 0x8DEF, 0x53CA, 0x7D71, 0x4E00, 0x78BC, 0x767C, 0x5C55, 0x3001, 0x06, 
0x570B, 0x969B, 0x5316, 0x53CA, 0x672C, 0x571F, 0x5316, 0x3001, 0x652F, 0x63F4, 0x7D71, 0x4E00, 0x78BC, 0x7684, 0x4F5C, 0x696D, 
0x7CFB, 0x7D71, 0x53CA, 0x61C9, 0x7528, 0x7A0B, 0x5F0F, 0x3001, 0x5B57, 0x578B, 0x3001, 0x6587, 0x5B57, 0x6392, 0x7248, 0x3001, 
0x96FB, 0x8166, 0x591A, 0x570B, 0x8A9E, 0x6587, 0x5316, 0x7B49, 0x591A, 0x9805, 0x8AB2, 0x984C, 0x3002, 0x20, 0x20, 0x06, 
0x7576, 0x4E16, 0x754C, 0x9700, 0x8981, 0x6E9D, 0x901A, 0x6642, 0xFF0C, 0x8ACB, 0x7528, 0x7D71, 0x4E00, 0x78BC, 0xFF08, 0x55,  // ...............U
0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0xFF09, 0x20, 0x20, 0x06, 0x06, 0x47, 0x65, 0x6F, 0x72, 0x67,  // nicode.  ..Georg
0x69, 0x61, 0x6E, 0x3A, 0x20, 0x10D4, 0x10D5, 0x10E0, 0x10DD, 0x10DE, 0x10D0, 0x2C, 0x20, 0x10DE, 0x10E0, 0x10DD,  // ian: ......, ...
0x10D2, 0x10E0, 0x10D0, 0x10DB, 0x10E3, 0x10DA, 0x10D8, 0x20, 0x10E3, 0x10D6, 0x10E0, 0x10E3, 0x10DC, 0x10D5, 0x10D4, 0x10DA, 
0x10E7, 0x10DD, 0x10E4, 0x10D0, 0x20, 0x2B, 0x20, 0x10D8, 0x10DC, 0x10E2, 0x10D4, 0x10E0, 0x10DC, 0x10D4, 0x10E2, 0x10D8,  // .... + .........
0x3A, 0x06, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x2D, 0x10D8, 0x20, 0x10DB, 0x10D7, 0x10D4, 0x10DA,  // :.Unicode-. ....
0x10E1, 0x20, 0x10DB, 0x10E1, 0x10DD, 0x10E4, 0x10DA, 0x10D8, 0x10DD, 0x10E8, 0x10D8, 0x20, 0x20, 0x06, 0x10D2, 0x10D7, 
0x10EE, 0x10DD, 0x10D5, 0x10D7, 0x20, 0x10D0, 0x10EE, 0x10DA, 0x10D0, 0x10D5, 0x10D4, 0x20, 0x10D2, 0x10D0, 0x10D8, 0x10D0, 
0x10E0, 0x10DD, 0x10D7, 0x20, 0x10E0, 0x10D4, 0x10D2, 0x10D8, 0x10E1, 0x10E2, 0x10E0, 0x10D0, 0x10EA, 0x10D8, 0x10D0, 0x20, 
0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x2D, 0x10D8, 0x10E1, 0x20, 0x10DB, 0x10D4, 0x10D0, 0x10D7, 0x10D4,  // Unicode-.. .....
0x06, 0x10E1, 0x10D0, 0x10D4, 0x10E0, 0x10D7, 0x10D0, 0x10E8, 0x10DD, 0x10E0, 0x10D8, 0x10E1, 0x10DD, 0x20, 0x10D9, 0x10DD, 
0x10DC, 0x10E4, 0x10D4, 0x10E0, 0x10D4, 0x10DC, 0x10EA, 0x10D8, 0x10D0, 0x10D6, 0x10D4, 0x20, 0x10D3, 0x10D0, 0x10E1, 0x10D0, 
0x10E1, 0x10EC, 0x10E0, 0x10D4, 0x10D1, 0x10D0, 0x10D3, 0x2C, 0x20, 0x10E0, 0x10DD, 0x10DB, 0x10D4, 0x10DA, 0x10D8, 0x10EA,  // ......., .......
0x06, 0x10D2, 0x10D0, 0x10D8, 0x10DB, 0x10D0, 0x10E0, 0x10D7, 0x10D4, 0x10D1, 0x10D0, 0x20, 0x31, 0x30, 0x2D, 0x31,  // ........... 10-1
0x32, 0x20, 0x10DB, 0x10D0, 0x10E0, 0x10E2, 0x10E1, 0x2C, 0x20, 0x10E5, 0x2E, 0x20, 0x10DB, 0x10D0, 0x10D8, 0x10DC,  // 2 ....., .. ....
0x10EA, 0x10E8, 0x10D8, 0x2C, 0x20, 0x10D2, 0x10D4, 0x10E0, 0x10DB, 0x10D0, 0x10DC, 0x10D8, 0x10D0, 0x10E8, 0x10D8, 0x2E,  // ..., ...........
0x06, 0x10D9, 0x10DD, 0x10DC, 0x10E4, 0x10D4, 0x10E0, 0x10D4, 0x10DC, 0x10EA, 0x10D8, 0x10D0, 0x20, 0x10E8, 0x10D4, 0x10F0, 
0x10D9, 0x10E0, 0x10D4, 0x10D1, 0x10E1, 0x20, 0x10D4, 0x10E0, 0x10D7, 0x10D0, 0x10D3, 0x20, 0x10DB, 0x10E1, 0x10DD, 0x10E4, 
0x10DA, 0x10D8, 0x10DD, 0x10E1, 0x20, 0x10D4, 0x10E5, 0x10E1, 0x10DE, 0x10D4, 0x10E0, 0x10E2, 0x10D4, 0x10D1, 0x10E1, 0x20, 
0x10D8, 0x10E1, 0x10D4, 0x10D7, 0x06, 0x10D3, 0x10D0, 0x10E0, 0x10D2, 0x10D4, 0x10D1, 0x10E8, 0x10D8, 0x20, 0x10E0, 0x10DD, 
0x10D2, 0x10DD, 0x10E0, 0x10D8, 0x10EA, 0x10D0, 0x10D0, 0x20, 0x10D8, 0x10DC, 0x10E2, 0x10D4, 0x10E0, 0x10DC, 0x10D4, 0x10E2, 
0x10D8, 0x20, 0x10D3, 0x10D0, 0x20, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x2D, 0x10D8, 0x2C, 0x06,  // . .. Unicode-.,.
0x10D8, 0x10DC, 0x10E2, 0x10D4, 0x10E0, 0x10DC, 0x10D0, 0x10EA, 0x10D8, 0x10DD, 0x10DC, 0x10D0, 0x10DA, 0x10D8, 0x10D6, 0x10D0, 
0x10EA, 0x10D8, 0x10D0, 0x20, 0x10D3, 0x10D0, 0x20, 0x10DA, 0x10DD, 0x10D9, 0x10D0, 0x10DA, 0x10D8, 0x10D6, 0x10D0, 0x10EA, 
0x10D8, 0x10D0, 0x2C, 0x20, 0x55, 0x6E, 0x69, 0x63, 0x6F, 0x64, 0x65, 0x2D, 0x10D8, 0x10E1, 0x06, 0x10D2,  // .., Unicode-....
0x10D0, 0x10DB, 0x10DD, 0x10E7, 0x10D4, 0x10DC, 0x10D4, 0x10D1, 0x10D0, 0x20, 0x10DD, 0x10DE, 0x10D4, 0x10E0, 0x10D0, 0x10EA, 
0x10D8, 0x10E3, 0x10DA, 0x20, 0x10E1, 0x10D8, 0x10E1, 0x10E2, 0x10D4, 0x10DB, 0x10D4, 0x10D1, 0x10E1, 0x10D0, 0x2C, 0x20,  // ... .........., 
0x10D3, 0x10D0, 0x20, 0x10D2, 0x10D0, 0x10DB, 0x10DD, 0x10E7, 0x10D4, 0x10DC, 0x10D4, 0x10D1, 0x10D8, 0x10D7, 0x06, 0x10DE, 
0x10E0, 0x10DD, 0x10D2, 0x10E0, 0x10D0, 0x10DB, 0x10D4, 0x10D1, 0x10E8, 0x10D8, 0x2C, 0x20, 0x10E8, 0x10E0, 0x10D8, 0x10E4,  // .........., ....
0x10E2, 0x10D4, 0x10D1, 0x10E8, 0x10D8, 0x2C, 0x20, 0x10E2, 0x10D4, 0x10E5, 0x10E1, 0x10E2, 0x10D4, 0x10D1, 0x10D8, 0x10E1,  // ....., .........
0x20, 0x10D3, 0x10D0, 0x10DB, 0x10E3, 0x10E8, 0x10D0, 0x10D5, 0x10D4, 0x10D1, 0x10D0, 0x10E1, 0x10D0, 0x20, 0x10D3, 0x10D0, 
0x06, 0x10DB, 0x10E0, 0x10D0, 0x10D5, 0x10D0, 0x10DA, 0x10D4, 0x10DC, 0x10DD, 0x10D5, 0x10D0, 0x10DC, 0x20, 0x10D9, 0x10DD, 
0x10DB, 0x10DE, 0x10D8, 0x10E3, 0x10E2, 0x10D4, 0x10E0, 0x10E3, 0x10DA, 0x20, 0x10E1, 0x10D8, 0x10E1, 0x10E2, 0x10D4, 0x10DB, 
0x10D4, 0x10D1, 0x10E8, 0x10D8, 0x2E, 0x20, 0x20, 0x06, 0x10E0, 0x10DD, 0x10D3, 0x10D4, 0x10E1, 0x10D0, 0x10EA, 0x20,  // .....  ........ 
0x10DB, 0x10E1, 0x10DD, 0x10E4, 0x10DA, 0x10D8, 0x10DD, 0x10E1, 0x20, 0x10E3, 0x10E0, 0x10D7, 0x10D8, 0x10E0, 0x10D7, 0x10DD, 
0x10D1, 0x10D0, 0x20, 0x10E1, 0x10E3, 0x10E0, 0x10E1, 0x2C, 0x20, 0x10D8, 0x10D2, 0x10D8, 0x20, 0x55, 0x6E, 0x69,  // .. ...., ... Uni
0x63, 0x6F, 0x64, 0x65, 0x2D, 0x10D8, 0x10E1, 0x20, 0x10D4, 0x10DC, 0x10D0, 0x10D6, 0x10D4, 0x06, 0x10DA, 0x10D0,  // code-.. ........
0x10DE, 0x10D0, 0x10E0, 0x10D0, 0x10D9, 0x10DD, 0x10D1, 0x10E1, 0x20, 0x20, 0x06, 0x6
}; // phew!

const TInt KRefData8Size = sizeof(RefData8) / sizeof(RefData8[0]);
const TInt KRefDataUnicodeSize = sizeof(RefDataUnicode) / sizeof(RefDataUnicode[0]);

const CDes8 KRefText8(RefData8, KRefData8Size);
const CDes16 KRefText16(RefDataUnicode,KRefDataUnicodeSize);

#endif // !defined(__PCSTOREEXAMPLESDEF_H__)

