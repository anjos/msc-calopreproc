
#include <stdlib.h>
#include <iostream.h>

#include "spec.h"
#include "cppgen.h"
#include "cgen.h"

// our scanner keeps track of line numbers
extern int yylineno;

// scanner calls yywrap() with C linkage
extern "C" {
  int yywrap() {
    return 1;
  }
};

// print syntax error message
int yyerror(const char *msg)
{
  cerr << msg << " in line " << yylineno << endl;
}

// declaration for parser
int yyparse();

// parser stores its result in this global variable
extern deque<Declaration*> *definitions;

// generate C++ code
static void gen_cpp(const string& basename)
{
  // build file names
  string header_name = basename + ".h";
  string impl_name   = basename + ".cc";

  // open files
  ofstream header(header_name.data());
  ofstream impl(impl_name.data());

  // create code generators
  CPPDeclarations decl(basename, header);
  CPPInput        input(basename, impl);
  CPPOutput       output(impl);

  // for each definition in the input file...
  for(deque<Declaration*>::iterator it = definitions->begin();
      it < definitions->end();
      it++) {
    (*it)->generate(decl);
    (*it)->generate(input);
    (*it)->generate(output);
  }  
}

// generate C code
static void gen_c(const string& basename)
{
  // build file name
  string header_name = basename + ".h";
  string impl_name   = basename + ".c";

  // open output files
  ofstream header(header_name.data());
  ofstream impl(impl_name.data());

  // create code generators
  CDeclarations decl(basename, header);
  CInput        input(basename, impl);
  COutput       output(impl);
  CFree         free(impl);

  // for each definition in the input file
  for(deque<Declaration*>::iterator it = definitions->begin();
      it < definitions->end();
      it++) {
    (*it)->generate(decl);
    (*it)->generate(input);
    (*it)->generate(output);
    (*it)->generate(free);
  }
}

int main(int argc, char *argv[])
{
  string lang = "cc";		// default is C++
  string base = "out";		// default name is 'out'

  // check if language argument given
  if(argc > 2)
    lang = argv[2];

  // check if basename argument given
  if(argc > 1)
    base = argv[1];
    
  // parse input (always stdin)
  if(yyparse() == 0) {

    // if successful, generate code
    if(lang == "cc") {
      gen_cpp(base);
    } else if (lang == "c") {
      gen_c(base);
    } else {
      cerr << "invalid output language: " << lang << endl;
      return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);

  } else {
    return (EXIT_FAILURE);
  }
}
