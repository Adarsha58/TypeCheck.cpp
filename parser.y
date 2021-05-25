  
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <iostream>

    #include "ast.hpp"
    
    #define YYDEBUG 1
    #define YYINITDEPTH 10000
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

/*%error-verbose*/
%define parse.error verbose

/* WRITEME: Copy your token and precedence specifiers from Project 3 here */
%token T_BOOL T_INT T_NONE
%token T_DO T_WHILE 
%token T_IF T_ELSE
%token T_ID 
%token T_NUMBER
%token T_NEW
%token T_ARROW
%token T_TRUE T_FALSE
%token T_OR T_AND
%token T_GT T_GTE T_EQUALS
%token T_OPEN_BRACE T_CLOSED_BRACE 
%token T_OPEN_PAREN T_CLOSED_PAREN
%token T_MULT T_DIV 
%token T_PLUS T_MINUS
%token T_NOT
%token T_ASSIGN
%token T_PRINT
%token T_COMMA 
%token T_PERIOD
%token T_SEMICOLON
%token T_RETURN
%token T_EXTEND
%token T_EOF

/* precedence is specified here*/
%left T_OR T_AND 
%left T_GTE T_GT T_EQUALS 
%left T_PLUS T_MINUS
%left T_MULT T_DIV
%precedence T_NOT
/* WRITEME: Specify types for all nonterminals and necessary terminals here */
%type <program_ptr> Start

%type <class_ptr> Class ClassBody
%type <class_list_ptr> Classes

%type <methodcall_ptr> MethodCall
%type <expression_ptr> Expr;
%type <expression_list_ptr> Argument Arguments
%type <parameter_list_ptr> Parameter Parameters
%type <assignment_ptr> Assignment
%type <ifelse_ptr> IfElse
%type <while_ptr> While
%type <dowhile_ptr> DoWhile

%type <type_ptr> Type 

%type <methodbody_ptr> Body 
%type <method_list_ptr> Methods

%type <declaration_ptr> Declaration 
%type <declaration_list_ptr> Declarations Members

%type <statement_list_ptr> Statements 
%type <statement_ptr> Statement 
%type <returnstatement_ptr> Return

%type <base_char_ptr> T_ID; 
%type <base_int> T_NUMBER

%%
/* WRITEME: This rule is a placeholder. Replace it with your grammar
            rules from Project 3 */
/* originally Start : Class */
Start : Classes            { $$ = new ProgramNode($1); astRoot = $$;}
      ;

Classes : Classes Class { $$ = $1; $$->push_back($2);}
      | %empty          { $$ = new std::list<ClassNode*>();}
      ;

Class : T_ID T_OPEN_BRACE ClassBody T_CLOSED_BRACE                { $$ = $3; $$->identifier_1 = new IdentifierNode($1);}
      | T_ID T_EXTEND T_ID T_OPEN_BRACE ClassBody T_CLOSED_BRACE  { $$ = $5; $$->identifier_1 = new IdentifierNode($1); $$->identifier_2 = new IdentifierNode($3);}
      ;

ClassBody : Members Methods               { $$ = new ClassNode(NULL, NULL,$1,$2);}
          | Members                       { $$ = new ClassNode(NULL, NULL,$1,NULL);}
          | Methods                       { $$ = new ClassNode(NULL, NULL,NULL,$1);}
          |%empty                         { $$ = new ClassNode(NULL, NULL,NULL,NULL);}
          ;

Members : Members Type T_ID T_SEMICOLON  { $$ = $1; std::list<IdentifierNode*>* list1 = new std::list<IdentifierNode*>(); list1->push_back(new IdentifierNode($3)); $$->push_back(new DeclarationNode($2,list1));}
        | Type T_ID T_SEMICOLON          { $$ = new std::list<DeclarationNode*>(); std::list<IdentifierNode*>* list1 = new std::list<IdentifierNode*>(); list1->push_back(new IdentifierNode($2)); $$->push_back(new DeclarationNode($1,list1));}
        ;

Methods : Methods T_ID T_OPEN_PAREN Parameter T_CLOSED_PAREN T_ARROW Type T_OPEN_BRACE Body T_CLOSED_BRACE  { $$ = $1; $$->push_back(new MethodNode(new IdentifierNode($2),$4,$7,$9));}
        | T_ID T_OPEN_PAREN Parameter T_CLOSED_PAREN T_ARROW Type T_OPEN_BRACE Body T_CLOSED_BRACE          { $$ = new std::list<MethodNode*>(); $$->push_back(new MethodNode(new IdentifierNode($1),$3,$6,$8));}
        ;
 
Body : Declarations Statements Return     { $$ = new MethodBodyNode($1, $2, $3);}
     | Declarations Statements            { $$ = new MethodBodyNode($1, $2, NULL);}
     | Declarations                       { $$ = new MethodBodyNode($1, NULL, NULL);}
     | Statements                         { $$ = new MethodBodyNode(NULL, $1, NULL);}
     | Declarations Return                { $$ = new MethodBodyNode($1, NULL, $2);}
     | Statements Return                  { $$ = new MethodBodyNode(NULL, $1, $2);}
     | Return                             { $$ = new MethodBodyNode(NULL, NULL, $1);}
     | %empty                             { $$ = new MethodBodyNode(NULL, NULL, NULL);}
     ;

Type : T_BOOL     { $$ = new BooleanTypeNode();}
     | T_INT      { $$ = new IntegerTypeNode();}
     | T_ID       { $$ = new ObjectTypeNode(new IdentifierNode($1));}
     | T_NONE     { $$ = new NoneNode();}
     ;

