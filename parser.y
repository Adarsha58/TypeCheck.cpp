%{
    #include <cstdlib>
    #include <cstdio>
    #include <iostream>
     #include "ast.hpp"

    #define YYDEBUG 1
    #define YYINITDEPTH 10000
    
    int yylex(void);
    void yyerror(const char *);
    extern ASTNode* astRoot;


    using namespace std;
%}

%define parse.error verbose



/* Statement token */
%token T_CLASS T_PRINT T_IF T_ELSE T_RETURN T_NEW T_WHILE T_DO T_EXTENDS T_FUNC

/* Literal token */
%token T_INT T_BOOL T_NONE //actual types NAME

/* ID and value */
%token T_ID T_LITERAL

/* Expression token */
%token T_GTE T_EQUALS T_UNARY T_AND T_OR T_NOT T_TRUE T_FALSE  T_EOF



/* WRITEME: Specify precedence here */
%left T_OR
%left T_AND
%left T_GTE '>' T_EQUALS
%left '+' '-'
%left '*' '/'
%precedence T_NOT 


%type <program_ptr> Start
%type <class_list_ptr> CLASSES
%type <class_ptr> CLASS ClassBody
%type <declaration_list_ptr> MEMBERS DECLARATIONS
%type <method_list_ptr> METHODS
%type <parameter_list_ptr> PARAMETERLIST PARAMETERS
%type <type_ptr> TYPE
%type <methodbody_ptr> BODY
%type <statement_list_ptr> STATEMENTS
%type <returnstatement_ptr> OPT_RETURN
%type <identifier_list_ptr> IDS
%type <expression_ptr> EXPRESSION
%type <assignment_ptr> ASSIGMEMT
%type <methodcall_ptr> METHOD_CALL
%type <ifelse_ptr> IF_ELSE
%type <while_ptr> WHILE_LOOP
%type <dowhile_ptr> DO_WHILE
%type <print_ptr> PRINT
%type <expression_list_ptr> ARGUMENTLIST ARGUMENTS
%type <statement_ptr> STATEMENT

%type <base_int> T_LITERAL T_TRUE T_FALSE
%type <base_char_ptr> T_ID







%%

Start : CLASSES {$$ = new ProgramNode($1); astRoot = $$;}
      ;

CLASSES : CLASS {$$ = new list<ClassNode*>(); $$->push_back($1);}
        | CLASSES CLASS {$$ = $1; $1->push_back($2);}
        ;

CLASS : T_ID '{' ClassBody '}' {$$ = new ClassNode(new IdentifierNode($1), NULL, $3->declaration_list, $3->method_list);}
      | T_ID T_EXTENDS T_ID '{' ClassBody '}' {$$ = new ClassNode(new IdentifierNode($1), new IdentifierNode($3), $5->declaration_list, $5->method_list);}
      ;


TYPE : T_ID  {$$ = new ObjectTypeNode(new IdentifierNode($1));}
     | T_INT {$$ = new IntegerTypeNode();}
     | T_BOOL {$$ = new BooleanTypeNode();}
     | T_NONE {$$ = new NoneNode();}
     ;


ClassBody : MEMBERS METHODS {$$ = new ClassNode(NULL, NULL, $1, $2);}
          | MEMBERS {$$ = new ClassNode(NULL, NULL, $1, NULL);}
          | METHODS {$$ = new ClassNode(NULL, NULL, NULL, $1);}
          | %empty  {$$ = new ClassNode(NULL, NULL, NULL, NULL);}
          ;

MEMBERS : MEMBERS TYPE T_ID ';' {$$ = $1;  $$->push_back(new DeclarationNode($2, new list<IdentifierNode*>(1, new IdentifierNode($3))));}
        | TYPE T_ID ';'  {$$ = new list <DeclarationNode*>();   $$->push_back(new DeclarationNode($1, new list<IdentifierNode*>(1, new IdentifierNode($2)))); }
        ;


METHODS : METHODS T_ID '(' PARAMETERS ')' T_FUNC TYPE '{' BODY '}'  {$$ = $1; $$->push_back( new MethodNode( new IdentifierNode($2), $4, $7,  $9 ) ); }
        | T_ID '(' PARAMETERS ')' T_FUNC TYPE '{' BODY '}' {$$ = new list<MethodNode*>(); $$->push_back(new MethodNode(new IdentifierNode($1), $3, $6, $8)); }
        ;

BODY: DECLARATIONS STATEMENTS OPT_RETURN {$$ = new MethodBodyNode($1, $2, $3);}
        | DECLARATIONS OPT_RETURN  {$$ = new MethodBodyNode($1, NULL, $2);}
        | STATEMENTS OPT_RETURN  {$$ = new MethodBodyNode(NULL, $1, $2);}
        | OPT_RETURN {$$ = new MethodBodyNode(NULL, NULL, $1);}
        ;


PARAMETERS : PARAMETERLIST {$$ = $1; }
           | %empty  {$$ = NULL;} ;


PARAMETERLIST : TYPE T_ID  {$$ = new list<ParameterNode*>(); $$->push_back(new ParameterNode($1, new IdentifierNode($2))); }
          | PARAMETERLIST ',' TYPE T_ID  {$$ = $1; $$->push_back(new ParameterNode($3, new IdentifierNode($4))); } ;


