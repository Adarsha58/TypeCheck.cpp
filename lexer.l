%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <errno.h>
    #include <limits.h>
    #include "ast.hpp"
    #include "parser.hpp"
    void yyerror(const char *);
%}

DIGIT         [0]|([1-9][0-9]*)
ID            [A-Za-z][A-Za-z0-9]*

%x comment
%%
"/*"          BEGIN(comment);

<comment>.        /* eat anything that's not a '*' */
<comment>"\n"   /* eat up '*'s not followed by '/'s */
<comment>"*/"        BEGIN(INITIAL);
<comment><<EOF>>        { yyerror("dangling comment"); }

  /* WRITEME: Copy your Flex rules from Project 3 here */
  
[ \t\n]           ; /*skip whitespace*/
"boolean"         { return T_BOOL;}
"integer"         { return T_INT;}
"none"            { return T_NONE;}

"do"              { return T_DO;}
"while"           { return T_WHILE;}
"if"              { return T_IF;}
"else"            { return T_ELSE;}
"print"           { return T_PRINT;}

"{"               { return T_OPEN_BRACE;}
"}"               { return T_CLOSED_BRACE;}

"("               { return T_OPEN_PAREN;}
")"               { return T_CLOSED_PAREN;}

";"               { return T_SEMICOLON;}
","               { return T_COMMA;}
"."               { return T_PERIOD;}

"="               { return T_ASSIGN;}
"or"              { return T_OR;}
"and"             { return T_AND;}
">"               { return T_GT;}
">="              { return T_GTE;}
"equals"          { return T_EQUALS;}
"+"               { return T_PLUS;}
"-"               { return T_MINUS;}
"*"               { return T_MULT;}
"/"               { return T_DIV;}
"not"             { return T_NOT;}
"new"             { return T_NEW;}
  /*unary minus?*/

"->"              { return T_ARROW;}
"return"          { return T_RETURN;}
"extends"         { return T_EXTEND;}

"true"            { return T_TRUE;}
"false"           { return T_FALSE;}

{ID}              { yylval.base_char_ptr = strdup(yytext); return T_ID;}
{DIGIT}           { yylval.base_int = std::atoi(yytext); return T_NUMBER;}
.                 { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}