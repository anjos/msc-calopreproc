/* this is -*- C++ -*- */
#ifndef _SPEC_H_
#define _SPEC_H_

/*
 * $Id: spec.h,v 1.1 2000/03/13 21:03:43 rabello Exp $
 */

#include <string>
#include <deque>
#include <vector>

// using std::vector;
// using std::deque;
// using std::string;

/*
 * nodes for abstract syntax tree
 */

class CodeGenerator;

/* 
 * Node defines the common interface for all nodes 
 */
class Node {
public:				// creation
  Node();
  virtual ~Node();
};

class Declaration : public Node {
public:
  Declaration();
  ~Declaration();
public:				// code generation
  virtual void generate(CodeGenerator& codeGen);
};

/* coming later... */
class Expression;

/* 
 * a field in a struct or tag 
 */
class Field : public Declaration {

public:
  Field(const string& name, 
	const string& type_name, 
	vector<Expression*> *indices,
	bool optional);
  virtual ~Field();

public:				// access
  const string&             name() const;
  const string&             type() const;
  const bool                optional() const;
  const vector<Expression*> *indices() const;

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);

private:			// representation
  string m_name;
  string m_type;
  bool   m_optional;
  vector<Expression*> *m_indices;
};

class FieldList : public Declaration {
public:				// creation
  FieldList();
  virtual ~FieldList();

public:				// access
  void add(Field *field);

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);

private:
  typedef deque<Field*> FieldListRep;
  FieldListRep m_list;

public:
  int  size() const;
  Field* operator[](int index) const;
};

/* 
 * a structure definition 
 */
class Struct : public Declaration {
public:
  Struct(const string& name, FieldList* fields);
  virtual ~Struct();

public:				// access
  const string& name() const;
  FieldList     *fields() const;

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);

private:			// representation
  string    m_name;
  FieldList *m_fields;
};

/* 
 * a tag definition
 */
class Tag : public Declaration {
public:				// creation
  Tag(const string& name, FieldList* fields);
  virtual ~Tag();

public:				// access
  const string& name() const;
  FieldList     *fields() const;

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);

private:			// representation
  string     m_name;
  FieldList  *m_fields;
};

class Number;

class Expression : public Node {
public:
  Expression();
  virtual ~Expression();
public:
  virtual void generate(CodeGenerator& codeGen);
  virtual bool is_constant() const = 0;
  virtual Number *eval() = 0;
};

class Identifier : public Expression {
public:				// creation
  Identifier(const string& name);
  virtual ~Identifier();

public:				// access
  const string& name() const;

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);
  virtual bool is_constant() const;
  virtual Number *eval();
private:
  string m_name;
};

class Number : public Expression {
public:				// creation
  Number(int value);
  virtual ~Number();

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);
  virtual bool is_constant() const;
  virtual Number *eval();

public:				// access
  int value() const;

private:
  int  m_value;
};

class BinaryExpression : public Expression {
public:
  enum Operand { Plus, Minus, Times, Div };

public:
  BinaryExpression(Operand op, Expression* left, Expression* right);
  virtual ~BinaryExpression();

public:				// access
  Operand op() const;
  Expression *left() const;
  Expression *right() const;

public:				// code generation
  virtual void generate(CodeGenerator& codeGen);
  virtual bool is_constant() const;
  virtual Number *eval();

private:
  Operand    m_op;
  Expression *m_left, *m_right;
};

#endif /* _SPEC_H_ */
