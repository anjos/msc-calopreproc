/* this is -*- C++ -*- */
#ifndef _CPPGEN_H_
#define _CPPGEN_H_

#include <iostream.h>
#include <fstream.h>
#include "spec.h"
#include "codegen.h"

/* 
 * implementation for C++
 */

// generates class declarations
class CPPDeclarations : public CodeGenerator {
public:
  CPPDeclarations(const string& basename, ostream& os);
  virtual ~CPPDeclarations();

public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);

private:			// implementation
  ostream& m_output;
  string   m_basename;
};

// common base class for CPPInput and CPPOutput
// generates code for expression
class CPPInOut : public CodeGenerator {
public:
  CPPInOut(ostream& os);
  virtual ~CPPInOut();
public:
  virtual void generate(BinaryExpression *);
  virtual void generate(Identifier *);
  virtual void generate(Number *);
protected:			// implementation
  ostream& m_output;
};

// generates input code
class CPPInput : public CPPInOut {
public:
  CPPInput(const string& basename, ostream& os);
  virtual ~CPPInput();
public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
};

// generates output code
class CPPOutput : public CPPInOut {
public:
  CPPOutput(ostream& os);
  virtual ~CPPOutput();
public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
};


#endif /* _CPPGEN_H */
