
/*
 * $Id: cgen.cc,v 1.1.1.1 2000/03/13 21:03:44 rabello Exp $
 */

#include "cgen.h"

/*
 * code generation implementation for C
 */

/*
 * constructor
 * write prologue
 */

CDeclarations::CDeclarations(const string& basename, ostream& os)
  :  CInOut(os), m_basename(basename)
{
  // write prologue for header
  m_output << "#ifndef _" << basename << "_h_" << endl;
  m_output << "#define _" << basename << "_h_" << endl << endl;

  m_output << "#include <stdlib.h>" << endl;
  m_output << "#include <string.h>" << endl;
  m_output << "#include <stdio.h>"  << endl << endl;

  m_output << "#include \"atrdmplib.h\"" << endl << endl;
}

/*
 * destructor for C++ code generator
 * write epilogues
 */
CDeclarations::~CDeclarations()
{

  // write epilogue for header
  m_output << "#endif /* _" << m_basename << "_h_ */" << endl;

}

// generate class member declaration for field
void CDeclarations::generate(Field *field)
{
  if(field->indices()->size() > 0) {
    Expression *expr = *field->indices()->begin();
    if(expr->is_constant()) {
      m_output << "    " << field->type() << "\t" << field->name() 
	       << "[";
      expr->generate(*this);
      m_output << "];" << endl;
    } else {
      m_output << "    " << field->type() << "*\t" 
	       << field->name() << ";" << endl;
    }
  } else {
    m_output << "    " << field->type() << "\t" 
		<< field->name() << ";" << endl;	
  }
}

// generate class declaration for 'struct'
void CDeclarations::generate(Struct *struc)
{
  // class definition
  m_output << "typedef struct " << struc->name() << " { " << endl;

  // member fields
  struc->fields()->generate(*this);

  m_output << "} " << struc->name() << ";" << endl << endl;
  
  // input operator
  m_output << "int read_" << struc->name() 
	   << "(FILE *fp, struct " << struc->name() << " *data);" 
	   << endl;

  // output operator
  m_output << "int write_" << struc->name() 
	   << "(FILE *fp, struct " << struc->name() << " *data);" 
	   << endl;

  m_output << "void free_" << struc->name()
	   << "(" << struc->name() << " *);" << endl << endl;

  m_output << "#define create_" << struc->name() 
	   << "s(n) (" << struc->name() 
	   << "* )malloc((n) * sizeof(" << struc->name() 
	   << "))" << endl << endl;
}


// generate class declaration for 'tag'
void CDeclarations::generate(Tag *tag)
{
  // class definition
  m_output << "typedef struct " << tag->name() << " { " << endl;

  // member fields
  tag->fields()->generate(*this);
  
  m_output  << "}" << tag->name() << ";" << endl << endl;

  // input operator
  m_output << "int read_" << tag->name() 
	   << "(FILE *fp, struct " << tag->name() << " *data);" 
	   << endl;

  // output operator
  m_output << "int write_" << tag->name() 
	   << "(FILE *fp, struct " << tag->name() << " *data);" 
	   << endl;

  m_output << "void free_" << tag->name()
	   << "(" << tag->name() << " *);" << endl << endl;

  m_output << "#define create_" << tag->name() 
	   << "s(n) (" << tag->name() 
	   << "* )malloc((n) * sizeof(" << tag->name() 
	   << "))" << endl << endl;
}

/*
 * CInOut
 */
CInOut::CInOut(ostream& os)
  : m_output(os)
{
}

CInOut::~CInOut()
{
}

// generate code for a binary expression
void CInOut::generate(BinaryExpression *expr)
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
void CInOut::generate(Identifier *ident)
{
  m_output << "data->" << ident->name();
}

// generate code for simple integer number
void CInOut::generate(Number *number)
{
  m_output << number->value();
}


/*
 * CPPInput
 */ 

CInput::CInput(const string& basename, ostream& os)
  : CInOut(os)
{
  // write prologue for implementation
  m_output << endl << "#include \"atrdmplib.h\"" << endl;
  m_output << endl << "#include \"" << basename << ".h\"" << endl << endl;
}

CInput::~CInput()
{
}

// generate code to read from input stream
void CInput::generate(Field *field)
{
  if(field->indices()->size() > 0) {

    // todo: only for one dimension at the moment...
    Expression *expr = *field->indices()->begin();

    m_output << "    { int i;" << endl;

    if(!expr->is_constant()) {
      m_output << "    data->" << field->name() 
	       << " = create_" << field->type() 
	       << "s(";
      expr->generate(*this);
      m_output << ");" << endl;
    }
    
    m_output << "    for(i = 0; i < ";
    expr->generate(*this);
    m_output << "; i++) {" << endl;
    m_output << "        read_" << field->type() 
	     << "(fp, &data->" << field->name() << "[i]);" << endl;
    m_output << "    }}" << endl; 
    
  } else {
    m_output << "    read_" << field->type()
	     << "(fp, &data->" << field->name() << ");" << endl;
  }
}

