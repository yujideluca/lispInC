/*standart input-output*/
//commands that start with "#" character are preprocessor commands
#include <stdio.h>
#include <stdlib.h>
 /*quotation marks searches the curent directory first, while angular
   brackets searches the system locations first*/
#include "mpc.h"
//if we are compiling on windows, compile these functions
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

//made up readline funnction.
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}
/*quotation marks searches the curent directory first, while angular
  brackets searches the system locations first*/
//fake add_history
void add_history(char* unused) {}

//otherwise include the editline headers
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

/*enum is a user defined data type in C used to assign names
  to integral constants (in this case, we use it in order to
  tell apart 0 = the structure is a number and 1 = the
  structure is an error)*/
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM}; //possible error types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR }; //possible lval types
/*Syntax expressions (sexpr) are variable lenght lists of other values
 crated by pointers, once structs are not able  to generate variable-size
data.*/

//declaring new lval (lval = l + val = Lisp VALue) struct.
typedef struct lval {
  int type; //represented by a single integer value
  long num;
  int err; //represented by a single integer value
  int sym;
  int count;
} lval;


//Constructs a pointer to a number in our language
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

//Constructs a pointer to an error in our language
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  /*C strings are null terminated (end with \0)
   strlen excludes the null terminator, so for making
  a proper memory allcation for the error string, we
  need to add the space for the null terminator*/
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m)
  return v;
}

//Constructs a pointer to an errora symbol in our language
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->err = malloc(strlen(s) + 1);
  strcpy(v->err, s)
    return v;
}

 //Constructs a pointer to a new empty Sexpr lval
 lval* lval_sexpr(void) {
   lval* v = malloc(sizeof(lval));
   v->type = LVAL_SEXPR;
   v->count = 0;
   v->cell = NULL; //NULL points to memory location 0 (signifies empty data)
   return v;
 }

void lval_print(lval v) {
  switch (v.type) {
      //If it is a number, print it and "break free" from the switch.
      case LVAL_NUM:
        printf("%li", v.num); break;
      //if the type is an error.
      case LVAL_ERR:
        if (v.err == LERR_DIV_ZERO) {
          printf("Error: Division by zero!");
        }
        if (v.err == LERR_BAD_OP) {
          printf("Error: Invalid operator!");
        }
        if (v.err == LERR_BAD_NUM) {
          printf("Error: Invalid Number!");
        }
      break;
  }
}

//Print an "lval" + a newline
void lval_println(lval v) { lval_print(v); putchar('\n'); }

/*atoi - converts a char* to a int
  strcmp - takes as input two char* and if they are equal, returns 0
  strstr takes as input two char* and returns a pointer to the locaiton
  of the second in the first, or 0 if the second is not a sub-string
  of the first*/


lval eval_op(lval x, char* op, lval y) {
  //If either value is an error, return it.
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  //Otherwise, interact the data.
  if (strcmp(op, "+") == 0) {return lval_num(x.num + y.num);}
  if (strcmp(op, "-") == 0) {return lval_num(x.num - y.num);}
  if (strcmp(op, "*") == 0) {return lval_num(x.num * y.num);}
  if (strcmp(op, "/") == 0) {
    //if the y operand (the divisor) is 0, return division by 0 error
    return y.num == 0
      /*this is a temary operator's structure:
        condition ? value_if_true : value_if_false*/
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }
  return lval_err(LERR_BAD_OP);
}


lval eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    //check if there is some error in conversion
    errno = 0;
    /*long int strtol(const char *str, char **endptr, int base)
     converts a string into a long integer value according to the base*/
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }
  //Variable definitions for the loop
  char* op = t->children[1]->contents; 
  lval x = eval(t->children[2]);
  int i = 3;
  //Loops through the tree evaluating the operations and getting the results.
  while (strstr(t->children[i]->tag, "expr")) {
    // x is the computed data (either result or division error) from eval_op.
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

//free the memory acquired from malloc by freeing the pointer itself
void lval_del(lval* v) {
  //inspect the type of lval and release any memory pointed to by its fields
  switch (v->type) {
    case LVAL_NUM: break;
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym)break;
    case LVAL_SEXPR: {
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }
  }
  free(v);
}

//same algorithm in another structure
//name rules
int main(int argc, char** argv) {
  mpc_parser_t* Symbol    = mpc_new("symbol");
  mpc_parser_t* Number    = mpc_new("number");
  mpc_parser_t* Sexpr     = mpc_new("sexpr");
  mpc_parser_t* Expr      = mpc_new("expr");
  mpc_parser_t* Lispy     = mpc_new("lispy");

  //definig rules named before
  mpca_lang(MPCA_LANG_DEFAULT,
      /*name of the rule: definition;
        "ab"	    The string ab is required.
        'a'	        The character a is required.
        'a' 'b'	    First 'a' is required, then 'b' is required.
        'a' | 'b'	Either 'a' is required, or 'b' is required.
        'a'*	    Zero or more 'a' are required.
        'a'+	    One or more 'a' are required.
        <abba>	    The rule called abba is required.*/
      "                                                       \
        number    : /-?[0-9]+/;                               \
        operator  : '+' |'-' |'*' |'/';                       \
        expr      : <number> | '('<operator> <expr>+ ')' ;    \
        lispy     :  /^/<operator> <expr>+ /$/ ;              \
      ",
      Number, Operator, Sexpr, Expr, Lispy);

  while (1) {

    char* input = readline(">> ");
    add_history(input);

    mpc_result_t r;
    /*attempt to Padrse the user input calls the mpc_parse function with
      the Lispy parser and the input string, then copies the parse into r
      and returns 1 on success or 0 on failiure*/
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      //on success print the AST
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else{
      //otherwise, print the error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);

  }

    //undefine and delete parsers
  mpc_cleanup(4, Number, Symbol, Sexpr, Expr, Lispy);
    return 0;

}
