/* this is -*- C++ -*- */
#ifndef _CODEGEN_H_
#define _CODEGEN_H_

class Node;
/* 
 * abstract interface for code generator
 */

class CodeGenerator {
public:
  CodeGenerator();
  virtual ~CodeGenerator();

public:
  virtual void generate(Node *);
  virtual void generate(Field *);
  virtual void generate(FieldList *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
  virtual void generate(Expression *);
  virtual void generate(BinaryExpression *);
  virtual void generate(Identifier *);
  virtual void generate(Number *);
};

#endif /* _CODEGEN_H_ */
