/* this is -*- C++ -*- */
#ifndef _CGEN_H_
#define _CGEN_H_

#include <iostream.h>
#include <fstream.h>
#include "spec.h"
#include "codegen.h"

/* 
 * code generator implementation for C
 */


// common base class for CPPInput and CPPOutput
// generates code for expression
class CInOut : public CodeGenerator {
public:
  CInOut(ostream& os);
  virtual ~CInOut();
public:
  virtual void generate(BinaryExpression *);
  virtual void generate(Identifier *);
  virtual void generate(Number *);
protected:			// implementation
  ostream& m_output;
};

// generates class declarations
class CDeclarations : public CInOut {
public:
  CDeclarations(const string& basename, ostream& os);
  virtual ~CDeclarations();

public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);

private:			// implementation
  string   m_basename;
};

// generates input code
class CInput : public CInOut {
public:
  CInput(const string& basename, ostream& os);
  virtual ~CInput();
public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
};

// generates output code
class COutput : public CInOut {
public:
  COutput(ostream& os);
  virtual ~COutput();
public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
};

class CFree : public CInOut {
public:
  CFree(ostream &os);
  virtual ~CFree();
public:
  virtual void generate(Field *);
  virtual void generate(Struct *);
  virtual void generate(Tag *);
};


#endif /* _CGEN_H */
