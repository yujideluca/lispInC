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

//declaring new lval struct.
typedef struct {
  int type; //represented by a single integer value
  long num;
  int err; //represented by a single integer value
} lval;
/*enum is a user defined data type in C used to assign names
  to integral constants (in this case, we use it in order to
  tell apart 0 = the structure is a number and 1 = the
  structure is an error)*/
enum { LVAL_NUM, LVAL_ERR};

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

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  if (t->children_num >= 1) {
    int total = 1;
    /*This loop applies this fuction recursively, making each call
      analyse a part of the origial tree as a tree itself. When
      there is only one children in the function, then its result
      will be added to te total in the loop that recursivelly called
      it untill all the sums sulve and the fisrt ayer of function solves
      and the code resumes.*/
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
  return 0;
}

/*atoi - converts a char* to a int
  strcmp - takes as input two char* and if they are equal, returns 0
  strstr takes as input two char* and returns a pointer to the locaiton
  of the second in the first, or 0 if the second is not a sub-string
  of the first*/

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) {return x + y;}
  if (strcmp(op, "-") == 0) {return x - y;}
  if (strcmp(op, "*") == 0) {return x * y;}
  if (strcmp(op, "/") == 0) {return x / y;}
  return 0;
}
long eval(mpc_ast_t* t) {
  //if it is tagged as number, just return it directly
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }
  //the operator is always second child
  char* op = t->children[1]->contents;
  
  long x = eval(t->children[2]); //third child = x

  //passas through the rest of the children
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}



//same algorithm in another structure
//name rules
int main(int argc, char** argv) {
  mpc_parser_t* Operator  = mpc_new("operator");
  mpc_parser_t* Number    = mpc_new("number");
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
      Number, Operator, Expr, Lispy);

  while (1) {

    char* input = readline(">> ");
    add_history(input);

    mpc_result_t r;
    /*attempt to Padrse the user input calls the mpc_parse function with
      the Lispy parser and the input string, then copies the parse into r
      and returns 1 on success or 0 on failiure*/
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      
      //on success print the AST
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else{
      //otherwise, print the error
      mpc_err_print(r.output);
      mpc_err_delete(r.output);
    }

    //prints the tree structure
    
    //// Abstract Syntax Tree (AST) from output.
    //mpc_ast_t* a = r.output;
    //printf("Tag: %s\n", a->tag);
    //printf("Contents: %s\n", a->contents);
    //printf("Number of children: %i\n", a->children_num);

    //// Get first child.
    //mpc_ast_t* c0 = a->children[0];
    //printf("First Child Tag: %s\n", c0->tag);
    //printf("First Child Contents: %s\n", c0->contents);
    //printf("First Child Number of Chidren: %i\n", c0->children_num);

    free(input);
  }
    //undefine and delete parsers
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
