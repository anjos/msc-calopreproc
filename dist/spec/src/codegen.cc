
#include "spec.h"
#include "codegen.h"

CodeGenerator::CodeGenerator()
{
  /* default : nothing */
}

CodeGenerator::~CodeGenerator()
{
  /* default : nothing */
}

void CodeGenerator::generate(Node *)
{
  /* default : nothing */
}


void CodeGenerator::generate(Field *)
{
  /* default : nothing */
}

void CodeGenerator::generate(FieldList *fields)
{
  /* default : call generate_declaration for each field in the list */
  for(int i = 0; i < fields->size(); i++)
    (*fields)[i]->generate(*this);
}


void CodeGenerator::generate(Struct *)
{
 /* default : nothing */
}


void CodeGenerator::generate(Tag *)
{
  /* default : nothing */
}


void CodeGenerator::generate(Expression *)
{ 
  /* default : nothing */
}

void CodeGenerator::generate(BinaryExpression *)
{
  /* default : nothing */
}

void CodeGenerator::generate(Identifier *)
{
  /* default : nothing */
}

void CodeGenerator::generate(Number *)
{
  /* default : nothing */
}

