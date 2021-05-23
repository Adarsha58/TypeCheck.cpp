%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cerrno>
    #include <climits>
    #include <limits>
     #include "ast.hpp"
    #include <iostream>
    #include "parser.hpp"
   
  
    using namespace std;

    
	void yyerror(const char *);
%}

%x IN_COMMENT

%%
"/*"              { BEGIN(IN_COMMENT); }
<IN_COMMENT>"*/"  { BEGIN(INITIAL); }
<IN_COMMENT>"\n"  { }
<IN_COMMENT>.     { }
<IN_COMMENT><<EOF>> {yyerror("incomplete comment");}

[\t\n]            ; // ignore new line new tab
" "               ; // ignore spaces
"extends"         {return T_EXTENDS;}
"return"          {return T_RETURN;}
"class"           {return T_CLASS;}
"print"           {return T_PRINT;}
"if"              {return T_IF;}
"else"            {return T_ELSE;}
"while"           {return T_WHILE;}
"equals"          {return T_EQUALS;}
"and"             {return T_AND;}
"or"              {return T_OR;}
"not"             {return T_NOT;}           
"true"            {yylval.base_int = 1; return T_TRUE;}  
"false"           {yylval.base_int = 0; return T_FALSE;}  
"new"             {return T_NEW;}  
"integer"         {return T_INT;} 
"boolean"         {return T_BOOL;} 
"none"            {return T_NONE;} 
"do"              {return T_DO;} 

">="              {return T_GTE;} 
"->"              { return T_FUNC;} 


[\(\)\{\}\*]      {return yytext[0];}
[+-/>.,;=]          {return yytext[0];}
[0-9]+            { yylval.base_int = atoi(yytext); return T_LITERAL; }
[a-zA-Z][a-zA-Z0-9]*      {yylval.base_char_ptr = strdup(yytext); return T_ID; }


.                 { yyerror("invalid character"); }
%%

int yywrap(void) {
  return 1;
}
