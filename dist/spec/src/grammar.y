
%{

  /* 
   * $Id: grammar.y,v 1.1.1.1 2000/03/13 21:03:44 rabello Exp $
   */
     
#ifdef __hpux
#include <alloca.h>
#endif /* __hpux */

#include "spec.h"

int yyerror(const char *msg);
int yylex();

/* global result */
deque<Declaration*> *definitions;

%}

%token IDENTIFIER NUMBER STRUCT TAG

%left '+' '-'
%left '*' '/'

%union {
  Declaration   *node;
  FieldList     *fieldList;
  Field         *field;
  char          *str;
  Expression    *expr;
  int           ivalue;
  vector<Expression*> *indices;
};

%type <str>  IDENTIFIER type
%type <node> definition struct_def tag_def 

%type <fieldList> field_list
%type <field>     field

%type <expr>      expression
%type <ivalue>    NUMBER

%type <indices>   opt_indices

%start definitions

%%

definitions: /* empty */
             {
	       definitions = new deque<Declaration*>;
             }
           | definitions definition
             {
	       definitions->push_back($2);
	     }
           ;

definition : struct_def
             {
	       $$ = $1;
              }
           | tag_def
             {
	       $$ = $1;
	     }
           ;

struct_def : STRUCT IDENTIFIER '{' field_list '}' ';' 
             {
	       $$ = new Struct($2, $4);
	     }
           ;

tag_def    : TAG IDENTIFIER '{' field_list '}' ';' 
             {
	       $$ = new Tag($2, $4);
	     }
           ;

field_list : /* empty */ 
             { 
	       $$ = new FieldList(); 
	     }
           | field_list field 
             {
	       $1->add($2);
	       $$ = $1;
	     }
           ;

field      : type IDENTIFIER opt_indices ';' 
             {
	       /* fix last parameter !! */
	       $$ = new Field($2, $1, $3, 0);
	     }
           ;

type       :  IDENTIFIER
           ;

opt_indices: /* empty */ 
             {
	       $$ = new vector<Expression*>;
	     }
           | opt_indices '[' expression ']'
             {
	       Number *num = $3->eval();
	       if(num != 0) {
		 $1->push_back(num);
		 delete $3;
	       } else {
		 $1->push_back($3);
	       }
	     }
           ;

expression: IDENTIFIER 
            { 
	      $$ = new Identifier($1); 
	    }
          | NUMBER 
            { 
	      $$ = new Number($1); 
	    }
          | expression '+' expression 
            {
	      $$ = new BinaryExpression(BinaryExpression::Plus,
					$1, $3);
	    }
          | expression '-' expression
            {
	      $$ = new BinaryExpression(BinaryExpression::Minus,
					$1, $3);
	    }
          | expression '*' expression
            {
	      $$ = new BinaryExpression(BinaryExpression::Times,
					$1, $3);
	    }
          | expression '/' expression
            {
	      $$ = new BinaryExpression(BinaryExpression::Div,
					$1, $3);
	    }
          ;
