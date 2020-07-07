/*standart input-output*/
//commands that start with "#" character are preprocessor commands
#include <stdio.h>
#include <stdlib.h>

//if we are compiling on windows, compile these functions
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

//made up readline funnction
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

//fake add_history
void add_history(char* unused) {}

//otherwise include the editline headers
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

//parser adjective
/*mpc_parser_t* Adjective = mpc_or(4, //list of adjectives
    mpc_sym("wow"), mpc_sym("many"),
    mpc_sym("so"), mpc_sym("such")
);*/

//build a parser 'noun' to recognize things
/*mpc_parser_t* Noun = mpc_or(5, //list of nouns
mpc_sym("lisp"), mpc_sym("language"),
mpc_sym("book"), mpc_sym("build"),
mpc_sym("c")
);*/

/*phrase parser needs to reference the precious
parsers in order to use */
/*mpc_parser_t* Phrase = mpc_and(2,
mpcf_strfold, Adjective, Noun, free);*/

/*specifying if zero or more parsers are required:
by creating a parser that looks for zero or more occurences
of another parser, this makes the parser is able to accept an
infinte amount of arguments*/
//mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);

//same algorithm in another structure
//name rules
mpc_parser_t* Adjective = mpc_new("adjective");
mpc_parser_t* Noun = mpc_new("noun");
mpc_parser_t* Sum = mpc_new("sum");
mpc_parser_t* Digit = mpc_new("digit");
mpc_parser_t* Number = mpc_new("number");
mpc_parser_t* Phrase = mpc_new("phrase");
mpc_parser_t* Doge = mpc_new("doge");

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
      "                                           \
        adjective : \"wow\" | \"many\"            \
                  | \"so\" | \"such\";            \
        noun      : \"lisp\" | \"language\"       \
                  | \"book\" | \"build\" | \"c\"; \
        digit     : \"0\" | \"1\" | \"2\" | \"3\" \
                  | \"4\" | \"5\" | \"6\" | \"7\" \
                  | \"8\" | \"9\";                \
        number    : <digit>;                      \
        sum       : \"+\" <number> <number>;      \                  \
        phrase    : <adjective> <noun>;           \
        doge      : <phrase>*;                    \
      ",
Adjective, Noun, Phrase, Doge);

mpc_cleanup(4, Adjective, Noun, Phrase, Doge);

sint main(int argc, char** argv) {

    while (1) {

        //Output our prompt (hey) and get input
        char* input = readline(">>");

        //Adds input to historys
        add_history(input);

        //Prints input back to user
        printf("%s\n", input);

        //frees the retrieved data stored in the input variable
        free(input);

    }


    return 0;
}