Declarations : Declarations Declaration T_SEMICOLON   { $$ = $1; $$->push_back($2);}
             | Declaration T_SEMICOLON                { $$ = new std::list<DeclarationNode*>(); $$->push_back($1);}
             ;

Declaration : Declaration T_COMMA T_ID    { $$ = $1; $$->identifier_list->push_back(new IdentifierNode($3));}
             | Type T_ID                  { $$ = new DeclarationNode($1, new std::list<IdentifierNode*>()); $$->identifier_list->push_back(new IdentifierNode($2)); }
             ;

Statements : Statements Statement         { $$ = $1; $$->push_back($2);}
           | Statement                    { $$ = new std::list<StatementNode*>(); $$->push_back($1);}
           ;

Statement : Assignment                    { $$ = $1;}
          | MethodCall T_SEMICOLON        { $$ = new CallNode($1);}
          | IfElse                        { $$ = $1;}
          | While                         { $$ = $1;}
          | DoWhile                       { $$ = $1;}
          | T_PRINT Expr T_SEMICOLON      { $$ = new PrintNode($2);}
          ;
          
IfElse : T_IF Expr T_OPEN_BRACE Statements T_CLOSED_BRACE       { $$ = new IfElseNode($2, $4, NULL);}
       | T_IF Expr T_OPEN_BRACE Statements T_CLOSED_BRACE T_ELSE T_OPEN_BRACE Statements T_CLOSED_BRACE { $$ = new IfElseNode($2,$4,$8);}
       ;

While : T_WHILE Expr T_OPEN_BRACE Statements T_CLOSED_BRACE       { $$ = new WhileNode($2,$4);}
      ;

DoWhile : T_DO T_OPEN_BRACE Statements T_CLOSED_BRACE T_WHILE T_OPEN_PAREN Expr T_CLOSED_PAREN T_SEMICOLON { $$ = new DoWhileNode($3,$7);} 
        ; 

Assignment : T_ID T_ASSIGN Expr T_SEMICOLON                 { $$ = new AssignmentNode(new IdentifierNode($1), NULL, $3);}
           | T_ID T_PERIOD T_ID T_ASSIGN Expr T_SEMICOLON   { $$ = new AssignmentNode(new IdentifierNode($1), new IdentifierNode($3), $5);}
           ;

Parameter : Parameters                    { $$ = $1;}
         | %empty                         { $$ = new std::list<ParameterNode*>();}
         ;

Parameters : Parameters T_COMMA Type T_ID { $$ = $1; $$ ->push_back(new ParameterNode($3, new IdentifierNode($4)));}
          | Type T_ID                     { $$ = new std::list<ParameterNode*>(); $$->push_back(new ParameterNode($1, new IdentifierNode($2)));}
          ;

Return : T_RETURN Expr T_SEMICOLON        {$$ = new ReturnStatementNode($2);}
       ;

Expr : Expr T_PLUS Expr                   { $$ = new PlusNode($1, $3);}
     | Expr T_MINUS Expr                  { $$ = new MinusNode($1, $3);}      
     | Expr T_MULT Expr                   { $$ = new TimesNode($1, $3);}
     | Expr T_DIV Expr                    { $$ = new DivideNode($1, $3);}
     | Expr T_GT Expr                     { $$ = new GreaterNode($1, $3);}
     | Expr T_GTE Expr                    { $$ = new GreaterEqualNode($1, $3);}
     | Expr T_EQUALS Expr                 { $$ = new EqualNode($1, $3);}
     | Expr T_AND Expr                    { $$ = new AndNode($1, $3);}
     | Expr T_OR Expr                     { $$ = new OrNode($1, $3);}
     | T_NOT Expr                         { $$ = new NotNode($2);}
     | T_MINUS Expr     %prec T_NOT       { $$ = new NegationNode($2);}
     | T_ID                               { $$ = new VariableNode(new IdentifierNode($1));}
     | T_ID T_PERIOD T_ID                 { $$ = new MemberAccessNode(new IdentifierNode($1), new IdentifierNode($3));}
     | MethodCall                         { $$ = $1;}
     | T_OPEN_PAREN Expr T_CLOSED_PAREN   { $$ = $2;}
     | T_NUMBER                           { $$ = new IntegerLiteralNode(new IntegerNode($1)); }
     | T_TRUE                             { $$ = new BooleanLiteralNode(new IntegerNode(1));}
     | T_FALSE                            { $$ = new BooleanLiteralNode(new IntegerNode(0));}
     | T_NEW T_ID                         { $$ = new NewNode(new IdentifierNode($2), new std::list<ExpressionNode*>());}
     | T_NEW T_ID T_OPEN_PAREN Argument T_CLOSED_PAREN { $$ = new NewNode(new IdentifierNode($2),$4);}
     ;

MethodCall : T_ID T_OPEN_PAREN Argument T_CLOSED_PAREN  {$$ = new MethodCallNode(new IdentifierNode($1) , NULL , $3);}
           | T_ID T_PERIOD T_ID T_OPEN_PAREN Argument T_CLOSED_PAREN {$$ = new MethodCallNode(new IdentifierNode($1),new IdentifierNode($3),$5);}
           ;

Argument : Arguments                      { $$ = $1;}
         | %empty                         { $$ = new std::list<ExpressionNode*>();}
         ;

Arguments : Arguments T_COMMA Expr        { $$ = $1; $$ ->push_back($3);}
          | Expr                          { $$ = new std::list<ExpressionNode*>(); $$->push_back($1);}
          ; 
%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(0);
}