// generate code to read structure from input stream
void CInput::generate(Struct *struc)
{
  
  m_output << "int read_" << struc->name() 
	   << "(FILE *fp, struct " << struc->name() << " *data)" << endl
	   << "{" << endl;
  
  struc->fields()->generate(*this); 

  m_output << "    return 1;" << endl;
  m_output << "}" << endl << endl;
}

// generate code to read 'tag' from input stream
void CInput::generate(Tag *tag)
{
  m_output << "int read_" << tag->name()
	   << "(FILE *fp, struct " << tag->name() << " *data)" << endl
	   << "{ int _res; " << endl;

  m_output << "    memset(data, 0, sizeof(*data));" << endl;

  m_output << "    if((_res = check_start_tag(fp,\"" 
	   << tag->name() << "\")) != ERR_SUCCESS) return _res;" << endl;
  
  tag->fields()->generate(*this);  
  
  m_output << "    return check_end_tag(fp,\""
	   << tag->name() << "\");" << endl;
  
  m_output << "}" << endl << endl;
}



COutput::COutput(ostream& os)
  : CInOut(os)
{
}

COutput::~COutput()
{
}

// generate code to write to output stream
void COutput::generate(Field *field)
{
  if(field->indices()->size() > 0) {

    // todo: only for one dimension at the moment...
    Expression *expr = *field->indices()->begin();

    m_output << "    { int i;" << endl;
    m_output << "    for(i = 0; i < ";
    expr->generate(*this);
    m_output << "; i++) {" << endl;
    m_output << "        write_" << field->type() 
	     << "(fp, &data->" << field->name() << "[i]);" << endl;
    m_output << "    } fprintf(fp,\"\\n\"); }" << endl;
    
  } else {
    m_output << "    write_" << field->type()
	     << "(fp, &data->" << field->name() << ");" << endl;
  }
}



// generate code to write structure to output stream
void COutput::generate(Struct *struc)
{
  m_output << "int write_" << struc->name() << "(FILE *fp, struct " 
	   << struc->name() << " *data)"
	   << endl << "{" << endl;
  
  struc->fields()->generate(*this); 

  m_output << "    return 1;" << endl;
  m_output << "}" << endl << endl;
}

// generate code to write 'tag' to output stream
void COutput::generate(Tag *tag)
{
  // start of function
  m_output << "int write_" << tag->name() << "(FILE *fp, struct " 
	   << tag->name() << " *data)" 
	   << endl << "{" << endl;

  // output start tag
  m_output << "    fprintf(fp,\" \\n { " << tag->name() << "\\n\");" << endl;

  // output fields
  tag->fields()->generate(*this); 

  // output end tag
  m_output << "    fprintf(fp,\"\\n } " << tag->name() << "\\n\");" << endl;

  // end of function
  m_output << "    return 1;" << endl;
  m_output << "}" << endl << endl;
}

CFree::CFree(ostream &os)
  : CInOut(os)
{
}

CFree::~CFree()
{
}

void CFree::generate(Struct *struc)
{
  m_output << "void free_" << struc->name()
	   << "(" << struc->name() << " *data)" << endl;

  m_output << "{" << endl;

  struc->fields()->generate(*this);

  m_output << "}" << endl << endl;
}

void CFree::generate(Tag *tag)
{

  m_output << "void free_" << tag->name()
	   << "(" << tag->name() << " *data)" << endl;

  m_output << "{" << endl;
  
  m_output << "     if(data == 0) return;" << endl;

  tag->fields()->generate(*this);

  m_output << "}" << endl << endl;
}

void CFree::generate(Field *field)
{

  if((field->type() == "int") ||
     (field->type() == "float") ||
     (field->type() == "double") ||
     (field->type() == "short") ||
     (field->type() == "long")) {
    if(field->indices()->size() > 0) {
      Expression *expr = *field->indices()->begin();
      if(!expr->is_constant()) {
	m_output << "        free_" << field->type() << "(data->" 
		 << field->name() << ");" << endl;
      }
    }
    return;
  }
  
  if(field->indices()->size() > 0) {

    // todo: only for one dimension at the moment...
    Expression *expr = *field->indices()->begin();

    m_output << "    { int i;" << endl;
    
    m_output << "    for(i = 0; i < ";
    expr->generate(*this);
    m_output << "; i++) {" << endl;
    m_output << "        free_" << field->type() 
	     << "(&data->" << field->name() << "[i]);" << endl;
    m_output << "    }}" << endl; 
    
    if(!expr->is_constant()) {
      m_output << "    free(data->" << field->name() << ");" << endl;
    }

  } else {
    m_output << "      free_" << field->type()
	     << "(&data->" << field->name() << ");" << endl;
  }  
}
