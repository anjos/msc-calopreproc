typedef union {
  Declaration   *node;
  FieldList     *fieldList;
  Field         *field;
  char          *str;
  Expression    *expr;
  int           ivalue;
  vector<Expression*> *indices;
} YYSTYPE;
#define	IDENTIFIER	258
#define	NUMBER	259
#define	STRUCT	260
#define	TAG	261


extern YYSTYPE yylval;
