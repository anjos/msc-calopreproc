
%{
#include "spec.h"
#include "grammar.tab.h"

int yylineno = 1;

%}

%%
[ \t]       ;
\n          yylineno++;
struct      return STRUCT;
tag         return TAG;

[0-9]+      yylval.ivalue = strtol(yytext,0,0); return NUMBER;
[a-zA-Z][a-zA-Z0-9_]*  { 
                         yylval.str = new char[strlen(yytext) + 1];
			 strcpy(yylval.str, yytext);
                         return IDENTIFIER;
                        }
.           return *yytext;