DECLARATIONS : DECLARATIONS TYPE IDS ';' {$$ = $1; $$->push_back(new DeclarationNode($2, $3)); }
        | TYPE IDS ';' {$$ = new list<DeclarationNode*>(); $$->push_back(new DeclarationNode($1, $2)); }
        ;

IDS : IDS ',' T_ID {$$ = $1; $$->push_back(new IdentifierNode($3));}
    | T_ID {$$ = new list<IdentifierNode*>(); $$->push_back(new IdentifierNode($1));}
    ;

STATEMENTS : STATEMENTS STATEMENT  {$$ = $1; $$->push_back($2);}
           | STATEMENT {$$ = new list<StatementNode*>(); $$->push_back($1);}
           ;


STATEMENT : ASSIGMEMT {$$ = $1;}
          | METHOD_CALL ';'  {$$ = new CallNode($1);}
          | IF_ELSE  {$$ = $1;}
          | WHILE_LOOP {$$ = $1;}
          | DO_WHILE {$$ = $1;}
          | PRINT {$$ = $1;}
          ;


ASSIGMEMT : T_ID '=' EXPRESSION ';' {$$ = new AssignmentNode(new IdentifierNode($1), NULL, $3);}
          | T_ID '.' T_ID  '=' EXPRESSION ';' {$$ = new AssignmentNode(new IdentifierNode($1), new IdentifierNode($3), $5);}
          ;


IF_ELSE : T_IF EXPRESSION '{' STATEMENTS '}' {$$ = new IfElseNode($2, $4, NULL);}
        | T_IF EXPRESSION '{' STATEMENTS '}' T_ELSE '{' STATEMENTS '}' {$$ = new IfElseNode($2, $4, $8);}
        ;

WHILE_LOOP : T_WHILE EXPRESSION '{' STATEMENTS '}' {$$ = new WhileNode($2, $4);}
           ;

DO_WHILE : T_DO '{' STATEMENTS '}' T_WHILE '(' EXPRESSION ')' ';' {$$ = new DoWhileNode($3, $7);} ;

PRINT : T_PRINT EXPRESSION ';' {$$ = new PrintNode($2);};




EXPRESSION : EXPRESSION '+' EXPRESSION {$$ = new PlusNode($1, $3);}
           | EXPRESSION '-' EXPRESSION {$$ = new MinusNode($1, $3);}
           | EXPRESSION '*' EXPRESSION {$$ = new TimesNode($1, $3);}
           | EXPRESSION '/' EXPRESSION {$$ = new DivideNode($1, $3);}
           | EXPRESSION '>' EXPRESSION {$$ = new GreaterNode($1, $3);} 
           | EXPRESSION T_GTE EXPRESSION {$$ = new GreaterEqualNode($1, $3);}
           | EXPRESSION T_EQUALS EXPRESSION {$$ = new EqualNode($1, $3);}
           | EXPRESSION T_AND EXPRESSION {$$ = new AndNode($1, $3);}
           | EXPRESSION T_OR EXPRESSION{$$ = new OrNode($1, $3);}
           | T_NOT EXPRESSION {$$ = new NotNode($2);}
           | '-' EXPRESSION %prec T_NOT {$$ = new NegationNode($2);}
           | T_ID {$$ = new VariableNode(new IdentifierNode($1));}
           | T_ID '.' T_ID {$$ = new MemberAccessNode(new IdentifierNode($1), new IdentifierNode($3));}
           | METHOD_CALL {$$ = $1;}
           | '(' EXPRESSION ')' {$$ = $2;}
           | T_LITERAL {$$ = new IntegerLiteralNode(new IntegerNode($1));}
           | T_TRUE {$$ = new BooleanLiteralNode(new IntegerNode($1));}
           | T_FALSE {$$ = new BooleanLiteralNode(new IntegerNode($1));}
           |  T_NEW  T_ID {$$ = new NewNode(new IdentifierNode($2), NULL);}
           | T_NEW T_ID '(' ARGUMENTS ')' {$$ = new NewNode(new IdentifierNode($2), $4);} ;
           
METHOD_CALL : T_ID '(' ARGUMENTS ')' {$$ = new MethodCallNode(new IdentifierNode($1), NULL, $3);}
        | T_ID '.' T_ID '(' ARGUMENTS ')' {$$ = new MethodCallNode(new IdentifierNode($1), new IdentifierNode($3), $5);}
        ;

ARGUMENTS : ARGUMENTLIST  {$$ = $1; }
          | %empty {$$ = NULL; };


ARGUMENTLIST : ARGUMENTLIST ',' EXPRESSION {$$ = $1; $$->push_back($3);}
          | EXPRESSION {$$ = new list<ExpressionNode*>(); $$->push_back($1);} ;


OPT_RETURN: T_RETURN EXPRESSION ';' {$$ = new ReturnStatementNode($2);}
        | %empty  {$$ = NULL;}
        ;
        


/* WRITME: Write your Bison grammar specification here */

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}
