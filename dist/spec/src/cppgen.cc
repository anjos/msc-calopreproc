
#include <iostream.h>
#include "cppgen.h"

/*
 * code generation implementation for C++
 */

/*
 * constructor
 * write prologue
 */

CPPDeclarations::CPPDeclarations(const string& basename, ostream& os)
  : m_output(os), m_basename(basename)
{
  // write prologue for header
  m_output << "/* this is -*- c++ -*- */" << endl;
  m_output << "#ifndef _" << basename << "_h_" << endl;
  m_output << "#define _" << basename << "_h_" << endl << endl;

  m_output << "#include <string>"  << endl;
  m_output << "#include <iostream.h>" << endl;
  m_output << "#include <vector.h>" << endl  << endl;

}

/*
 * destructor for C++ code generator
 * write epilogues
 */
CPPDeclarations::~CPPDeclarations()
{

  // write epilogue for header
  m_output << "#endif // _" << m_basename << "_h_" << endl;

}

// generate class member declaration for field
void CPPDeclarations::generate(Field *field)
{
  if(field->indices()->size() > 0) {
    m_output << "    vector<" << field->type() << ">\t" 
		<< field->name() << ";" << endl;
  } else {
    m_output << "    " << field->type() << "\t" 
		<< field->name() << ";" << endl;		
  }
}

// generate class declaration for 'struct'
void CPPDeclarations::generate(Struct *struc)
{
  // class definition
  m_output << "class " << struc->name() << " { " << endl
	      << "public:" << endl;

  // default constructor
  m_output << "    " << struc->name()  << "() {};" << endl;

  // copy constructor
  //   m_output << "    " << struc->name()  << "(const "
  //               << struc->name() << "& other);" << endl;

  // assignment
  // m_output << "    " << struc->name() << "& operator=(const " 
  //             << struc->name()  << "& );" << endl;

  // member fields
  m_output << "public:" << endl;
  struc->fields()->generate(*this);
  
  // input operator
  m_output << "    friend istream& operator>>(istream&, "
	      << struc->name() << "& x);" << endl;

  // output operator
  m_output << "    friend ostream& operator<<(ostream&, const "
	      << struc->name() << "& x);" << endl;

  m_output << "};" << endl << endl;
}


// generate class declaration for 'tag'
void CPPDeclarations::generate(Tag *tag)
{
  // class definition
  m_output << "class " << tag->name() << " { " << endl
	      << "public:" << endl;

  // default constructor
  m_output << "    " << tag->name()  << "() {};" << endl;

  // copy constructor
  // m_output << "    " << tag->name() << "(const "
  //             << tag->name() << "& other);" << endl;

  // assignment
  // m_output << "    " << tag->name() << "& operator=(const " 
  //             << tag->name()  << "& );" << endl;

  // member fields
  m_output << "public:" << endl;
  tag->fields()->generate(*this);

  m_output << "    friend istream& operator>>(istream&, "
	      << tag->name() << "& x);" << endl;

  // output operator
  m_output << "    friend ostream& operator<<(ostream&, const "
	      << tag->name() << "& x);" << endl;
  
  m_output  << "};" << endl << endl;
}

/*
 * CPPInOut
 */
CPPInOut::CPPInOut(ostream& os)
  : m_output(os)
{
}

CPPInOut::~CPPInOut()
{
}


// generate code for a binary expression
void CPPInOut::generate(BinaryExpression *expr)
{
  expr->left()->generate(*this);
  switch(expr->op()) {
  case BinaryExpression::Plus:
    m_output << " + ";
    break;
  case BinaryExpression::Minus:
    m_output << " - ";
    break;
  case BinaryExpression::Times:
    m_output << " * ";
    break;
  case BinaryExpression::Div:    
    m_output << " / ";
    break;
  default:
    cerr << "error in BinaryExpression::generate_code: invalid operation" << endl;
    break;
  }
  expr->right()->generate(*this);
}

// generate code for identifier
void CPPInOut::generate(Identifier *ident)
{
  m_output << "x." << ident->name();
}

// generate code for simple integer number
void CPPInOut::generate(Number *number)
{
  m_output << number->value();
}


/*
 * CPPInput
 */ 

CPPInput::CPPInput(const string& basename, ostream& os)
  : CPPInOut(os)
{
  // write prologue for implementation
  m_output << endl << "#include \"" << basename << ".h\"" << endl << endl;
}

CPPInput::~CPPInput()
{
}

// generate code to read from input stream
void CPPInput::generate(Field *field)
{
  if(field->indices()->size() > 0) {

    // todo: only for one dimension at the moment...
    Expression *expr = *field->indices()->begin();

    m_output << "    for(int i = 0; i < ";
    expr->generate(*this);
    m_output << "; i++) {" << endl;
    m_output << "        is >> x." << field->name() << "[i];" << endl;
    m_output << "    }" << endl;
    
  } else {
    m_output << "    is >> x." << field->name() << ";" << endl;
  }
}

// generate code to read structure from input stream
void CPPInput::generate(Struct *struc)
{
  m_output << "istream& operator>>(istream& is, "
	    << struc->name() << "& x)" << endl
	    << "{" << endl;

  struc->fields()->generate(*this); 

  m_output << "}" << endl << endl;
}

// generate code to read 'tag' from input stream
void CPPInput::generate(Tag *tag)
{
  m_output << "istream& operator>>(istream& is, "
	    << tag->name() << "& x)" << endl
	    << "{" << endl;

  tag->fields()->generate(*this);  

  m_output << "}" << endl << endl;
}



CPPOutput::CPPOutput(ostream& os)
  : CPPInOut(os)
{
}

CPPOutput::~CPPOutput()
{
}

// generate code to write to output stream
void CPPOutput::generate(Field *field)
{
  if(field->indices()->size() > 0) {

    // todo: only for one dimension at the moment...
    Expression *expr = *field->indices()->begin();

    m_output << "    for(int i = 0; i < ";
    expr->generate(*this);
    m_output << "; i++) {" << endl;
    m_output << "        os << \" \" << x." << field->name() << "[i];" << endl;
    m_output << "    }" << endl;
    
  } else {
    m_output << "    os << \" \" << x." << field->name() << ";" << endl;
  }
}



// generate code to write structure to output stream
void CPPOutput::generate(Struct *struc)
{
  m_output << "ostream& " << "operator<<(ostream &os,const " 
	    << struc->name() << "& x)" 
	    << endl << "{" << endl;
  
  struc->fields()->generate(*this);
  m_output << "    os << endl;" << endl;
  m_output << "}" << endl << endl;

}



// generate code to write 'tag' to output stream
void CPPOutput::generate(Tag *tag)
{
  // start of function
  m_output << "ostream& " << "operator<<(ostream &os,const " 
	    << tag->name() << "& x)" 
	    << endl << "{" << endl;

  // output start tag
  m_output << "    os << endl << \"{ " << tag->name() << "\" << endl;" << endl;
  // output fields
  tag->fields()->generate(*this); 
  // output end tag
  m_output << "    os << endl << \"} " << tag->name() << "\" << endl;" << endl;

  // end of function
  m_output << "}" << endl << endl;
}

