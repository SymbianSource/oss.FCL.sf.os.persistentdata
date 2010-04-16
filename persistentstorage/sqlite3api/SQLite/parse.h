/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#define TK_SEMI                            1
#define TK_EXPLAIN                         2
#define TK_QUERY                           3
#define TK_PLAN                            4
#define TK_BEGIN                           5
#define TK_TRANSACTION                     6
#define TK_DEFERRED                        7
#define TK_IMMEDIATE                       8
#define TK_EXCLUSIVE                       9
#define TK_COMMIT                         10
#define TK_END                            11
#define TK_ROLLBACK                       12
#define TK_CREATE                         13
#define TK_TABLE                          14
#define TK_IF                             15
#define TK_NOT                            16
#define TK_EXISTS                         17
#define TK_TEMP                           18
#define TK_LP                             19
#define TK_RP                             20
#define TK_AS                             21
#define TK_COMMA                          22
#define TK_ID                             23
#define TK_ABORT                          24
#define TK_AFTER                          25
#define TK_ANALYZE                        26
#define TK_ASC                            27
#define TK_ATTACH                         28
#define TK_BEFORE                         29
#define TK_CASCADE                        30
#define TK_CAST                           31
#define TK_CONFLICT                       32
#define TK_DATABASE                       33
#define TK_DESC                           34
#define TK_DETACH                         35
#define TK_EACH                           36
#define TK_FAIL                           37
#define TK_FOR                            38
#define TK_IGNORE                         39
#define TK_INITIALLY                      40
#define TK_INSTEAD                        41
#define TK_LIKE_KW                        42
#define TK_MATCH                          43
#define TK_KEY                            44
#define TK_OF                             45
#define TK_OFFSET                         46
#define TK_PRAGMA                         47
#define TK_RAISE                          48
#define TK_REPLACE                        49
#define TK_RESTRICT                       50
#define TK_ROW                            51
#define TK_TRIGGER                        52
#define TK_VACUUM                         53
#define TK_VIEW                           54
#define TK_VIRTUAL                        55
#define TK_REINDEX                        56
#define TK_RENAME                         57
#define TK_CTIME_KW                       58
#define TK_ANY                            59
#define TK_OR                             60
#define TK_AND                            61
#define TK_IS                             62
#define TK_BETWEEN                        63
#define TK_IN                             64
#define TK_ISNULL                         65
#define TK_NOTNULL                        66
#define TK_NE                             67
#define TK_EQ                             68
#define TK_GT                             69
#define TK_LE                             70
#define TK_LT                             71
#define TK_GE                             72
#define TK_ESCAPE                         73
#define TK_BITAND                         74
#define TK_BITOR                          75
#define TK_LSHIFT                         76
#define TK_RSHIFT                         77
#define TK_PLUS                           78
#define TK_MINUS                          79
#define TK_STAR                           80
#define TK_SLASH                          81
#define TK_REM                            82
#define TK_CONCAT                         83
#define TK_COLLATE                        84
#define TK_UMINUS                         85
#define TK_UPLUS                          86
#define TK_BITNOT                         87
#define TK_STRING                         88
#define TK_JOIN_KW                        89
#define TK_CONSTRAINT                     90
#define TK_DEFAULT                        91
#define TK_NULL                           92
#define TK_PRIMARY                        93
#define TK_UNIQUE                         94
#define TK_CHECK                          95
#define TK_REFERENCES                     96
#define TK_AUTOINCR                       97
#define TK_ON                             98
#define TK_DELETE                         99
#define TK_UPDATE                         100
#define TK_INSERT                         101
#define TK_SET                            102
#define TK_DEFERRABLE                     103
#define TK_FOREIGN                        104
#define TK_DROP                           105
#define TK_UNION                          106
#define TK_ALL                            107
#define TK_EXCEPT                         108
#define TK_INTERSECT                      109
#define TK_SELECT                         110
#define TK_DISTINCT                       111
#define TK_DOT                            112
#define TK_FROM                           113
#define TK_JOIN                           114
#define TK_USING                          115
#define TK_ORDER                          116
#define TK_BY                             117
#define TK_GROUP                          118
#define TK_HAVING                         119
#define TK_LIMIT                          120
#define TK_WHERE                          121
#define TK_INTO                           122
#define TK_VALUES                         123
#define TK_INTEGER                        124
#define TK_FLOAT                          125
#define TK_BLOB                           126
#define TK_REGISTER                       127
#define TK_VARIABLE                       128
#define TK_CASE                           129
#define TK_WHEN                           130
#define TK_THEN                           131
#define TK_ELSE                           132
#define TK_INDEX                          133
#define TK_ALTER                          134
#define TK_TO                             135
#define TK_ADD                            136
#define TK_COLUMNKW                       137
#define TK_TO_TEXT                        138
#define TK_TO_BLOB                        139
#define TK_TO_NUMERIC                     140
#define TK_TO_INT                         141
#define TK_TO_REAL                        142
#define TK_END_OF_FILE                    143
#define TK_ILLEGAL                        144
#define TK_SPACE                          145
#define TK_UNCLOSED_STRING                146
#define TK_FUNCTION                       147
#define TK_COLUMN                         148
#define TK_AGG_FUNCTION                   149
#define TK_AGG_COLUMN                     150
#define TK_CONST_FUNC                     151
