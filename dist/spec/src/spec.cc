
#include <stdlib.h>
#include "spec.h"
#include "codegen.h"

/* 
 * Node defines the common interface for all nodes 
 */

Node::Node()
{
}

Node::~Node()
{
}

Declaration::Declaration()
{
}

Declaration::~Declaration()
{
}

void Declaration::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}


/*
 * a field in a struct or tag
 */

Field::Field(const string& name, const string& type_name, 
	     vector<Expression*> *indices, bool optional)
  : m_name(name), m_type(type_name), m_optional(optional), m_indices(indices)
{
}

Field::~Field()
{
  delete m_indices;
}

const string& Field::name() const
{
  return m_name;
}

const string& Field::type() const
{
  return m_type;
}

const bool Field::optional() const
{
  return m_optional;
}

const vector<Expression*> *Field::indices() const
{
  return m_indices;
}

void Field::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

/*
 * a list of fields
 */

FieldList::FieldList()
{
}

FieldList::~FieldList()
{
}

void FieldList::add(Field *field)
{
  m_list.push_back(field);
}

void FieldList::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}


int  FieldList::size() const
{
  return m_list.size();
}


Field* FieldList::operator[](int index) const
{
  return m_list[index];
}


/* 
 * a structure definition 
 */

Struct::Struct(const string& name, FieldList* fields)
  : m_name(name), m_fields(fields)
{
}

Struct::~Struct()
{
  delete m_fields;
}

const string& Struct::name() const
{
  return m_name;
}

FieldList *Struct::fields() const
{
  return m_fields;
}

void Struct::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}


/* 
 * a tag definition
 */
Tag::Tag(const string& name, FieldList* fields)
  : m_name(name), m_fields(fields)
{
}

Tag::~Tag()
{
  delete m_fields;
}

const string& Tag::name() const
{
  return m_name;
}

FieldList *Tag::fields() const
{
  return m_fields;
}

void Tag::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

Expression::Expression()
{
}

Expression::~Expression()
{
}

void Expression::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

Identifier::Identifier(const string& name)
  : m_name(name)
{
}

Identifier::~Identifier()
{
}

const string& Identifier::name() const
{
  return m_name;
}

void Identifier::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

bool Identifier::is_constant() const
{
  return false;
}

Number *Identifier::eval()
{
  return 0;
}

Number::Number(int value)
  : m_value(value)
{
}
Number::~Number()
{
}

void Number::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

bool Number::is_constant() const
{
  return true;
}

int Number::value() const
{
  return m_value;
}

Number* Number::eval()
{
  return new Number(m_value);
}

BinaryExpression::BinaryExpression(Operand op, 
				   Expression* left, 
				   Expression* right)
  : m_op(op), m_left(left), m_right(right)
{
}

BinaryExpression::~BinaryExpression()
{
  delete m_left;
  delete m_right;
}

BinaryExpression::Operand BinaryExpression::op() const
{
  return m_op;
}


Expression *BinaryExpression::left() const
{
  return m_left;
}

Expression *BinaryExpression::right() const
{
  return m_right;
}

void BinaryExpression::generate(CodeGenerator& codeGen)
{
  codeGen.generate(this);
}

bool BinaryExpression::is_constant() const
{
  return false;
}

Number *BinaryExpression::eval()
{
  Number *left  = m_left->eval();
  if(left == 0)
    return 0;

  Number *right = m_right->eval();
  if(right == 0)
    return 0;

  int result;

  switch(m_op) {
  case Plus:
    result = left->value() + right->value();
    break;
  case Minus:
    result = left->value() - right->value();
    break;    
  case Times:
    result = left->value() * right->value();
    break;
  case Div:
    result = left->value() / right->value();
    break;
  default:
    abort();
  }

  delete left;
  delete right;

  return new Number(result);
}
