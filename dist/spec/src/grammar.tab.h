typedef union {
  Declaration   *node;
  FieldList     *fieldList;
  Field         *field;
  char          *str;
  Expression    *expr;
  int           ivalue;
  vector<Expression*> *indices;
} YYSTYPE;
#define	IDENTIFIER	257
#define	NUMBER	258
#define	STRUCT	259
#define	TAG	260


extern YYSTYPE yylval;
