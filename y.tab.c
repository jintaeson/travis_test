/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20140422

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 22 "./parse.y"
#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#include "filecntl.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#include <stdio.h>
#include "chartypes.h"
#include <signal.h>

#include "memalloc.h"

#include "bashintl.h"

#define NEED_STRFTIME_DECL	/* used in externs.h */

#include "shell.h"
#include "trap.h"
#include "flags.h"
#include "parser.h"
#include "mailcheck.h"
#include "test.h"
#include "builtins.h"
#include "builtins/common.h"
#include "builtins/builtext.h"

#include "shmbutil.h"

#if defined (READLINE)
#  include "bashline.h"
#  include <readline/readline.h>
#endif /* READLINE */

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif /* HISTORY */

#if defined (JOB_CONTROL)
#  include "jobs.h"
#endif /* JOB_CONTROL */

#if defined (ALIAS)
#  include "alias.h"
#else
typedef void *alias_t;
#endif /* ALIAS */

#if defined (PROMPT_STRING_DECODE)
#  ifndef _MINIX
#    include <sys/param.h>
#  endif
#  include <time.h>
#  if defined (TM_IN_SYS_TIME)
#    include <sys/types.h>
#    include <sys/time.h>
#  endif /* TM_IN_SYS_TIME */
#  include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#ifdef DEBUG
#  define YYDEBUG 1
#else
#  define YYDEBUG 0
#endif

#if defined (HANDLE_MULTIBYTE)
#  define last_shell_getc_is_singlebyte \
	((shell_input_line_index > 1) \
		? shell_input_line_property[shell_input_line_index - 1] \
		: 1)
#  define MBTEST(x)	((x) && last_shell_getc_is_singlebyte)
#else
#  define last_shell_getc_is_singlebyte	1
#  define MBTEST(x)	((x))
#endif

#if defined (EXTENDED_GLOB)
extern int extended_glob;
#endif

extern int eof_encountered;
extern int no_line_editing, running_under_emacs;
extern int current_command_number;
extern int sourcelevel;
extern int posixly_correct;
extern int last_command_exit_value;
extern char *shell_name, *current_host_name;
extern char *dist_version;
extern int patch_level;
extern int dump_translatable_strings, dump_po_strings;
extern sh_builtin_func_t *last_shell_builtin, *this_shell_builtin;
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

extern int errno;
/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

#ifdef DEBUG
static void debug_parser __P((int));
#endif

static int yy_getc __P((void));
static int yy_ungetc __P((int));

#if defined (READLINE)
static int yy_readline_get __P((void));
static int yy_readline_unget __P((int));
#endif

static int yy_string_get __P((void));
static int yy_string_unget __P((int));
static int yy_stream_get __P((void));
static int yy_stream_unget __P((int));

static int shell_getc __P((int));
static void shell_ungetc __P((int));
static void discard_until __P((int));

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
static void push_string __P((char *, int, alias_t *));
static void pop_string __P((void));
static void free_string_list __P((void));
#endif

static char *read_a_line __P((int));

static int reserved_word_acceptable __P((int));
static int yylex __P((void));

static void push_heredoc __P((REDIRECT *));
static char *mk_alexpansion __P((char *));
static int alias_expand_token __P((char *));
static int time_command_acceptable __P((void));
static int special_case_tokens __P((char *));
static int read_token __P((int));
static char *parse_matched_pair __P((int, int, int, int *, int));
#if defined (ARRAY_VARS)
static char *parse_compound_assignment __P((int *));
#endif
#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
static int parse_dparen __P((int));
static int parse_arith_cmd __P((char **, int));
#endif
#if defined (COND_COMMAND)
static void cond_error __P((void));
static COND_COM *cond_expr __P((void));
static COND_COM *cond_or __P((void));
static COND_COM *cond_and __P((void));
static COND_COM *cond_term __P((void));
static int cond_skip_newlines __P((void));
static COMMAND *parse_cond_command __P((void));
#endif
#if defined (ARRAY_VARS)
static int token_is_assignment __P((char *, int));
static int token_is_ident __P((char *, int));
#endif
static int read_token_word __P((int));
static void discard_parser_constructs __P((int));

static char *error_token_from_token __P((int));
static char *error_token_from_text __P((void));
static void print_offending_line __P((void));
static void report_syntax_error __P((char *));

static void handle_eof_input_unit __P((void));
static void prompt_again __P((void));
#if 0
static void reset_readline_prompt __P((void));
#endif
static void print_prompt __P((void));

#if defined (HANDLE_MULTIBYTE)
static void set_line_mbstate __P((void));
static char *shell_input_line_property = NULL;
#else
#  define set_line_mbstate()
#endif

extern int yyerror __P((const char *));

#ifdef DEBUG
extern int yydebug;
#endif

/* Default prompt strings */
char *primary_prompt = PPROMPT;
char *secondary_prompt = SPROMPT;

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* Non-zero means we expand aliases in commands. */
int expand_aliases = 0;

/* If non-zero, the decoded prompt string undergoes parameter and
   variable substitution, command substitution, arithmetic substitution,
   string expansion, process substitution, and quote removal in
   decode_prompt_string. */
int promptvars = 1;

/* If non-zero, $'...' and $"..." are expanded when they appear within
   a ${...} expansion, even when the expansion appears within double
   quotes. */
int extended_quote = 1;

/* The decoded prompt string.  Used if READLINE is not defined or if
   editing is turned off.  Analogous to current_readline_prompt. */
static char *current_decoded_prompt;

/* The number of lines read from input while creating the current command. */
int current_command_line_count;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
#define HEREDOC_MAX 16

static REDIRECT *redir_stack[HEREDOC_MAX];
int need_here_doc;

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
static char *shell_input_line = (char *)NULL;
static int shell_input_line_index;
static int shell_input_line_size;	/* Amount allocated for shell_input_line. */
static int shell_input_line_len;	/* strlen (shell_input_line) */

/* Either zero or EOF. */
static int shell_input_line_terminator;

/* The line number in a script on which a function definition starts. */
static int function_dstart;

/* The line number in a script on which a function body starts. */
static int function_bstart;

/* The line number in a script at which an arithmetic for command starts. */
static int arith_for_lineno;

/* The line number in a script where the word in a `case WORD', `select WORD'
   or `for WORD' begins.  This is a nested command maximum, since the array
   index is decremented after a case, select, or for command is parsed. */
#define MAX_CASE_NEST	128
static int word_lineno[MAX_CASE_NEST+1];
static int word_top = -1;

/* If non-zero, it is the token that we want read_token to return
   regardless of what text is (or isn't) present to be read.  This
   is reset by read_token.  If token_to_read == WORD or
   ASSIGNMENT_WORD, yylval.word should be set to word_desc_to_read. */
static int token_to_read;
static WORD_DESC *word_desc_to_read;

static REDIRECTEE redir;
#line 300 "./parse.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 315 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define IF 257
#define THEN 258
#define ELSE 259
#define ELIF 260
#define FI 261
#define CASE 262
#define ESAC 263
#define FOR 264
#define SELECT 265
#define WHILE 266
#define UNTIL 267
#define DO 268
#define DONE 269
#define FUNCTION 270
#define COND_START 271
#define COND_END 272
#define COND_ERROR 273
#define IN 274
#define BANG 275
#define TIME 276
#define TIMEOPT 277
#define WORD 278
#define ASSIGNMENT_WORD 279
#define NUMBER 280
#define ARITH_CMD 281
#define ARITH_FOR_EXPRS 282
#define COND_CMD 283
#define AND_AND 284
#define OR_OR 285
#define GREATER_GREATER 286
#define LESS_LESS 287
#define LESS_AND 288
#define LESS_LESS_LESS 289
#define GREATER_AND 290
#define SEMI_SEMI 291
#define LESS_LESS_MINUS 292
#define AND_GREATER 293
#define LESS_GREATER 294
#define GREATER_BAR 295
#define yacc_EOF 296
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    0,    0,   27,   27,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   24,   24,   24,   24,   24,
   24,   24,   24,   24,   24,   26,   26,   26,   25,   25,
   10,   10,    1,    1,    1,    1,   11,   11,   11,   11,
   11,   11,   11,   11,   11,   11,   11,   12,   12,   12,
   12,   12,   12,   12,   12,   18,   18,   18,   18,   13,
   13,   13,   13,   13,   13,   14,   14,   14,   19,   19,
   19,   20,   20,   23,   21,   21,   21,   15,   16,   17,
   22,   22,   22,   31,   31,   29,   29,   29,   29,   30,
   30,   28,   28,    4,    7,    7,    5,    5,    5,    6,
    6,    6,    6,    6,    6,   34,   34,   33,   33,   33,
   35,   35,    8,    8,    8,    9,    9,    9,    9,    9,
    3,    3,    3,    3,    3,    3,    2,    2,   32,   32,
};
static const YYINT yylen[] = {                            2,
    2,    1,    2,    1,    1,    2,    2,    2,    3,    3,
    2,    3,    2,    3,    2,    3,    2,    3,    2,    3,
    2,    3,    2,    3,    2,    3,    2,    3,    2,    3,
    2,    3,    2,    2,    3,    1,    1,    1,    1,    2,
    1,    2,    1,    1,    2,    1,    1,    1,    5,    5,
    1,    1,    1,    1,    1,    1,    1,    6,    6,    7,
    7,   10,   10,    9,    9,    7,    7,    5,    5,    6,
    6,    7,    7,   10,   10,    6,    7,    6,    5,    6,
    4,    1,    2,    3,    5,    7,    6,    3,    1,    3,
    4,    6,    5,    1,    2,    4,    4,    5,    5,    2,
    3,    1,    3,    2,    1,    2,    3,    3,    3,    4,
    4,    4,    4,    4,    1,    1,    1,    1,    1,    1,
    0,    2,    1,    2,    2,    4,    4,    3,    3,    1,
    1,    2,    2,    3,    3,    2,    4,    1,    1,    2,
};
static const YYINT yydefred[] = {                         0,
    0,  121,    0,    0,    0,  121,  121,    0,    0,    0,
    0,    0,   37,    0,   89,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    2,    4,    0,    0,  121,  121,
    0,  138,    0,  130,    0,    0,    0,    0,   47,   51,
   48,   54,   55,   56,   57,   46,   52,   53,   38,   41,
    0,    3,  105,    0,    0,  121,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  140,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   11,   13,   21,
   17,   29,   15,   23,   19,   27,   25,   31,   33,   34,
    7,    8,    0,    0,  121,  116,  117,    1,  121,  121,
    0,    0,   36,   42,   39,    0,    0,  119,  118,  120,
    0,  136,  121,  122,  115,  104,    0,    0,  121,    0,
  121,  121,  121,  121,    0,  121,  121,    0,    0,   90,
    0,  121,   12,   14,   22,   18,   30,   16,   24,   20,
   28,   26,   32,   35,    9,   10,   88,   84,    0,    0,
    0,    0,    0,   40,    0,    0,  121,  121,  121,  121,
  121,  121,    0,  121,    0,  121,    0,    0,    0,    0,
  121,    0,  121,    0,    0,  121,    0,   81,    0,    0,
  126,  127,    0,    0,  121,  121,   85,    0,    0,    0,
    0,    0,    0,    0,  121,    0,    0,  121,  121,    0,
    5,    0,  121,    0,   68,   69,  121,  121,  121,  121,
    0,    0,    0,    0,   49,   50,    0,    0,   79,    0,
    0,   87,  110,  111,    0,    0,    0,  100,    0,    0,
   78,   76,  102,    0,    0,    0,    0,   58,    6,  121,
    0,   59,    0,    0,    0,    0,   70,    0,  121,   71,
   80,   86,  121,  121,  121,  121,  101,   77,    0,    0,
  121,   60,   61,    0,  121,  121,   66,   67,   72,   73,
    0,    0,    0,    0,    0,  121,  103,   96,    0,  121,
  121,    0,    0,  121,  121,  121,   93,   98,    0,    0,
    0,   64,   65,    0,    0,   92,   62,   63,   74,   75,
};
static const YYINT yydgoto[] = {                         31,
   32,   33,  115,   53,  116,  117,   54,   35,  152,   37,
   38,   39,   40,   41,   42,   43,   44,   45,   46,  178,
   47,  188,   48,   49,  106,   50,  202,  235,  194,  195,
  196,   51,  112,   98,   55,
};
static const YYINT yysindex[] = {                        31,
   14,    0, -252, -257, -243,    0,    0, -234, -232,  954,
 -221,   21,    0,  126,    0, -215, -211,  -43, -209,  -35,
 -206, -205, -204, -202,    0,    0, -199, -198,    0,    0,
    0,    0,  -47,    0,    3,   99, 1018, 1029,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   71,    0,    0, -176,  597,    0,   24,   -5,   25, -182,
 -181,   48, -172,  -47,  993,    0,   49, -177, -174,  -34,
 -173,   94, -171, -170, -169, -168, -162,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -23,   82,    0,    0,    0,    0,    0,    0,
  908,  908,    0,    0,    0, 1029,  993,    0,    0,    0,
  -47,    0,    0,    0,    0,    0,   60,   -9,    0,   -1,
    0,    0,    0,    0,   79,    0,    0,   84,  217,    0,
  -47,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  664,  597,
  597, -242, -242,    0,  -47, -222,    0,    0,    0,    0,
    0,    0,   -6,    0,   35,    0, -143,    2,   75,   86,
    0, -150,    0, -135, -134,    0, 1029,    0,  217,  -47,
    0,    0,  908,  908,    0,    0,    0, -125,  597,  597,
  597,  597,  597, -153,    0, -121,   -7,    0,    0, -126,
    0,   36,    0,   19,    0,    0,    0,    0,    0,    0,
 -124,  597,   36,   22,    0,    0,  217, 1029,    0, -113,
 -109,    0,    0,    0, -237, -237, -237,    0, -141,   52,
    0,    0,    0, -123,  -25, -117,   28,    0,    0,    0,
   87,    0, -112,   34, -106,   39,    0,   60,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -21, -122,
    0,    0,    0,  102,    0,    0,    0,    0,    0,    0,
  103, -201,  597,  597,  597,    0,    0,    0,  597,    0,
    0, -104,   41,    0,    0,    0,    0,    0,  597, -102,
   43,    0,    0, -100,   51,    0,    0,    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  111,  -10,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  470,    0,    0,    4,  165,  179,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  119,    0,  119,    0,
    0,  312,    0,  489,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    9,   12,    0,    0,    0,  370,    0,    0,    0,    0,
  509,    0,    0,    0,    0,    0,  328,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  523,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   -4,   -2,    0,  543,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  417,    0,    0,  562,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  749,  783,  824, -103,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  431,    0,    0,
    0,    0,    0,    0,  704,  807,  841,    0,  -89,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -84,    0,    0,    0,    0,    0,    0, -251,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -231,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
    0,   13,  514, -144,    0,  331,  864,    0,   30,    0,
 -111,    0,    0,    0,    0,    0,    0,    0,    0, -164,
    0,  -94,    0,  -31,    5,  142,   11,  -50,   -3,    0,
    0,  175,  -41,    0, 1233,
};
#define YYTABLESIZE 1518
static const YYINT yytable[] = {                         36,
  114,   82,  114,  114,  109,  128,  105,  129,  114,   86,
  137,   97,   96,  123,  219,  261,  123,  177,  124,  276,
   57,  125,   64,   52,   58,   56,  211,   36,  214,   36,
   36,   99,  234,  128,   59,  129,  185,  186,  187,   97,
   25,   99,  100,   62,  109,  109,  157,  158,   36,   36,
   63,   36,  251,  108,  128,   66,  129,  286,  186,   99,
   67,  114,   78,  111,  245,  246,   79,  177,   83,  161,
   30,   87,   88,   89,  154,   90,   95,  131,   91,   92,
  109,  113,  119,  124,  114,  126,  127,  128,  114,  132,
   28,  234,   27,  108,  108,  114,  114,  159,  260,  130,
  133,  147,  260,  134,  138,  177,  142,  143,  144,  145,
   30,  114,  114,   36,   36,  146,  199,  122,  160,  155,
  139,  166,  148,  203,  176,  205,  206,  201,  121,  108,
   28,  153,   27,  215,  216,  222,  101,  228,  141,  294,
  295,  231,  238,  242,  247,  105,  250,  252,  253,  257,
  139,  262,  263,   29,  233,  277,  267,  102,  268,   94,
  240,  180,  269,  270,  292,  293,  297,  298,  299,  139,
  139,  249,  139,   95,   43,  300,   91,  287,  104,  181,
  182,  218,  213,  259,   65,   77,  154,   76,   44,    0,
    0,  229,    0,   29,    0,    0,    0,  208,    0,    0,
    0,  173,   43,    0,    0,   43,    0,    0,  210,  266,
    0,    0,    0,  153,    0,    0,   44,    0,    0,   44,
    0,    0,    0,   43,  281,  285,  114,    0,    0,    0,
    0,    0,    0,  139,   80,    0,   81,   44,    0,    0,
    0,  121,   84,  135,   85,  136,    0,   36,   36,   36,
   36,    0,   36,    0,    0,  232,   30,   36,   36,    0,
    0,  198,  121,    0,  162,    0,  164,   36,   36,   36,
  233,    0,  165,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,   36,    1,    2,   43,   43,
  110,  128,    3,  129,    4,    5,    6,    7,   97,  123,
    8,    9,   44,   44,  124,   10,   11,  125,   12,   13,
   14,   15,  201,  239,  258,    0,   16,   17,   18,   19,
   20,  121,   21,   22,   23,   24,   26,    2,    0,  233,
  110,  110,    3,    0,    4,    5,    6,    7,    0,   29,
    8,    9,  207,  157,  158,  107,  171,    0,   12,   13,
   14,   15,  172,  209,  265,    0,   16,   17,   18,   19,
   20,    0,   21,   22,   23,   24,  110,  139,  106,  280,
  284,  139,  139,  140,  139,  139,  139,  139,    0,   45,
  139,  139,   99,  100,    0,  139,  121,    0,  139,  139,
  139,  139,  121,    0,    0,    0,  139,  139,  139,  139,
  139,    0,  139,  139,  139,  139,  139,   45,    0,    0,
   45,   68,   69,   70,   71,   72,    0,   73,    0,   74,
   75,    0,   43,   43,   43,   43,   82,   43,   45,    0,
    0,    0,   43,   43,  121,    0,   44,   44,   44,   44,
   83,   44,    0,    0,    0,    0,   44,   44,   43,   43,
    0,    0,  106,    0,   82,   43,    0,   82,    0,    0,
   43,    0,   44,   44,    0,    0,    0,    0,   83,   44,
    0,   83,    0,    2,   44,   82,    0,    0,    3,  131,
    4,    5,    6,    7,    0,    0,    0,    9,    0,   83,
    0,    0,    0,   45,   45,    0,    0,   15,  132,    0,
    0,    0,    0,    0,    0,    0,    0,  131,    0,    0,
  131,    0,    0,   34,    0,    0,    0,    0,  133,  223,
  224,  225,  226,  227,    0,    0,  132,    0,  131,  132,
    0,    0,  135,    0,    0,    0,    0,    0,    0,    0,
   82,   82,  248,    0,    0,    0,  133,  132,    0,  133,
    0,    0,  134,    0,   83,   83,    0,    0,    0,    0,
  135,    0,    0,  135,    0,    0,    0,  133,  121,    0,
    0,  137,    0,  121,    0,  121,  121,  121,  121,    0,
  134,  135,  121,  134,    0,  106,  106,  106,  106,    0,
  106,    0,  121,    0,  131,  106,  106,    0,    0,  137,
    0,  134,  137,  225,  226,  227,  114,    0,    0,    0,
    0,    0,    0,  132,   34,   34,    0,    0,  106,    0,
  137,    0,    0,    0,    0,    0,    0,   45,   45,   45,
   45,    0,   45,  133,    0,    0,   30,   45,   45,    0,
    0,    0,    0,    0,    0,    0,    0,  135,    0,    0,
    0,    0,    0,   45,   45,    0,   28,    0,   27,    0,
   45,    0,    0,   34,   34,   45,    0,  134,    0,    0,
    0,    0,    0,  114,   82,   82,   82,   82,    0,   82,
    0,    0,    0,    0,   82,   82,  137,    0,   83,   83,
   83,   83,    0,   83,    0,    0,   34,   34,   83,   83,
   82,   82,    0,   30,    0,    0,    0,   82,    0,    0,
    0,    0,   82,  112,   83,   83,    0,    0,    0,   29,
    0,   83,    0,   28,    0,   27,   83,  131,  131,  131,
  131,    0,  131,    0,    0,    0,    0,  131,  131,    0,
    0,  112,    0,    0,  112,    0,  132,  132,  132,  132,
    0,  132,    0,  131,  131,    0,  132,  132,    0,    0,
  131,    0,  112,    0,    0,  131,  133,  133,  133,  133,
    0,  133,  132,  132,    0,    0,  133,  133,    0,  132,
  135,  135,  135,  135,  132,  135,   29,    0,    0,  108,
  135,  135,  133,  133,    0,    0,    0,    0,    0,  133,
  134,  134,  134,  134,  133,  134,  135,  135,    0,    0,
  134,  134,    0,  135,    0,    0,  113,    0,  135,  137,
  137,  137,  137,  109,  137,    0,  134,  134,  112,  137,
  137,    0,    0,  134,    0,    0,    0,    0,  134,    0,
    0,    0,    0,    0,  113,  137,  137,  113,    0,    0,
  114,    0,  137,    2,    0,    0,    0,  137,    3,    0,
    4,    5,    6,    7,  107,  113,    8,    9,    0,   60,
   61,   10,   11,  108,   12,   13,   14,   15,  114,    0,
    0,  114,   16,   17,   18,   19,   20,    0,   21,   22,
   23,   24,   93,   94,    0,    0,    0,    0,    0,  114,
    0,    0,    0,    0,    0,    0,    0,  109,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    2,    0,    0,    0,    0,    3,    0,    4,    5,    6,
    7,  113,    0,    8,    9,    0,    0,    0,    0,    0,
    0,   12,   13,   14,   15,    0,    0,   30,  107,   16,
   17,   18,   19,   20,    0,   21,   22,   23,   24,    0,
    0,  112,  112,  112,  112,  114,  112,   28,    0,   27,
    0,  112,  112,    0,    0,    0,  156,    0,    0,    0,
    0,    0,    0,    0,  167,  168,    0,    0,    0,  174,
  175,    0,    0,   30,  112,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  108,  108,  108,  108,
    0,  108,    0,   28,    0,   27,  108,  108,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  200,    0,  204,
   29,    0,   30,    0,    0,    0,    0,    0,    0,  108,
  109,  109,  109,  109,    0,  109,    0,    0,  220,  221,
  109,  109,   28,    0,   27,    0,    0,    0,    0,    0,
    0,  236,  237,    0,  113,  113,  113,  113,    0,  113,
  243,  244,    0,  109,  113,  113,   29,   28,    0,   27,
    0,  107,  107,  107,  107,    0,  107,    0,   28,    0,
   27,  107,  107,    0,    0,    0,    0,  113,  114,  114,
  114,  114,    0,  114,    0,    0,    0,    0,  114,  114,
    0,    0,    0,    0,  107,   29,  272,    0,    0,    0,
    0,    0,    0,    0,  278,    0,    0,    0,  282,  283,
    0,  114,    0,    0,    0,    0,    0,    0,    0,  288,
    0,    0,    0,  290,  291,    0,    0,    0,    0,  296,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    2,    0,    0,    0,    0,    3,
    0,    4,    5,    6,    7,    0,    0,    8,    9,    0,
    0,    0,   10,   11,    0,   12,   13,   14,   15,    0,
    0,    0,    0,   16,   17,   18,   19,   20,    0,   21,
   22,   23,   24,    0,    0,    0,    0,    0,    0,    0,
    2,    0,    0,    0,    0,    3,    0,    4,    5,    6,
    7,    0,    0,    8,    9,    0,    0,    0,    0,   11,
    0,   12,   13,   14,   15,    0,    0,    0,    0,   16,
   17,   18,   19,   20,    0,   21,   22,   23,   24,    2,
    0,    0,    0,    0,    3,    0,    4,    5,    6,    7,
    0,    0,    8,    9,    0,    0,    0,    0,    0,    0,
   12,   13,   14,   15,    0,    0,    0,    0,   16,   17,
   18,   19,   20,    0,   21,   22,   23,   24,  118,  120,
    0,  125,    0,    0,  129,  103,   13,   14,    0,    0,
    0,    0,    0,   16,   17,   18,   19,   20,   14,   21,
   22,   23,   24,    0,   16,   17,   18,   19,   20,    0,
   21,   22,   23,   24,    0,    0,    0,  149,    0,    0,
    0,  150,  151,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  163,    0,    0,    0,  169,  170,    0,    0,    0,
    0,    0,    0,    0,  179,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  189,
  190,  191,  192,  193,  197,    0,    0,    0,    0,    0,
    0,    0,    0,  212,    0,  212,    0,    0,  217,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  230,    0,    0,
    0,    0,    0,    0,    0,  241,    0,    0,    0,    0,
    0,  212,  212,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  264,    0,    0,    0,    0,    0,    0,    0,
    0,  271,    0,    0,    0,    0,  273,  274,  275,    0,
    0,    0,    0,  279,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  289,    0,
    0,    0,    0,    0,    0,    0,  212,  212,
};
static const YYINT yycheck[] = {                         10,
   10,   45,   10,   10,   10,   10,   38,   10,   10,   45,
   45,  263,   10,   10,  179,   41,   58,  129,   10,   41,
  278,   10,   10,   10,  282,  278,  171,   38,  173,    0,
   41,  263,   40,   38,  278,   38,  259,  260,  261,  291,
   10,  284,  285,  278,   10,   10,  284,  285,   59,   60,
  283,   62,  217,   59,   59,  277,   59,  259,  260,  291,
   40,   10,  278,   51,  209,  210,  278,  179,  278,   10,
   40,  278,  278,  278,  106,  278,  124,   65,  278,  278,
   10,  258,   59,   59,   10,  268,  268,   40,   10,   41,
   60,   40,   62,   59,   59,   10,   10,   38,  124,  272,
  278,  125,  124,  278,  278,  217,  278,  278,  278,  278,
   40,   10,   10,  124,  125,  278,  123,  123,   59,  107,
   10,  123,   41,  165,   41,  269,  125,  278,   10,   59,
   60,  102,   62,  269,  269,  261,   38,  291,   45,  284,
  285,  263,  269,  125,  269,  177,  125,  261,  258,  291,
   40,  269,  125,  123,  278,  278,  269,   59,  125,  263,
  202,  149,  269,  125,  269,  125,  269,  125,  269,   59,
   60,  213,   62,  263,   10,  125,  261,  272,   37,  150,
  151,  177,  172,  234,   10,   60,  218,   62,   10,   -1,
   -1,  195,   -1,  123,   -1,   -1,   -1,  123,   -1,   -1,
   -1,  123,   38,   -1,   -1,   41,   -1,   -1,  123,  123,
   -1,   -1,   -1,  184,   -1,   -1,   38,   -1,   -1,   41,
   -1,   -1,   -1,   59,  123,  123,   10,   -1,   -1,   -1,
   -1,   -1,   -1,  123,  278,   -1,  280,   59,   -1,   -1,
   -1,  123,  278,  278,  280,  280,   -1,  258,  259,  260,
  261,   -1,  263,   -1,   -1,  263,   40,  268,  269,   -1,
   -1,  268,  268,   -1,  274,   -1,  268,  278,  279,  280,
  278,   -1,  274,  284,  285,  286,  287,  288,  289,  290,
  291,  292,  293,  294,  295,  296,  256,  257,  124,  125,
  296,  296,  262,  296,  264,  265,  266,  267,  296,  296,
  270,  271,  124,  125,  296,  275,  276,  296,  278,  279,
  280,  281,  278,  278,  263,   -1,  286,  287,  288,  289,
  290,   10,  292,  293,  294,  295,  296,  257,   -1,  278,
  296,  296,  262,   -1,  264,  265,  266,  267,   -1,  123,
  270,  271,  268,  284,  285,  275,  268,   -1,  278,  279,
  280,  281,  274,  268,  268,   -1,  286,  287,  288,  289,
  290,   -1,  292,  293,  294,  295,  296,  257,   41,  268,
  268,  278,  262,  280,  264,  265,  266,  267,   -1,   10,
  270,  271,  284,  285,   -1,  275,  268,   -1,  278,  279,
  280,  281,  274,   -1,   -1,   -1,  286,  287,  288,  289,
  290,   -1,  292,  293,  294,  295,  296,   38,   -1,   -1,
   41,  286,  287,  288,  289,  290,   -1,  292,   -1,  294,
  295,   -1,  258,  259,  260,  261,   10,  263,   59,   -1,
   -1,   -1,  268,  269,  123,   -1,  258,  259,  260,  261,
   10,  263,   -1,   -1,   -1,   -1,  268,  269,  284,  285,
   -1,   -1,  125,   -1,   38,  291,   -1,   41,   -1,   -1,
  296,   -1,  284,  285,   -1,   -1,   -1,   -1,   38,  291,
   -1,   41,   -1,  257,  296,   59,   -1,   -1,  262,   10,
  264,  265,  266,  267,   -1,   -1,   -1,  271,   -1,   59,
   -1,   -1,   -1,  124,  125,   -1,   -1,  281,   10,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   38,   -1,   -1,
   41,   -1,   -1,    0,   -1,   -1,   -1,   -1,   10,  189,
  190,  191,  192,  193,   -1,   -1,   38,   -1,   59,   41,
   -1,   -1,   10,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  124,  125,  212,   -1,   -1,   -1,   38,   59,   -1,   41,
   -1,   -1,   10,   -1,  124,  125,   -1,   -1,   -1,   -1,
   38,   -1,   -1,   41,   -1,   -1,   -1,   59,  257,   -1,
   -1,   10,   -1,  262,   -1,  264,  265,  266,  267,   -1,
   38,   59,  271,   41,   -1,  258,  259,  260,  261,   -1,
  263,   -1,  281,   -1,  125,  268,  269,   -1,   -1,   38,
   -1,   59,   41,  273,  274,  275,   10,   -1,   -1,   -1,
   -1,   -1,   -1,  125,  101,  102,   -1,   -1,  291,   -1,
   59,   -1,   -1,   -1,   -1,   -1,   -1,  258,  259,  260,
  261,   -1,  263,  125,   -1,   -1,   40,  268,  269,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  125,   -1,   -1,
   -1,   -1,   -1,  284,  285,   -1,   60,   -1,   62,   -1,
  291,   -1,   -1,  150,  151,  296,   -1,  125,   -1,   -1,
   -1,   -1,   -1,   10,  258,  259,  260,  261,   -1,  263,
   -1,   -1,   -1,   -1,  268,  269,  125,   -1,  258,  259,
  260,  261,   -1,  263,   -1,   -1,  183,  184,  268,  269,
  284,  285,   -1,   40,   -1,   -1,   -1,  291,   -1,   -1,
   -1,   -1,  296,   10,  284,  285,   -1,   -1,   -1,  123,
   -1,  291,   -1,   60,   -1,   62,  296,  258,  259,  260,
  261,   -1,  263,   -1,   -1,   -1,   -1,  268,  269,   -1,
   -1,   38,   -1,   -1,   41,   -1,  258,  259,  260,  261,
   -1,  263,   -1,  284,  285,   -1,  268,  269,   -1,   -1,
  291,   -1,   59,   -1,   -1,  296,  258,  259,  260,  261,
   -1,  263,  284,  285,   -1,   -1,  268,  269,   -1,  291,
  258,  259,  260,  261,  296,  263,  123,   -1,   -1,   41,
  268,  269,  284,  285,   -1,   -1,   -1,   -1,   -1,  291,
  258,  259,  260,  261,  296,  263,  284,  285,   -1,   -1,
  268,  269,   -1,  291,   -1,   -1,   10,   -1,  296,  258,
  259,  260,  261,   41,  263,   -1,  284,  285,  125,  268,
  269,   -1,   -1,  291,   -1,   -1,   -1,   -1,  296,   -1,
   -1,   -1,   -1,   -1,   38,  284,  285,   41,   -1,   -1,
   10,   -1,  291,  257,   -1,   -1,   -1,  296,  262,   -1,
  264,  265,  266,  267,   41,   59,  270,  271,   -1,    6,
    7,  275,  276,  125,  278,  279,  280,  281,   38,   -1,
   -1,   41,  286,  287,  288,  289,  290,   -1,  292,  293,
  294,  295,   29,   30,   -1,   -1,   -1,   -1,   -1,   59,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  125,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  257,   -1,   -1,   -1,   -1,  262,   -1,  264,  265,  266,
  267,  125,   -1,  270,  271,   -1,   -1,   -1,   -1,   -1,
   -1,  278,  279,  280,  281,   -1,   -1,   40,  125,  286,
  287,  288,  289,  290,   -1,  292,  293,  294,  295,   -1,
   -1,  258,  259,  260,  261,  125,  263,   60,   -1,   62,
   -1,  268,  269,   -1,   -1,   -1,  113,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  121,  122,   -1,   -1,   -1,  126,
  127,   -1,   -1,   40,  291,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  258,  259,  260,  261,
   -1,  263,   -1,   60,   -1,   62,  268,  269,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  164,   -1,  166,
  123,   -1,   40,   -1,   -1,   -1,   -1,   -1,   -1,  291,
  258,  259,  260,  261,   -1,  263,   -1,   -1,  185,  186,
  268,  269,   60,   -1,   62,   -1,   -1,   -1,   -1,   -1,
   -1,  198,  199,   -1,  258,  259,  260,  261,   -1,  263,
  207,  208,   -1,  291,  268,  269,  123,   60,   -1,   62,
   -1,  258,  259,  260,  261,   -1,  263,   -1,   60,   -1,
   62,  268,  269,   -1,   -1,   -1,   -1,  291,  258,  259,
  260,  261,   -1,  263,   -1,   -1,   -1,   -1,  268,  269,
   -1,   -1,   -1,   -1,  291,  123,  253,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  261,   -1,   -1,   -1,  265,  266,
   -1,  291,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  276,
   -1,   -1,   -1,  280,  281,   -1,   -1,   -1,   -1,  286,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  257,   -1,   -1,   -1,   -1,  262,
   -1,  264,  265,  266,  267,   -1,   -1,  270,  271,   -1,
   -1,   -1,  275,  276,   -1,  278,  279,  280,  281,   -1,
   -1,   -1,   -1,  286,  287,  288,  289,  290,   -1,  292,
  293,  294,  295,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  257,   -1,   -1,   -1,   -1,  262,   -1,  264,  265,  266,
  267,   -1,   -1,  270,  271,   -1,   -1,   -1,   -1,  276,
   -1,  278,  279,  280,  281,   -1,   -1,   -1,   -1,  286,
  287,  288,  289,  290,   -1,  292,  293,  294,  295,  257,
   -1,   -1,   -1,   -1,  262,   -1,  264,  265,  266,  267,
   -1,   -1,  270,  271,   -1,   -1,   -1,   -1,   -1,   -1,
  278,  279,  280,  281,   -1,   -1,   -1,   -1,  286,  287,
  288,  289,  290,   -1,  292,  293,  294,  295,   56,   57,
   -1,   59,   -1,   -1,   62,  278,  279,  280,   -1,   -1,
   -1,   -1,   -1,  286,  287,  288,  289,  290,  280,  292,
  293,  294,  295,   -1,  286,  287,  288,  289,  290,   -1,
  292,  293,  294,  295,   -1,   -1,   -1,   95,   -1,   -1,
   -1,   99,  100,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  119,   -1,   -1,   -1,  123,  124,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  132,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  157,
  158,  159,  160,  161,  162,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  171,   -1,  173,   -1,   -1,  176,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  195,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  203,   -1,   -1,   -1,   -1,
   -1,  209,  210,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  240,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  249,   -1,   -1,   -1,   -1,  254,  255,  256,   -1,
   -1,   -1,   -1,  261,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  276,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  284,  285,
};
#define YYFINAL 31
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 296
#define YYUNDFTOKEN 334
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"'&'",0,"'('","')'",0,0,0,"'-'",0,0,0,0,0,0,0,0,0,0,0,0,0,"';'",
"'<'",0,"'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'","'|'","'}'",0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,"IF","THEN","ELSE","ELIF","FI","CASE","ESAC","FOR","SELECT",
"WHILE","UNTIL","DO","DONE","FUNCTION","COND_START","COND_END","COND_ERROR",
"IN","BANG","TIME","TIMEOPT","WORD","ASSIGNMENT_WORD","NUMBER","ARITH_CMD",
"ARITH_FOR_EXPRS","COND_CMD","AND_AND","OR_OR","GREATER_GREATER","LESS_LESS",
"LESS_AND","LESS_LESS_LESS","GREATER_AND","SEMI_SEMI","LESS_LESS_MINUS",
"AND_GREATER","LESS_GREATER","GREATER_BAR","yacc_EOF",0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : inputunit",
"inputunit : simple_list simple_list_terminator",
"inputunit : '\\n'",
"inputunit : error '\\n'",
"inputunit : yacc_EOF",
"word_list : WORD",
"word_list : word_list WORD",
"redirection : '>' WORD",
"redirection : '<' WORD",
"redirection : NUMBER '>' WORD",
"redirection : NUMBER '<' WORD",
"redirection : GREATER_GREATER WORD",
"redirection : NUMBER GREATER_GREATER WORD",
"redirection : LESS_LESS WORD",
"redirection : NUMBER LESS_LESS WORD",
"redirection : LESS_LESS_LESS WORD",
"redirection : NUMBER LESS_LESS_LESS WORD",
"redirection : LESS_AND NUMBER",
"redirection : NUMBER LESS_AND NUMBER",
"redirection : GREATER_AND NUMBER",
"redirection : NUMBER GREATER_AND NUMBER",
"redirection : LESS_AND WORD",
"redirection : NUMBER LESS_AND WORD",
"redirection : GREATER_AND WORD",
"redirection : NUMBER GREATER_AND WORD",
"redirection : LESS_LESS_MINUS WORD",
"redirection : NUMBER LESS_LESS_MINUS WORD",
"redirection : GREATER_AND '-'",
"redirection : NUMBER GREATER_AND '-'",
"redirection : LESS_AND '-'",
"redirection : NUMBER LESS_AND '-'",
"redirection : AND_GREATER WORD",
"redirection : NUMBER LESS_GREATER WORD",
"redirection : LESS_GREATER WORD",
"redirection : GREATER_BAR WORD",
"redirection : NUMBER GREATER_BAR WORD",
"simple_command_element : WORD",
"simple_command_element : ASSIGNMENT_WORD",
"simple_command_element : redirection",
"redirection_list : redirection",
"redirection_list : redirection_list redirection",
"simple_command : simple_command_element",
"simple_command : simple_command simple_command_element",
"command : simple_command",
"command : shell_command",
"command : shell_command redirection_list",
"command : function_def",
"shell_command : for_command",
"shell_command : case_command",
"shell_command : WHILE compound_list DO compound_list DONE",
"shell_command : UNTIL compound_list DO compound_list DONE",
"shell_command : select_command",
"shell_command : if_command",
"shell_command : subshell",
"shell_command : group_command",
"shell_command : arith_command",
"shell_command : cond_command",
"shell_command : arith_for_command",
"for_command : FOR WORD newline_list DO compound_list DONE",
"for_command : FOR WORD newline_list '{' compound_list '}'",
"for_command : FOR WORD ';' newline_list DO compound_list DONE",
"for_command : FOR WORD ';' newline_list '{' compound_list '}'",
"for_command : FOR WORD newline_list IN word_list list_terminator newline_list DO compound_list DONE",
"for_command : FOR WORD newline_list IN word_list list_terminator newline_list '{' compound_list '}'",
"for_command : FOR WORD newline_list IN list_terminator newline_list DO compound_list DONE",
"for_command : FOR WORD newline_list IN list_terminator newline_list '{' compound_list '}'",
"arith_for_command : FOR ARITH_FOR_EXPRS list_terminator newline_list DO compound_list DONE",
"arith_for_command : FOR ARITH_FOR_EXPRS list_terminator newline_list '{' compound_list '}'",
"arith_for_command : FOR ARITH_FOR_EXPRS DO compound_list DONE",
"arith_for_command : FOR ARITH_FOR_EXPRS '{' compound_list '}'",
"select_command : SELECT WORD newline_list DO list DONE",
"select_command : SELECT WORD newline_list '{' list '}'",
"select_command : SELECT WORD ';' newline_list DO list DONE",
"select_command : SELECT WORD ';' newline_list '{' list '}'",
"select_command : SELECT WORD newline_list IN word_list list_terminator newline_list DO list DONE",
"select_command : SELECT WORD newline_list IN word_list list_terminator newline_list '{' list '}'",
"case_command : CASE WORD newline_list IN newline_list ESAC",
"case_command : CASE WORD newline_list IN case_clause_sequence newline_list ESAC",
"case_command : CASE WORD newline_list IN case_clause ESAC",
"function_def : WORD '(' ')' newline_list function_body",
"function_def : FUNCTION WORD '(' ')' newline_list function_body",
"function_def : FUNCTION WORD newline_list function_body",
"function_body : shell_command",
"function_body : shell_command redirection_list",
"subshell : '(' compound_list ')'",
"if_command : IF compound_list THEN compound_list FI",
"if_command : IF compound_list THEN compound_list ELSE compound_list FI",
"if_command : IF compound_list THEN compound_list elif_clause FI",
"group_command : '{' compound_list '}'",
"arith_command : ARITH_CMD",
"cond_command : COND_START COND_CMD COND_END",
"elif_clause : ELIF compound_list THEN compound_list",
"elif_clause : ELIF compound_list THEN compound_list ELSE compound_list",
"elif_clause : ELIF compound_list THEN compound_list elif_clause",
"case_clause : pattern_list",
"case_clause : case_clause_sequence pattern_list",
"pattern_list : newline_list pattern ')' compound_list",
"pattern_list : newline_list pattern ')' newline_list",
"pattern_list : newline_list '(' pattern ')' compound_list",
"pattern_list : newline_list '(' pattern ')' newline_list",
"case_clause_sequence : pattern_list SEMI_SEMI",
"case_clause_sequence : case_clause_sequence pattern_list SEMI_SEMI",
"pattern : WORD",
"pattern : pattern '|' WORD",
"list : newline_list list0",
"compound_list : list",
"compound_list : newline_list list1",
"list0 : list1 '\\n' newline_list",
"list0 : list1 '&' newline_list",
"list0 : list1 ';' newline_list",
"list1 : list1 AND_AND newline_list list1",
"list1 : list1 OR_OR newline_list list1",
"list1 : list1 '&' newline_list list1",
"list1 : list1 ';' newline_list list1",
"list1 : list1 '\\n' newline_list list1",
"list1 : pipeline_command",
"simple_list_terminator : '\\n'",
"simple_list_terminator : yacc_EOF",
"list_terminator : '\\n'",
"list_terminator : ';'",
"list_terminator : yacc_EOF",
"newline_list :",
"newline_list : newline_list '\\n'",
"simple_list : simple_list1",
"simple_list : simple_list1 '&'",
"simple_list : simple_list1 ';'",
"simple_list1 : simple_list1 AND_AND newline_list simple_list1",
"simple_list1 : simple_list1 OR_OR newline_list simple_list1",
"simple_list1 : simple_list1 '&' simple_list1",
"simple_list1 : simple_list1 ';' simple_list1",
"simple_list1 : pipeline_command",
"pipeline_command : pipeline",
"pipeline_command : BANG pipeline",
"pipeline_command : timespec pipeline",
"pipeline_command : timespec BANG pipeline",
"pipeline_command : BANG timespec pipeline",
"pipeline_command : timespec list_terminator",
"pipeline : pipeline '|' newline_list pipeline",
"pipeline : command",
"timespec : TIME",
"timespec : TIME TIMEOPT",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 1020 "./parse.y"

/* Possible states for the parser that require it to do special things. */
#define PST_CASEPAT	0x0001		/* in a case pattern list */
#define PST_ALEXPNEXT	0x0002		/* expand next word for aliases */
#define PST_ALLOWOPNBRC	0x0004		/* allow open brace for function def */
#define PST_NEEDCLOSBRC	0x0008		/* need close brace */
#define PST_DBLPAREN	0x0010		/* double-paren parsing */
#define PST_SUBSHELL	0x0020		/* ( ... ) subshell */
#define PST_CMDSUBST	0x0040		/* $( ... ) command substitution */
#define PST_CASESTMT	0x0080		/* parsing a case statement */
#define PST_CONDCMD	0x0100		/* parsing a [[...]] command */
#define PST_CONDEXPR	0x0200		/* parsing the guts of [[...]] */
#define PST_ARITHFOR	0x0400		/* parsing an arithmetic for command */
#define PST_ALEXPAND	0x0800		/* OK to expand aliases - unused */
#define PST_CMDTOKEN	0x1000		/* command token OK - unused */
#define PST_COMPASSIGN	0x2000		/* parsing x=(...) compound assignment */
#define PST_ASSIGNOK	0x4000		/* assignment statement ok in this context */
#define PST_REGEXP	0x8000		/* parsing an ERE/BRE as a single word */

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_INITIAL_SIZE 496
#define TOKEN_DEFAULT_GROW_SIZE 512

/* Should we call prompt_again? */
#define SHOULD_PROMPT() \
  (interactive && (bash_input.type == st_stdin || bash_input.type == st_stream))

#if defined (ALIAS)
#  define expanding_alias() (pushed_string_list && pushed_string_list->expander)
#else
#  define expanding_alias() 0
#endif

/* The token currently being read. */
static int current_token;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
static int last_read_token;

/* The token read prior to last_read_token. */
static int token_before_that;

/* The token read prior to token_before_that. */
static int two_tokens_ago;

/* The current parser state. */
static int parser_state;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

#ifdef DEBUG
static void
debug_parser (i)
     int i;
{
#if YYDEBUG != 0
  yydebug = i;
#endif
}
#endif

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget, type, location) makes the function GET the
   installed function for getting the next character, makes UNGET the
   installed function for un-getting a character, sets the type of stream
   (either string or file) from TYPE, and makes LOCATION point to where
   the input is coming from. */

/* Unconditionally returns end-of-file. */
int
return_EOF ()
{
  return (EOF);
}

/* Variable containing the current get and unget functions.
   See ./input.h for a clearer description. */
BASH_INPUT bash_input;

/* Set all of the fields in BASH_INPUT to NULL.  Free bash_input.name if it
   is non-null, avoiding a memory leak. */
void
initialize_bash_input ()
{
  bash_input.type = st_none;
  FREE (bash_input.name);
  bash_input.name = (char *)NULL;
  bash_input.location.file = (FILE *)NULL;
  bash_input.location.string = (char *)NULL;
  bash_input.getter = (sh_cget_func_t *)NULL;
  bash_input.ungetter = (sh_cunget_func_t *)NULL;
}

/* Set the contents of the current bash input stream from
   GET, UNGET, TYPE, NAME, and LOCATION. */
void
init_yy_io (get, unget, type, name, location)
     sh_cget_func_t *get;
     sh_cunget_func_t *unget;
     enum stream_type type;
     const char *name;
     INPUT_STREAM location;
{
  bash_input.type = type;
  FREE (bash_input.name);
  bash_input.name = name ? savestring (name) : (char *)NULL;

  /* XXX */
#if defined (CRAY)
  memcpy((char *)&bash_input.location.string, (char *)&location.string, sizeof(location));
#else
  bash_input.location = location;
#endif
  bash_input.getter = get;
  bash_input.ungetter = unget;
}

char *
yy_input_name ()
{
  return (bash_input.name ? bash_input.name : "stdin");
}

/* Call this to get the next character of input. */
static int
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
static int
yy_ungetc (c)
     int c;
{
  return (*(bash_input.ungetter)) (c);
}

#if defined (BUFFERED_INPUT)
#ifdef INCLUDE_UNUSED
int
input_file_descriptor ()
{
  switch (bash_input.type)
    {
    case st_stream:
      return (fileno (bash_input.location.file));
    case st_bstream:
      return (bash_input.location.buffered_fd);
    case st_stdin:
    default:
      return (fileno (stdin));
    }
}
#endif
#endif /* BUFFERED_INPUT */

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#if defined (READLINE)
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int
yy_readline_get ()
{
  SigHandler *old_sigint;
  int line_len;
  unsigned char c;

  if (!current_readline_line)
    {
      if (!bash_readline_initialized)
	initialize_readline ();

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp, 0);
#endif /* JOB_CONTROL */

      old_sigint = (SigHandler *)NULL;
      if (signal_is_ignored (SIGINT) == 0)
	{
	  old_sigint = (SigHandler *)set_signal_handler (SIGINT, sigint_sighandler);
	  interrupt_immediately++;
	}
      terminate_immediately = 1;

      current_readline_line = readline (current_readline_prompt ?
      					  current_readline_prompt : "");

      terminate_immediately = 0;
      if (signal_is_ignored (SIGINT) == 0 && old_sigint)
	{
	  interrupt_immediately--;
	  set_signal_handler (SIGINT, old_sigint);
	}

#if 0
      /* Reset the prompt to the decoded value of prompt_string_pointer. */
      reset_readline_prompt ();
#endif

      if (current_readline_line == 0)
	return (EOF);

      current_readline_line_index = 0;
      line_len = strlen (current_readline_line);

      current_readline_line = (char *)xrealloc (current_readline_line, 2 + line_len);
      current_readline_line[line_len++] = '\n';
      current_readline_line[line_len] = '\0';
    }

  if (current_readline_line[current_readline_line_index] == 0)
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      c = current_readline_line[current_readline_line_index++];
      return (c);
    }
}

static int
yy_readline_unget (c)
     int c;
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
  return (c);
}

void
with_input_from_stdin ()
{
  INPUT_STREAM location;

  if (bash_input.type != st_stdin && stream_on_stack (st_stdin) == 0)
    {
      location.string = current_readline_line;
      init_yy_io (yy_readline_get, yy_readline_unget,
		  st_stdin, "readline stdin", location);
    }
}

#else  /* !READLINE */

void
with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif	/* !READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

static int
yy_string_get ()
{
  register char *string;
  register unsigned char c;

  string = bash_input.location.string;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *string++;
      bash_input.location.string = string;
      return (c);
    }
  else
    return (EOF);
}

static int
yy_string_unget (c)
     int c;
{
  *(--bash_input.location.string) = c;
  return (c);
}

void
with_input_from_string (string, name)
     char *string;
     const char *name;
{
  INPUT_STREAM location;

  location.string = string;
  init_yy_io (yy_string_get, yy_string_unget, st_string, name, location);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

/* These two functions used to test the value of the HAVE_RESTARTABLE_SYSCALLS
   define, and just use getc/ungetc if it was defined, but since bash
   installs its signal handlers without the SA_RESTART flag, some signals
   (like SIGCHLD, SIGWINCH, etc.) received during a read(2) will not cause
   the read to be restarted.  We need to restart it ourselves. */

static int
yy_stream_get ()
{
  int result;

  result = EOF;
  if (bash_input.location.file)
    {
      if (interactive)
	{
	  interrupt_immediately++;
	  terminate_immediately++;
	}
      result = getc_with_restart (bash_input.location.file);
      if (interactive)
	{
	  interrupt_immediately--;
	  terminate_immediately--;
	}
    }
  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
  return (ungetc_with_restart (c, bash_input.location.file));
}

void
with_input_from_stream (stream, name)
     FILE *stream;
     const char *name;
{
  INPUT_STREAM location;

  location.file = stream;
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, name, location);
}

typedef struct stream_saver {
  struct stream_saver *next;
  BASH_INPUT bash_input;
  int line;
#if defined (BUFFERED_INPUT)
  BUFFERED_STREAM *bstream;
#endif /* BUFFERED_INPUT */
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

#if defined (COND_COMMAND)
static int cond_lineno;
static int cond_token;
#endif

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

void
push_stream (reset_lineno)
     int reset_lineno;
{
  STREAM_SAVER *saver = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));

  xbcopy ((char *)&bash_input, (char *)&(saver->bash_input), sizeof (BASH_INPUT));

#if defined (BUFFERED_INPUT)
  saver->bstream = (BUFFERED_STREAM *)NULL;
  /* If we have a buffered stream, clear out buffers[fd]. */
  if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
    saver->bstream = set_buffered_stream (bash_input.location.buffered_fd,
    					  (BUFFERED_STREAM *)NULL);
#endif /* BUFFERED_INPUT */

  saver->line = line_number;
  bash_input.name = (char *)NULL;
  saver->next = stream_list;
  stream_list = saver;
  EOF_Reached = 0;
  if (reset_lineno)
    line_number = 0;
}

void
pop_stream ()
{
  if (!stream_list)
    EOF_Reached = 1;
  else
    {
      STREAM_SAVER *saver = stream_list;

      EOF_Reached = 0;
      stream_list = stream_list->next;

      init_yy_io (saver->bash_input.getter,
		  saver->bash_input.ungetter,
		  saver->bash_input.type,
		  saver->bash_input.name,
		  saver->bash_input.location);

#if defined (BUFFERED_INPUT)
      /* If we have a buffered stream, restore buffers[fd]. */
      /* If the input file descriptor was changed while this was on the
	 save stack, update the buffered fd to the new file descriptor and
	 re-establish the buffer <-> bash_input fd correspondence. */
      if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
	{
	  if (bash_input_fd_changed)
	    {
	      bash_input_fd_changed = 0;
	      if (default_buffered_input >= 0)
		{
		  bash_input.location.buffered_fd = default_buffered_input;
		  saver->bstream->b_fd = default_buffered_input;
		  SET_CLOSE_ON_EXEC (default_buffered_input);
		}
	    }
	  /* XXX could free buffered stream returned as result here. */
	  set_buffered_stream (bash_input.location.buffered_fd, saver->bstream);
	}
#endif /* BUFFERED_INPUT */

      line_number = saver->line;

      FREE (saver->bash_input.name);
      free (saver);
    }
}

/* Return 1 if a stream of type TYPE is saved on the stack. */
int
stream_on_stack (type)
     enum stream_type type;
{
  register STREAM_SAVER *s;

  for (s = stream_list; s; s = s->next)
    if (s->bash_input.type == type)
      return 1;
  return 0;
}

/* Save the current token state and return it in a malloced array. */
int *
save_token_state ()
{
  int *ret;

  ret = (int *)xmalloc (3 * sizeof (int));
  ret[0] = last_read_token;
  ret[1] = token_before_that;
  ret[2] = two_tokens_ago;
  return ret;
}

void
restore_token_state (ts)
     int *ts;
{
  if (ts == 0)
    return;
  last_read_token = ts[0];
  token_before_that = ts[1];
  two_tokens_ago = ts[2];
}

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list' is:
 *
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)

#define END_OF_ALIAS 0

/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

/*
 * Pushing and popping strings.  This works together with shell_getc to
 * implement alias expansion on a per-token basis.
 */

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
#if defined (ALIAS)
  alias_t *expander;   /* alias that caused this line to be pushed. */
#endif
  int saved_line_size, saved_line_index, saved_line_terminator;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, ap)
     char *s;
     int expand;
     alias_t *ap;
{
  STRING_SAVER *temp = (STRING_SAVER *)xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
#if defined (ALIAS)
  temp->expander = ap;
#endif
  temp->next = pushed_string_list;
  pushed_string_list = temp;

#if defined (ALIAS)
  if (ap)
    ap->flags |= AL_BEINGEXPANDED;
#endif

  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
#if 0
  parser_state &= ~PST_ALEXPNEXT;	/* XXX */
#endif

  set_line_mbstate ();
}

/*
 * Make the top of the pushed_string stack be the current shell input.
 * Only called when there is something on the stack.  Called from shell_getc
 * when it thinks it has consumed the string generated by an alias expansion
 * and needs to return to the original input line.
 */
static void
pop_string ()
{
  STRING_SAVER *t;

  FREE (shell_input_line);
  shell_input_line = pushed_string_list->saved_line;
  shell_input_line_index = pushed_string_list->saved_line_index;
  shell_input_line_size = pushed_string_list->saved_line_size;
  shell_input_line_terminator = pushed_string_list->saved_line_terminator;

  if (pushed_string_list->expand_alias)
    parser_state |= PST_ALEXPNEXT;
  else
    parser_state &= ~PST_ALEXPNEXT;

  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;

#if defined (ALIAS)
  if (t->expander)
    t->expander->flags &= ~AL_BEINGEXPANDED;
#endif

  free ((char *)t);

  set_line_mbstate ();
}

static void
free_string_list ()
{
  register STRING_SAVER *t, *t1;

  for (t = pushed_string_list; t; )
    {
      t1 = t->next;
      FREE (t->saved_line);
#if defined (ALIAS)
      if (t->expander)
	t->expander->flags &= ~AL_BEINGEXPANDED;
#endif
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

#endif /* ALIAS || DPAREN_ARITHMETIC */

void
free_pushed_string_input ()
{
#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  free_string_list ();
#endif
}

/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL.  If REMOVE_QUOTED_NEWLINE
   is non-zero, we remove unquoted \<newline> pairs.  This is used by
   read_secondary_line to read here documents. */
static char *
read_a_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  static char *line_buffer = (char *)NULL;
  static int buffer_size = 0;
  int indx = 0, c, peekc, pass_next;

#if defined (READLINE)
  if (no_line_editing && SHOULD_PROMPT ())
#else
  if (SHOULD_PROMPT ())
#endif
    print_prompt ();

  pass_next = 0;
  while (1)
    {
      /* Allow immediate exit if interrupted during input. */
      QUIT;

      c = yy_getc ();

      /* Ignore null bytes in input. */
      if (c == 0)
	{
#if 0
	  internal_warning ("read_a_line: ignored null byte in input");
#endif
	  continue;
	}

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  if (interactive && bash_input.type == st_stream)
	    clearerr (stdin);
	  if (indx == 0)
	    return ((char *)NULL);
	  c = '\n';
	}

      /* `+2' in case the final character in the buffer is a newline. */
      RESIZE_MALLOCED_BUFFER (line_buffer, indx, 2, buffer_size, 128);

      /* IF REMOVE_QUOTED_NEWLINES is non-zero, we are reading a
	 here document with an unquoted delimiter.  In this case,
	 the line will be expanded as if it were in double quotes.
	 We allow a backslash to escape the next character, but we
	 need to treat the backslash specially only if a backslash
	 quoting a backslash-newline pair appears in the line. */
      if (pass_next)
	{
	  line_buffer[indx++] = c;
	  pass_next = 0;
	}
      else if (c == '\\' && remove_quoted_newline)
	{
	  peekc = yy_getc ();
	  if (peekc == '\n')
	    {
	      line_number++;
	      continue;	/* Make the unquoted \<newline> pair disappear. */
	    }
	  else
	    {
	      yy_ungetc (peekc);
	      pass_next = 1;
	      line_buffer[indx++] = c;		/* Preserve the backslash. */
	    }
	}
      else
	line_buffer[indx++] = c;

      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt.  This is used to read the lines of a here
   document.  REMOVE_QUOTED_NEWLINE is non-zero if we should remove
   newlines quoted with backslashes while reading the line.  It is
   non-zero unless the delimiter of the here document was quoted. */
char *
read_secondary_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  prompt_string_pointer = &ps2_prompt;
  if (SHOULD_PROMPT())
    prompt_again ();
  return (read_a_line (remove_quoted_newline));
}

/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command. */
STRING_INT_ALIST word_token_alist[] = {
  { "if", IF },
  { "then", THEN },
  { "else", ELSE },
  { "elif", ELIF },
  { "fi", FI },
  { "case", CASE },
  { "esac", ESAC },
  { "for", FOR },
#if defined (SELECT_COMMAND)
  { "select", SELECT },
#endif
  { "while", WHILE },
  { "until", UNTIL },
  { "do", DO },
  { "done", DONE },
  { "in", IN },
  { "function", FUNCTION },
#if defined (COMMAND_TIMING)
  { "time", TIME },
#endif
  { "{", '{' },
  { "}", '}' },
  { "!", BANG },
#if defined (COND_COMMAND)
  { "[[", COND_START },
  { "]]", COND_END },
#endif
  { (char *)NULL, 0}
};

/* other tokens that can be returned by read_token() */
STRING_INT_ALIST other_token_alist[] = {
  /* Multiple-character tokens with special values */
  { "-p", TIMEOPT },
  { "&&", AND_AND },
  { "||", OR_OR },
  { ">>", GREATER_GREATER },
  { "<<", LESS_LESS },
  { "<&", LESS_AND },
  { ">&", GREATER_AND },
  { ";;", SEMI_SEMI },
  { "<<-", LESS_LESS_MINUS },
  { "<<<", LESS_LESS_LESS },
  { "&>", AND_GREATER },
  { "<>", LESS_GREATER },
  { ">|", GREATER_BAR },
  { "EOF", yacc_EOF },
  /* Tokens whose value is the character itself */
  { ">", '>' },
  { "<", '<' },
  { "-", '-' },
  { "{", '{' },
  { "}", '}' },
  { ";", ';' },
  { "(", '(' },
  { ")", ')' },
  { "|", '|' },
  { "&", '&' },
  { "newline", '\n' },
  { (char *)NULL, 0}
};

/* others not listed here:
	WORD			look at yylval.word
	ASSIGNMENT_WORD		look at yylval.word
	NUMBER			look at yylval.number
	ARITH_CMD		look at yylval.word_list
	ARITH_FOR_EXPRS		look at yylval.word_list
	COND_CMD		look at yylval.command
*/

/* These are used by read_token_word, but appear up here so that shell_getc
   can use them to decide when to add otherwise blank lines to the history. */

/* The primary delimiter stack. */
struct dstack dstack = {  (char *)NULL, 0, 0 };

/* A temporary delimiter stack to be used when decoding prompt strings.
   This is needed because command substitutions in prompt strings (e.g., PS2)
   can screw up the parser's quoting state. */
static struct dstack temp_dstack = { (char *)NULL, 0, 0 };

/* Macro for accessing the top delimiter on the stack.  Returns the
   delimiter or zero if none. */
#define current_delimiter(ds) \
  (ds.delimiter_depth ? ds.delimiters[ds.delimiter_depth - 1] : 0)

#define push_delimiter(ds, character) \
  do \
    { \
      if (ds.delimiter_depth + 2 > ds.delimiter_space) \
	ds.delimiters = (char *)xrealloc \
	  (ds.delimiters, (ds.delimiter_space += 10) * sizeof (char)); \
      ds.delimiters[ds.delimiter_depth] = character; \
      ds.delimiter_depth++; \
    } \
  while (0)

#define pop_delimiter(ds)	ds.delimiter_depth--

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line.  This is called by read_token when the shell is
   processing normal command input. */

/* This implements one-character lookahead/lookbehind across physical input
   lines, to avoid something being lost because it's pushed back with
   shell_ungetc when we're at the start of a line. */
static int eol_ungetc_lookahead = 0;

static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  register int i;
  int c;
  unsigned char uc;

  QUIT;

  if (sigwinch_received)
    {
      sigwinch_received = 0;
      get_new_window_size (0, (int *)0, (int *)0);
    }
      
  if (eol_ungetc_lookahead)
    {
      c = eol_ungetc_lookahead;
      eol_ungetc_lookahead = 0;
      return (c);
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS && !DPAREN_ARITHMETIC */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS && !DPAREN_ARITHMETIC */
    {
      line_number++;

    restart_read:

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      i = 0;
      shell_input_line_terminator = 0;

      /* If the shell is interatctive, but not currently printing a prompt
         (interactive_shell && interactive == 0), we don't want to print
         notifies or cleanup the jobs -- we want to defer it until we do
         print the next prompt. */
      if (interactive_shell == 0 || SHOULD_PROMPT())
	{
#if defined (JOB_CONTROL)
      /* This can cause a problem when reading a command as the result
	 of a trap, when the trap is called from flush_child.  This call
	 had better not cause jobs to disappear from the job table in
	 that case, or we will have big trouble. */
	  notify_and_cleanup ();
#else /* !JOB_CONTROL */
	  cleanup_dead_jobs ();
#endif /* !JOB_CONTROL */
	}

#if defined (READLINE)
      if (no_line_editing && SHOULD_PROMPT())
#else
      if (SHOULD_PROMPT())
#endif
	print_prompt ();

      if (bash_input.type == st_stream)
	clearerr (stdin);

      while (1)
	{
	  c = yy_getc ();

	  /* Allow immediate exit if interrupted during input. */
	  QUIT;

	  if (c == '\0')
	    {
#if 0
	      internal_warning ("shell_getc: ignored null byte in input");
#endif
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (shell_input_line, i, 2, shell_input_line_size, 256);

	  if (c == EOF)
	    {
	      if (bash_input.type == st_stream)
		clearerr (stdin);

	      if (i == 0)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      current_command_line_count++;
	      break;
	    }
	}

      shell_input_line_index = 0;
      shell_input_line_len = i;		/* == strlen (shell_input_line) */

      set_line_mbstate ();

#if defined (HISTORY)
      if (remember_on_history && shell_input_line && shell_input_line[0])
	{
	  char *expansions;
#  if defined (BANG_HISTORY)
	  int old_hist;

	  /* If the current delimiter is a single quote, we should not be
	     performing history expansion, even if we're on a different
	     line from the original single quote. */
	  old_hist = history_expansion_inhibited;
	  if (current_delimiter (dstack) == '\'')
	    history_expansion_inhibited = 1;
#  endif
	  expansions = pre_process_line (shell_input_line, 1, 1);
#  if defined (BANG_HISTORY)
	  history_expansion_inhibited = old_hist;
#  endif
	  if (expansions != shell_input_line)
	    {
	      free (shell_input_line);
	      shell_input_line = expansions;
	      shell_input_line_len = shell_input_line ?
					strlen (shell_input_line) : 0;
	      if (!shell_input_line_len)
		current_command_line_count--;

	      /* We have to force the xrealloc below because we don't know
		 the true allocated size of shell_input_line anymore. */
	      shell_input_line_size = shell_input_line_len;

	      set_line_mbstate ();
	    }
	}
      /* Try to do something intelligent with blank lines encountered while
	 entering multi-line commands.  XXX - this is grotesque */
      else if (remember_on_history && shell_input_line &&
	       shell_input_line[0] == '\0' &&
	       current_command_line_count > 1)
	{
	  if (current_delimiter (dstack))
	    /* We know shell_input_line[0] == 0 and we're reading some sort of
	       quoted string.  This means we've got a line consisting of only
	       a newline in a quoted string.  We want to make sure this line
	       gets added to the history. */
	    maybe_add_history (shell_input_line);
	  else
	    {
	      char *hdcs;
	      hdcs = history_delimiting_chars ();
	      if (hdcs && hdcs[0] == ';')
		maybe_add_history (shell_input_line);
	    }
	}

#endif /* HISTORY */

      if (shell_input_line)
	{
	  /* Lines that signify the end of the shell's input should not be
	     echoed. */
	  if (echo_input_at_read && (shell_input_line[0] ||
				     shell_input_line_terminator != EOF))
	    fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  if (SHOULD_PROMPT ())
	    prompt_again ();
	  goto restart_read;
	}

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  if (shell_input_line_len + 3 > shell_input_line_size)
	    shell_input_line = (char *)xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  shell_input_line[shell_input_line_len] = '\n';
	  shell_input_line[shell_input_line_len + 1] = '\0';

	  set_line_mbstate ();
	}
    }

  uc = shell_input_line[shell_input_line_index];

  if (uc)
    shell_input_line_index++;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If UC is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  if (!uc && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      pop_string ();
      uc = shell_input_line[shell_input_line_index];
      if (uc)
	shell_input_line_index++;
    }
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if MBTEST(uc == '\\' && remove_quoted_newline && shell_input_line[shell_input_line_index] == '\n')
    {
	if (SHOULD_PROMPT ())
	  prompt_again ();
	line_number++;
	goto restart_read;
    }

  if (!uc && shell_input_line_terminator == EOF)
    return ((shell_input_line_index != 0) ? '\n' : EOF);

  return (uc);
}

/* Put C back into the input for the shell.  This might need changes for
   HANDLE_MULTIBYTE around EOLs.  Since we (currently) never push back a
   character different than we read, shell_input_line_property doesn't need
   to change when manipulating shell_input_line.  The define for
   last_shell_getc_is_singlebyte should take care of it, though. */
static void
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
  else
    eol_ungetc_lookahead = c;
}

char *
parser_remaining_input ()
{
  if (shell_input_line == 0)
    return 0;
  if (shell_input_line_index < 0 || shell_input_line_index >= shell_input_line_len)
    return '\0';	/* XXX */
  return (shell_input_line + shell_input_line_index);
}

#ifdef INCLUDE_UNUSED
/* Back the input pointer up by one, effectively `ungetting' a character. */
static void
shell_ungetchar ()
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line_index--;
}
#endif

/* Discard input until CHARACTER is seen, then push that character back
   onto the input stream. */
static void
discard_until (character)
     int character;
{
  int c;

  while ((c = shell_getc (0)) != EOF && c != character)
    ;

  if (c != EOF)
    shell_ungetc (c);
}

void
execute_variable_command (command, vname)
     char *command, *vname;
{
  char *last_lastarg;
  sh_parser_state_t ps;

  save_parser_state (&ps);
  last_lastarg = get_string_value ("_");
  if (last_lastarg)
    last_lastarg = savestring (last_lastarg);

  parse_and_execute (savestring (command), vname, SEVAL_NONINT|SEVAL_NOHIST);

  restore_parser_state (&ps);
  bind_variable ("_", last_lastarg, 0);
  FREE (last_lastarg);

  if (token_to_read == '\n')	/* reset_parser was called */
    token_to_read = 0;
}

/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
static char *token = (char *)NULL;

/* Current size of the token buffer. */
static int token_buffer_size;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */
static int
yylex ()
{
  if (interactive && (current_token == 0 || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (prompt_is_ps1 && time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Avoid printing a prompt if we're not going to read anything, e.g.
	 after resetting the parser with read_token (RESET). */
      if (token_to_read == 0 && SHOULD_PROMPT ())
	prompt_again ();
    }

  two_tokens_ago = token_before_that;
  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int esacs_needed_count;

static void
push_heredoc (r)
     REDIRECT *r;
{
  if (need_here_doc >= HEREDOC_MAX)
    {
      last_command_exit_value = EX_BADUSAGE;
      need_here_doc = 0;
      report_syntax_error (_("maximum here-document count exceeded"));
      reset_parser ();
      exit_shell (last_command_exit_value);
    }
  redir_stack[need_here_doc++] = r;
}

void
gather_here_documents ()
{
  int r = 0;
  while (need_here_doc)
    {
      make_here_document (redir_stack[r++]);
      need_here_doc--;
    }
}

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_count;

#define command_token_position(token) \
  (((token) == ASSIGNMENT_WORD) || \
   ((token) != SEMI_SEMI && reserved_word_acceptable(token)))

#define assignment_acceptable(token) \
  (command_token_position(token) && ((parser_state & PST_CASEPAT) == 0))

/* Check to see if TOKEN is a reserved word and return the token
   value if it is. */
#define CHECK_FOR_RESERVED_WORD(tok) \
  do { \
    if (!dollar_present && !quoted && \
	reserved_word_acceptable (last_read_token)) \
      { \
	int i; \
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++) \
	  if (STREQ (tok, word_token_alist[i].word)) \
	    { \
	      if ((parser_state & PST_CASEPAT) && (word_token_alist[i].token != ESAC)) \
		break; \
	      if (word_token_alist[i].token == TIME && time_command_acceptable () == 0) \
		break; \
	      if (word_token_alist[i].token == ESAC) \
		parser_state &= ~(PST_CASEPAT|PST_CASESTMT); \
	      else if (word_token_alist[i].token == CASE) \
		parser_state |= PST_CASESTMT; \
	      else if (word_token_alist[i].token == COND_END) \
		parser_state &= ~(PST_CONDCMD|PST_CONDEXPR); \
	      else if (word_token_alist[i].token == COND_START) \
		parser_state |= PST_CONDCMD; \
	      else if (word_token_alist[i].token == '{') \
		open_brace_count++; \
	      else if (word_token_alist[i].token == '}' && open_brace_count) \
		open_brace_count--; \
	      return (word_token_alist[i].token); \
	    } \
      } \
  } while (0)

#if defined (ALIAS)

    /* OK, we have a token.  Let's try to alias expand it, if (and only if)
       it's eligible.

       It is eligible for expansion if EXPAND_ALIASES is set, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (parser_state & PST_CASEPAT). */

static char *
mk_alexpansion (s)
     char *s;
{
  int l;
  char *r;

  l = strlen (s);
  r = xmalloc (l + 2);
  strcpy (r, s);
  if (r[l -1] != ' ')
    r[l++] = ' ';
  r[l] = '\0';
  return r;
}

static int
alias_expand_token (tokstr)
     char *tokstr;
{
  char *expanded;
  alias_t *ap;

  if (((parser_state & PST_ALEXPNEXT) || command_token_position (last_read_token)) &&
	(parser_state & PST_CASEPAT) == 0)
    {
      ap = find_alias (tokstr);

      /* Currently expanding this token. */
      if (ap && (ap->flags & AL_BEINGEXPANDED))
	return (NO_EXPANSION);

      /* mk_alexpansion puts an extra space on the end of the alias expansion,
         so the lookahead by the parser works right.  If this gets changed,
         make sure the code in shell_getc that deals with reaching the end of
         an expanded alias is changed with it. */
      expanded = ap ? mk_alexpansion (ap->value) : (char *)NULL;

      if (expanded)
	{
	  push_string (expanded, ap->flags & AL_EXPANDNEXT, ap);
	  return (RE_READ_TOKEN);
	}
      else
	/* This is an eligible token that does not have an expansion. */
	return (NO_EXPANSION);
    }
  return (NO_EXPANSION);
}
#endif /* ALIAS */

static int
time_command_acceptable ()
{
#if defined (COMMAND_TIMING)
  switch (last_read_token)
    {
    case 0:
    case ';':
    case '\n':
    case AND_AND:
    case OR_OR:
    case '&':
    case DO:
    case THEN:
    case ELSE:
    case '{':		/* } */
    case '(':		/* ) */
      return 1;
    default:
      return 0;
    }
#else
  return 0;
#endif /* COMMAND_TIMING */
}

/* Handle special cases of token recognition:
	IN is recognized if the last token was WORD and the token
	before that was FOR or CASE or SELECT.

	DO is recognized if the last token was WORD and the token
	before that was FOR or SELECT.

	ESAC is recognized if the last token caused `esacs_needed_count'
	to be set

	`{' is recognized if the last token as WORD and the token
	before that was FUNCTION, or if we just parsed an arithmetic
	`for' command.

	`}' is recognized if there is an unclosed `{' present.

	`-p' is returned as TIMEOPT if the last read token was TIME.

	']]' is returned as COND_END if the parser is currently parsing
	a conditional expression ((parser_state & PST_CONDEXPR) != 0)

	`time' is returned as TIME if and only if it is immediately
	preceded by one of `;', `\n', `||', `&&', or `&'.
*/

static int
special_case_tokens (tokstr)
     char *tokstr;
{
  if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
      ((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
      ((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
      (tokstr[0] == 'i' && tokstr[1] == 'n' && tokstr[2] == 0))
    {
      if (token_before_that == CASE)
	{
	  parser_state |= PST_CASEPAT;
	  esacs_needed_count++;
	}
      return (IN);
    }

  if (last_read_token == WORD &&
#if defined (SELECT_COMMAND)
      (token_before_that == FOR || token_before_that == SELECT) &&
#else
      (token_before_that == FOR) &&
#endif
      (tokstr[0] == 'd' && tokstr[1] == 'o' && tokstr[2] == '\0'))
    return (DO);

  /* Ditto for ESAC in the CASE case.
     Specifically, this handles "case word in esac", which is a legal
     construct, certainly because someone will pass an empty arg to the
     case construct, and we don't want it to barf.  Of course, we should
     insist that the case construct has at least one pattern in it, but
     the designers disagree. */
  if (esacs_needed_count)
    {
      esacs_needed_count--;
      if (STREQ (tokstr, "esac"))
	{
	  parser_state &= ~PST_CASEPAT;
	  return (ESAC);
	}
    }

  /* The start of a shell function definition. */
  if (parser_state & PST_ALLOWOPNBRC)
    {
      parser_state &= ~PST_ALLOWOPNBRC;
      if (tokstr[0] == '{' && tokstr[1] == '\0')		/* } */
	{
	  open_brace_count++;
	  function_bstart = line_number;
	  return ('{');					/* } */
	}
    }

  /* We allow a `do' after a for ((...)) without an intervening
     list_terminator */
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == 'd' && tokstr[1] == 'o' && !tokstr[2])
    return (DO);
  if (last_read_token == ARITH_FOR_EXPRS && tokstr[0] == '{' && tokstr[1] == '\0')	/* } */
    {
      open_brace_count++;
      return ('{');			/* } */
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && tokstr[0] == '}' && !tokstr[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

#if defined (COMMAND_TIMING)
  /* Handle -p after `time'. */
  if (last_read_token == TIME && tokstr[0] == '-' && tokstr[1] == 'p' && !tokstr[2])
    return (TIMEOPT);
#endif

#if 0
#if defined (COMMAND_TIMING)
  if (STREQ (token, "time") && ((parser_state & PST_CASEPAT) == 0) && time_command_acceptable ())
    return (TIME);
#endif /* COMMAND_TIMING */
#endif

#if defined (COND_COMMAND) /* [[ */
  if ((parser_state & PST_CONDEXPR) && tokstr[0] == ']' && tokstr[1] == ']' && tokstr[2] == '\0')
    return (COND_END);
#endif

  return (-1);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
void
reset_parser ()
{
  dstack.delimiter_depth = 0;	/* No delimiters found so far. */
  open_brace_count = 0;

  parser_state = 0;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  if (pushed_string_list)
    free_string_list ();
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if (shell_input_line)
    {
      free (shell_input_line);
      shell_input_line = (char *)NULL;
      shell_input_line_size = shell_input_line_index = 0;
    }

  FREE (word_desc_to_read);
  word_desc_to_read = (WORD_DESC *)NULL;

  eol_ungetc_lookahead = 0;

  last_read_token = '\n';
  token_to_read = '\n';
}

/* Read the next token.  Command can be READ (normal operation) or
   RESET (to normalize state). */
static int
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */

  if (command == RESET)
    {
      reset_parser ();
      return ('\n');
    }

  if (token_to_read)
    {
      result = token_to_read;
      if (token_to_read == WORD || token_to_read == ASSIGNMENT_WORD)
	{
	  yylval.word = word_desc_to_read;
	  word_desc_to_read = (WORD_DESC *)NULL;
	}
      token_to_read = 0;
      return (result);
    }

#if defined (COND_COMMAND)
  if ((parser_state & (PST_CONDCMD|PST_CONDEXPR)) == PST_CONDCMD)
    {
      cond_lineno = line_number;
      parser_state |= PST_CONDEXPR;
      yylval.command = parse_cond_command ();
      if (cond_token != COND_END)
	{
	  cond_error ();
	  return (-1);
	}
      token_to_read = COND_END;
      parser_state &= ~(PST_CONDEXPR|PST_CONDCMD);
      return (COND_CMD);
    }
#endif

#if defined (ALIAS)
  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
 re_read_token:
#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && shellblank (character))
    ;

  if (character == EOF)
    {
      EOF_Reached = 1;
      return (yacc_EOF);
    }

  if MBTEST(character == '#' && (!interactive || interactive_comments))
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);
      character = '\n';	/* this will take the next if statement and return. */
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	gather_here_documents ();

#if defined (ALIAS)
      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      parser_state &= ~PST_ASSIGNOK;

      return (character);
    }

  if (parser_state & PST_REGEXP)
    goto tokword;

  /* Shell meta-characters. */
  if MBTEST(shellmeta (character) && ((parser_state & PST_DBLPAREN) == 0))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      parser_state &= ~PST_ASSIGNOK;

      peek_char = shell_getc (1);
      if (character == peek_char)
	{
	  switch (character)
	    {
	    case '<':
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	      peek_char = shell_getc (1);
	      if (peek_char == '-')
		return (LESS_LESS_MINUS);
	      else if (peek_char == '<')
		return (LESS_LESS_LESS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '>':
	      return (GREATER_GREATER);

	    case ';':
	      parser_state |= PST_CASEPAT;
#if defined (ALIAS)
	      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
	    case '(':		/* ) */
	      result = parse_dparen (character);
	      if (result == -2)
	        break;
	      else
	        return result;
#endif
	    }
	}
      else if MBTEST(character == '<' && peek_char == '&')
	return (LESS_AND);
      else if MBTEST(character == '>' && peek_char == '&')
	return (GREATER_AND);
      else if MBTEST(character == '<' && peek_char == '>')
	return (LESS_GREATER);
      else if MBTEST(character == '>' && peek_char == '|')
	return (GREATER_BAR);
      else if MBTEST(peek_char == '>' && character == '&')
	return (AND_GREATER);

      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if MBTEST(character == ')' && last_read_token == '(' && token_before_that == WORD)
	{
	  parser_state |= PST_ALLOWOPNBRC;
#if defined (ALIAS)
	  parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	  function_dstart = line_number;
	}

      /* case pattern lists may be preceded by an optional left paren.  If
	 we're not trying to parse a case pattern list, the left paren
	 indicates a subshell. */
      if MBTEST(character == '(' && (parser_state & PST_CASEPAT) == 0) /* ) */
	parser_state |= PST_SUBSHELL;
      /*(*/
      else if MBTEST((parser_state & PST_CASEPAT) && character == ')')
	parser_state &= ~PST_CASEPAT;
      /*(*/
      else if MBTEST((parser_state & PST_SUBSHELL) && character == ')')
	parser_state &= ~PST_SUBSHELL;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if MBTEST(posixly_correct || ((character != '>' && character != '<') || peek_char != '(')) /*)*/
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case.  Also <&N- (dup and close). */
  if MBTEST(character == '-' && (last_read_token == LESS_AND || last_read_token == GREATER_AND))
    return (character);

tokword:
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  result = read_token_word (character);
#if defined (ALIAS)
  if (result == RE_READ_TOKEN)
    goto re_read_token;
#endif
  return result;
}

/*
 * Match a $(...) or other grouping construct.  This has to handle embedded
 * quoted strings ('', ``, "") and nested constructs.  It also must handle
 * reprompting the user, if necessary, after reading a newline, and returning
 * correct error values if it reads EOF.
 */
#define P_FIRSTCLOSE	0x01
#define P_ALLOWESC	0x02
#define P_DQUOTE	0x04
#define P_COMMAND	0x08	/* parsing a command, so look for comments */
#define P_BACKQUOTE	0x10	/* parsing a backquoted command substitution */

static char matched_pair_error;
static char *
parse_matched_pair (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int count, ch, was_dollar, in_comment, check_comment;
  int pass_next_character, backq_backslash, nestlen, ttranslen, start_lineno;
  char *ret, *nestret, *ttrans;
  int retind, retsize, rflags;

/* itrace("parse_matched_pair: open = %c close = %c", open, close); */
  count = 1;
  pass_next_character = backq_backslash = was_dollar = in_comment = 0;
  check_comment = (flags & P_COMMAND) && qc != '`' && qc != '\'' && qc != '"' && (flags & P_DQUOTE) == 0;

  /* RFLAGS is the set of flags we want to pass to recursive calls. */
  rflags = (qc == '"') ? P_DQUOTE : (flags & P_DQUOTE);

  ret = (char *)xmalloc (retsize = 64);
  retind = 0;

  start_lineno = line_number;
  while (count)
    {
      ch = shell_getc (qc != '\'' && pass_next_character == 0 && backq_backslash == 0);

      if (ch == EOF)
	{
	  free (ret);
	  parser_error (start_lineno, _("unexpected EOF while looking for matching `%c'"), close);
	  EOF_Reached = 1;	/* XXX */
	  return (&matched_pair_error);
	}

      /* Possible reprompting. */
      if (ch == '\n' && SHOULD_PROMPT ())
	prompt_again ();

      if (in_comment)
	{
	  /* Add this character. */
	  RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
	  ret[retind++] = ch;

	  if (ch == '\n')
	    in_comment = 0;

	  continue;
	}
      /* Not exactly right yet, should handle shell metacharacters, too.  If
	 any changes are made to this test, make analogous changes to subst.c:
	 extract_delimited_string(). */
      else if MBTEST(check_comment && in_comment == 0 && ch == '#' && (retind == 0 || ret[retind-1] == '\n' || whitespace (ret[retind - 1])))
	in_comment = 1;

      /* last char was backslash inside backquoted command substitution */
      if (backq_backslash)
	{
	  backq_backslash = 0;
	  /* Placeholder for adding special characters */
	}

      if (pass_next_character)		/* last char was backslash */
	{
	  pass_next_character = 0;
	  if (qc != '\'' && ch == '\n')	/* double-quoted \<newline> disappears. */
	    {
	      if (retind > 0) retind--;	/* swallow previously-added backslash */
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  if MBTEST(ch == CTLESC || ch == CTLNUL)
	    ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == CTLESC || ch == CTLNUL)	/* special shell escapes */
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if MBTEST(ch == close)		/* ending delimiter */
	count--;
      /* handle nested ${...} specially. */
      else if MBTEST(open != close && was_dollar && open == '{' && ch == open) /* } */
	count++;
      else if MBTEST(((flags & P_FIRSTCLOSE) == 0) && ch == open)	/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      if (open == '\'')			/* '' inside grouping construct */
	{
	  if MBTEST((flags & P_ALLOWESC) && ch == '\\')
	    pass_next_character++;
#if 0
	  else if MBTEST((flags & P_BACKQUOTE) && ch == '\\')
	    backq_backslash++;
#endif
	  continue;
	}

      if MBTEST(ch == '\\')			/* backslashes */
	pass_next_character++;

      if (open != close)		/* a grouping construct */
	{
	  if MBTEST(shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      if MBTEST(was_dollar && ch == '\'')	/* $'...' inside group */
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, P_ALLOWESC|rflags);
	      else
		nestret = parse_matched_pair (ch, ch, ch, &nestlen, rflags);
	      pop_delimiter (dstack);
	      if (nestret == &matched_pair_error)
		{
		  free (ret);
		  return &matched_pair_error;
		}
	      if MBTEST(was_dollar && ch == '\'' && (extended_quote || (rflags & P_DQUOTE) == 0))
		{
		  /* Translate $'...' here. */
		  ttrans = ansiexpand (nestret, 0, nestlen - 1, &ttranslen);
		  xfree (nestret);

		  if ((rflags & P_DQUOTE) == 0)
		    {
		      nestret = sh_single_quote (ttrans);
		      free (ttrans);
		      nestlen = strlen (nestret);
		    }
		  else
		    {
		      nestret = ttrans;
		      nestlen = ttranslen;
		    }
		  retind -= 2;		/* back up before the $' */
		}
	      else if MBTEST(was_dollar && ch == '"' && (extended_quote || (rflags & P_DQUOTE) == 0))
		{
		  /* Locale expand $"..." here. */
		  ttrans = localeexpand (nestret, 0, nestlen - 1, start_lineno, &ttranslen);
		  xfree (nestret);

		  nestret = sh_mkdoublequoted (ttrans, ttranslen, 0);
		  free (ttrans);
		  nestlen = ttranslen + 2;
		  retind -= 2;		/* back up before the $" */
		}

	      if (nestlen)
		{
		  RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
		  strcpy (ret + retind, nestret);
		  retind += nestlen;
		}
	      FREE (nestret);
	    }
	}
      /* Parse an old-style command substitution within double quotes as a
	 single word. */
      /* XXX - sh and ksh93 don't do this - XXX */
      else if MBTEST(open == '"' && ch == '`')
	{
	  nestret = parse_matched_pair (0, '`', '`', &nestlen, rflags);
add_nestret:
	  if (nestret == &matched_pair_error)
	    {
	      free (ret);
	      return &matched_pair_error;
	    }
	  if (nestlen)
	    {
	      RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
	      strcpy (ret + retind, nestret);
	      retind += nestlen;
	    }
	  FREE (nestret);
	}
#if 0
      else if MBTEST(qc == '`' && (ch == '"' || ch == '\'') && in_comment == 0)
	{
	  /* Add P_BACKQUOTE so backslash quotes the next character and
	     shell_getc does the right thing with \<newline>.  We do this for
	     a measure  of backwards compatibility -- it's not strictly the
	     right POSIX thing. */
	  nestret = parse_matched_pair (0, ch, ch, &nestlen, rflags|P_BACKQUOTE);
	  goto add_nestret;
	}
#endif
      else if MBTEST(open != '`' && was_dollar && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	/* check for $(), $[], or ${} inside quoted string. */
	{
	  if (open == ch)	/* undo previous increment */
	    count--;
	  if (ch == '(')		/* ) */
	    nestret = parse_matched_pair (0, '(', ')', &nestlen, rflags & ~P_DQUOTE);
	  else if (ch == '{')		/* } */
	    nestret = parse_matched_pair (0, '{', '}', &nestlen, P_FIRSTCLOSE|rflags);
	  else if (ch == '[')		/* ] */
	    nestret = parse_matched_pair (0, '[', ']', &nestlen, rflags);

	  goto add_nestret;
	}
      was_dollar = MBTEST(ch == '$');
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
  return ret;
}

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
/* Parse a double-paren construct.  It can be either an arithmetic
   command, an arithmetic `for' command, or a nested subshell.  Returns
   the parsed token, -1 on error, or -2 if we didn't do anything and
   should just go on. */
static int
parse_dparen (c)
     int c;
{
  int cmdtyp, sline;
  char *wval;
  WORD_DESC *wd;

#if defined (ARITH_FOR_COMMAND)
  if (last_read_token == FOR)
    {
      arith_for_lineno = line_number;
      cmdtyp = parse_arith_cmd (&wval, 0);
      if (cmdtyp == 1)
	{
	  wd = alloc_word_desc ();
	  wd->word = wval;
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  return (ARITH_FOR_EXPRS);
	}
      else
	return -1;		/* ERROR */
    }
#endif

#if defined (DPAREN_ARITHMETIC)
  if (reserved_word_acceptable (last_read_token))
    {
      sline = line_number;

      cmdtyp = parse_arith_cmd (&wval, 0);
      if (cmdtyp == 1)	/* arithmetic command */
	{
	  wd = alloc_word_desc ();
	  wd->word = wval;
	  wd->flags = W_QUOTED|W_NOSPLIT|W_NOGLOB|W_DQUOTE;
	  yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
	  return (ARITH_CMD);
	}
      else if (cmdtyp == 0)	/* nested subshell */
	{
	  push_string (wval, 0, (alias_t *)NULL);
	  if ((parser_state & PST_CASEPAT) == 0)
	    parser_state |= PST_SUBSHELL;
	  return (c);
	}
      else			/* ERROR */
	return -1;
    }
#endif

  return -2;			/* XXX */
}

/* We've seen a `(('.  Look for the matching `))'.  If we get it, return 1.
   If not, assume it's a nested subshell for backwards compatibility and
   return 0.  In any case, put the characters we've consumed into a locally-
   allocated buffer and make *ep point to that buffer.  Return -1 on an
   error, for example EOF. */
static int
parse_arith_cmd (ep, adddq)
     char **ep;
     int adddq;
{
  int exp_lineno, rval, c;
  char *ttok, *tokstr;
  int ttoklen;

  exp_lineno = line_number;
  ttok = parse_matched_pair (0, '(', ')', &ttoklen, 0);
  rval = 1;
  if (ttok == &matched_pair_error)
    return -1;
  /* Check that the next character is the closing right paren.  If
     not, this is a syntax error. ( */
  c = shell_getc (0);
  if MBTEST(c != ')')
    rval = 0;

  tokstr = (char *)xmalloc (ttoklen + 4);

  /* if ADDDQ != 0 then (( ... )) -> "..." */
  if (rval == 1 && adddq)	/* arith cmd, add double quotes */
    {
      tokstr[0] = '"';
      strncpy (tokstr + 1, ttok, ttoklen - 1);
      tokstr[ttoklen] = '"';
      tokstr[ttoklen+1] = '\0';
    }
  else if (rval == 1)		/* arith cmd, don't add double quotes */
    {
      strncpy (tokstr, ttok, ttoklen - 1);
      tokstr[ttoklen-1] = '\0';
    }
  else				/* nested subshell */
    {
      tokstr[0] = '(';
      strncpy (tokstr + 1, ttok, ttoklen - 1);
      tokstr[ttoklen] = ')';
      tokstr[ttoklen+1] = c;
      tokstr[ttoklen+2] = '\0';
    }

  *ep = tokstr;
  FREE (ttok);
  return rval;
}
#endif /* DPAREN_ARITHMETIC || ARITH_FOR_COMMAND */

#if defined (COND_COMMAND)
static void
cond_error ()
{
  char *etext;

  if (EOF_Reached && cond_token != COND_ERROR)		/* [[ */
    parser_error (cond_lineno, _("unexpected EOF while looking for `]]'"));
  else if (cond_token != COND_ERROR)
    {
      if (etext = error_token_from_token (cond_token))
	{
	  parser_error (cond_lineno, _("syntax error in conditional expression: unexpected token `%s'"), etext);
	  free (etext);
	}
      else
	parser_error (cond_lineno, _("syntax error in conditional expression"));
    }
}

static COND_COM *
cond_expr ()
{
  return (cond_or ());  
}

static COND_COM *
cond_or ()
{
  COND_COM *l, *r;

  l = cond_and ();
  if (cond_token == OR_OR)
    {
      r = cond_or ();
      l = make_cond_node (COND_OR, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static COND_COM *
cond_and ()
{
  COND_COM *l, *r;

  l = cond_term ();
  if (cond_token == AND_AND)
    {
      r = cond_and ();
      l = make_cond_node (COND_AND, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static int
cond_skip_newlines ()
{
  while ((cond_token = read_token (READ)) == '\n')
    {
      if (SHOULD_PROMPT ())
	prompt_again ();
    }
  return (cond_token);
}

#define COND_RETURN_ERROR() \
  do { cond_token = COND_ERROR; return ((COND_COM *)NULL); } while (0)

static COND_COM *
cond_term ()
{
  WORD_DESC *op;
  COND_COM *term, *tleft, *tright;
  int tok, lineno;
  char *etext;

  /* Read a token.  It can be a left paren, a `!', a unary operator, or a
     word that should be the first argument of a binary operator.  Start by
     skipping newlines, since this is a compound command. */
  tok = cond_skip_newlines ();
  lineno = line_number;
  if (tok == COND_END)
    {
      COND_RETURN_ERROR ();
    }
  else if (tok == '(')
    {
      term = cond_expr ();
      if (cond_token != ')')
	{
	  if (term)
	    dispose_cond_node (term);		/* ( */
	  if (etext = error_token_from_token (cond_token))
	    {
	      parser_error (lineno, _("unexpected token `%s', expected `)'"), etext);
	      free (etext);
	    }
	  else
	    parser_error (lineno, _("expected `)'"));
	  COND_RETURN_ERROR ();
	}
      term = make_cond_node (COND_EXPR, (WORD_DESC *)NULL, term, (COND_COM *)NULL);
      (void)cond_skip_newlines ();
    }
  else if (tok == BANG || (tok == WORD && (yylval.word->word[0] == '!' && yylval.word->word[1] == '\0')))
    {
      if (tok == WORD)
	dispose_word (yylval.word);	/* not needed */
      term = cond_term ();
      if (term)
	term->flags |= CMD_INVERT_RETURN;
    }
  else if (tok == WORD && test_unop (yylval.word->word))
    {
      op = yylval.word;
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	}
      else
	{
	  dispose_word (op);
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected argument `%s' to conditional unary operator"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("unexpected argument to conditional unary operator"));
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else if (tok == WORD)		/* left argument to binary operator */
    {
      /* lhs */
      tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);

      /* binop */
      tok = read_token (READ);
      if (tok == WORD && test_binop (yylval.word->word))
	op = yylval.word;
#if defined (COND_REGEXP)
      else if (tok == WORD && STREQ (yylval.word->word, "=~"))
	{
	  op = yylval.word;
	  parser_state |= PST_REGEXP;
	}
#endif
      else if (tok == '<' || tok == '>')
	op = make_word_from_token (tok);  /* ( */
      /* There should be a check before blindly accepting the `)' that we have
	 seen the opening `('. */
      else if (tok == COND_END || tok == AND_AND || tok == OR_OR || tok == ')')
	{
	  /* Special case.  [[ x ]] is equivalent to [[ -n x ]], just like
	     the test command.  Similarly for [[ x && expr ]] or
	     [[ x || expr ]] or [[ (x) ]]. */
	  op = make_word ("-n");
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	  cond_token = tok;
	  return (term);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected token `%s', conditional binary operator expected"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("conditional binary operator expected"));
	  dispose_cond_node (tleft);
	  COND_RETURN_ERROR ();
	}

      /* rhs */
      tok = read_token (READ);
      parser_state &= ~PST_REGEXP;
      if (tok == WORD)
	{
	  tright = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_BINARY, op, tleft, tright);
	}
      else
	{
	  if (etext = error_token_from_token (tok))
	    {
	      parser_error (line_number, _("unexpected argument `%s' to conditional binary operator"), etext);
	      free (etext);
	    }
	  else
	    parser_error (line_number, _("unexpected argument to conditional binary operator"));
	  dispose_cond_node (tleft);
	  dispose_word (op);
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else
    {
      if (tok < 256)
	parser_error (line_number, _("unexpected token `%c' in conditional command"), tok);
      else if (etext = error_token_from_token (tok))
	{
	  parser_error (line_number, _("unexpected token `%s' in conditional command"), etext);
	  free (etext);
	}
      else
	parser_error (line_number, _("unexpected token %d in conditional command"), tok);
      COND_RETURN_ERROR ();
    }
  return (term);
}      

/* This is kind of bogus -- we slip a mini recursive-descent parser in
   here to handle the conditional statement syntax. */
static COMMAND *
parse_cond_command ()
{
  COND_COM *cexp;

  cexp = cond_expr ();
  return (make_cond_command (cexp));
}
#endif

#if defined (ARRAY_VARS)
/* When this is called, it's guaranteed that we don't care about anything
   in t beyond i.  We do save and restore the chars, though. */
static int
token_is_assignment (t, i)
     char *t;
     int i;
{
  unsigned char c, c1;
  int r;

  c = t[i]; c1 = t[i+1];
  t[i] = '='; t[i+1] = '\0';
  r = assignment (t, (parser_state & PST_COMPASSIGN) != 0);
  t[i] = c; t[i+1] = c1;
  return r;
}

/* XXX - possible changes here for `+=' */
static int
token_is_ident (t, i)
     char *t;
     int i;
{
  unsigned char c;
  int r;

  c = t[i];
  t[i] = '\0';
  r = legal_identifier (t);
  t[i] = c;
  return r;
}
#endif

static int
read_token_word (character)
     int character;
{
  /* The value for YYLVAL when a WORD is read. */
  WORD_DESC *the_word;

  /* Index into the token that we are building. */
  int token_index;

  /* ALL_DIGITS becomes zero when we see a non-digit. */
  int all_digit_token;

  /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
  int dollar_present;

  /* COMPOUND_ASSIGNMENT becomes non-zero if we are parsing a compound
     assignment. */
  int compound_assignment;

  /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
  int quoted;

  /* Non-zero means to ignore the value of the next character, and just
     to add it no matter what. */
 int pass_next_character;

  /* The current delimiting character. */
  int cd;
  int result, peek_char;
  char *ttok, *ttrans;
  int ttoklen, ttranslen;
  intmax_t lvalue;

  if (token_buffer_size < TOKEN_DEFAULT_INITIAL_SIZE)
    token = (char *)xrealloc (token, token_buffer_size = TOKEN_DEFAULT_INITIAL_SIZE);

  token_index = 0;
  all_digit_token = DIGIT (character);
  dollar_present = quoted = pass_next_character = compound_assignment = 0;

  for (;;)
    {
      if (character == EOF)
	goto got_token;

      if (pass_next_character)
	{
	  pass_next_character = 0;
	  goto got_escaped_character;
	}

      cd = current_delimiter (dstack);

      /* Handle backslashes.  Quote lots of things when not inside of
	 double-quotes, quote some things inside of double-quotes. */
      if MBTEST(character == '\\')
	{
	  peek_char = shell_getc (0);

	  /* Backslash-newline is ignored in all cases except
	     when quoted with single quotes. */
	  if (peek_char == '\n')
	    {
	      character = '\n';
	      goto next_character;
	    }
	  else
	    {
	      shell_ungetc (peek_char);

	      /* If the next character is to be quoted, note it now. */
	      if (cd == 0 || cd == '`' ||
		  (cd == '"' && peek_char >= 0 && (sh_syntaxtab[peek_char] & CBSDQUOTE)))
		pass_next_character++;

	      quoted = 1;
	      goto got_character;
	    }
	}

      /* Parse a matched pair of quote characters. */
      if MBTEST(shellquote (character))
	{
	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (character, character, character, &ttoklen, (character == '`') ? P_COMMAND : 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  all_digit_token = 0;
	  quoted = 1;
	  dollar_present |= (character == '"' && strchr (ttok, '$') != 0);
	  FREE (ttok);
	  goto next_character;
	}

#ifdef COND_REGEXP
      /* When parsing a regexp as a single word inside a conditional command,
	 we need to special-case characters special to both the shell and
	 regular expressions.  Right now, that is only '(' and '|'. */ /*)*/
      if MBTEST((parser_state & PST_REGEXP) && (character == '(' || character == '|'))		/*)*/
        {
          if (character == '|')
            goto got_character;

	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  FREE (ttok);
	  dollar_present = all_digit_token = 0;
	  goto next_character;
        }
#endif /* COND_REGEXP */

#ifdef EXTENDED_GLOB
      /* Parse a ksh-style extended pattern matching specification. */
      if MBTEST(extended_glob && PATTERN_CHAR (character))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = all_digit_token = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif /* EXTENDED_GLOB */

      /* If the delimiter character is not single quote, parse some of
	 the shell expansions that must be read as a single word. */
      if (shellexp (character))
	{
	  peek_char = shell_getc (1);
	  /* $(...), <(...), >(...), $((...)), ${...}, and $[...] constructs */
	  if MBTEST(peek_char == '(' || \
		((peek_char == '{' || peek_char == '[') && character == '$'))	/* ) ] } */
	    {
	      if (peek_char == '{')		/* } */
		ttok = parse_matched_pair (cd, '{', '}', &ttoklen, P_FIRSTCLOSE);
	      else if (peek_char == '(')		/* ) */
		{
		  /* XXX - push and pop the `(' as a delimiter for use by
		     the command-oriented-history code.  This way newlines
		     appearing in the $(...) string get added to the
		     history literally rather than causing a possibly-
		     incorrect `;' to be added. ) */
		  push_delimiter (dstack, peek_char);
		  ttok = parse_matched_pair (cd, '(', ')', &ttoklen, P_COMMAND);
		  pop_delimiter (dstack);
		}
	      else
		ttok = parse_matched_pair (cd, '[', ']', &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
	  else if MBTEST(character == '$' && (peek_char == '\'' || peek_char == '"'))
	    {
	      int first_line;

	      first_line = line_number;
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (peek_char, peek_char, peek_char,
					 &ttoklen,
					 (peek_char == '\'') ? P_ALLOWESC : 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;
	      if (peek_char == '\'')
		{
		  ttrans = ansiexpand (ttok, 0, ttoklen - 1, &ttranslen);
		  free (ttok);

		  /* Insert the single quotes and correctly quote any
		     embedded single quotes (allowed because P_ALLOWESC was
		     passed to parse_matched_pair). */
		  ttok = sh_single_quote (ttrans);
		  free (ttrans);
		  ttranslen = strlen (ttok);
		  ttrans = ttok;
		}
	      else
		{
		  /* Try to locale-expand the converted string. */
		  ttrans = localeexpand (ttok, 0, ttoklen - 1, first_line, &ttranslen);
		  free (ttok);

		  /* Add the double quotes back */
		  ttok = sh_mkdoublequoted (ttrans, ttranslen, 0);
		  free (ttrans);
		  ttranslen += 2;
		  ttrans = ttok;
		}

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttranslen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttrans);
	      token_index += ttranslen;
	      FREE (ttrans);
	      quoted = 1;
	      all_digit_token = 0;
	      goto next_character;
	    }
	  /* This could eventually be extended to recognize all of the
	     shell's single-character parameter expansions, and set flags.*/
	  else if MBTEST(character == '$' && peek_char == '$')
	    {
	      ttok = (char *)xmalloc (3);
	      ttok[0] = ttok[1] = '$';
	      ttok[2] = '\0';
	      RESIZE_MALLOCED_BUFFER (token, token_index, 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttok);
	      token_index += 2;
	      dollar_present = 1;
	      all_digit_token = 0;
	      FREE (ttok);
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible array subscript assignment; match [...] */
      else if MBTEST(character == '[' && token_index > 0 && assignment_acceptable (last_read_token) && token_is_ident (token, token_index))	/* ] */
        {
	  ttok = parse_matched_pair (cd, '[', ']', &ttoklen, 0);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size,
				  TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  FREE (ttok);
	  all_digit_token = 0;
	  goto next_character;
        }
      /* Identify possible compound array variable assignment. */
      else if MBTEST(character == '=' && token_index > 0 && (assignment_acceptable (last_read_token) || (parser_state & PST_ASSIGNOK)) && token_is_assignment (token, token_index))
	{
	  peek_char = shell_getc (1);
	  if MBTEST(peek_char == '(')		/* ) */
	    {
	      ttok = parse_compound_assignment (&ttoklen);

	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 4,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);

	      token[token_index++] = '=';
	      token[token_index++] = '(';
	      if (ttok)
		{
		  strcpy (token + token_index, ttok);
		  token_index += ttoklen;
		}
	      token[token_index++] = ')';
	      FREE (ttok);
	      all_digit_token = 0;
	      compound_assignment = 1;
#if 1
	      goto next_character;
#else
	      goto got_token;		/* ksh93 seems to do this */
#endif
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif

      /* When not parsing a multi-character word construct, shell meta-
	 characters break words. */
      if MBTEST(shellbreak (character))
	{
	  shell_ungetc (character);
	  goto got_token;
	}

    got_character:

      if (character == CTLESC || character == CTLNUL)
	token[token_index++] = CTLESC;

    got_escaped_character:

      all_digit_token &= DIGIT (character);
      dollar_present |= character == '$';

      token[token_index++] = character;

      RESIZE_MALLOCED_BUFFER (token, token_index, 1, token_buffer_size,
			      TOKEN_DEFAULT_GROW_SIZE);

    next_character:
      if (character == '\n' && SHOULD_PROMPT ())
	prompt_again ();

      /* We want to remove quoted newlines (that is, a \<newline> pair)
	 unless we are within single quotes or pass_next_character is
	 set (the shell equivalent of literal-next). */
      cd = current_delimiter (dstack);
      character = shell_getc (cd != '\'' && pass_next_character == 0);
    }	/* end for (;;) */

got_token:

  token[token_index] = '\0';

  /* Check to see what thing we should return.  If the last_read_token
     is a `<', or a `&', or the character which ended this token is
     a '>' or '<', then, and ONLY then, is this input token a NUMBER.
     Otherwise, it is just a word, and should be returned as such. */
  if MBTEST(all_digit_token && (character == '<' || character == '>' || \
		    last_read_token == LESS_AND || \
		    last_read_token == GREATER_AND))
      {
	if (legal_number (token, &lvalue) && (int)lvalue == lvalue)
	  yylval.number = lvalue;
	else
	  yylval.number = -1;
	return (NUMBER);
      }

  /* Check for special case tokens. */
  result = (last_shell_getc_is_singlebyte) ? special_case_tokens (token) : -1;
  if (result >= 0)
    return result;

#if defined (ALIAS)
  /* Posix.2 does not allow reserved words to be aliased, so check for all
     of them, including special cases, before expanding the current token
     as an alias. */
  if MBTEST(posixly_correct)
    CHECK_FOR_RESERVED_WORD (token);

  /* Aliases are expanded iff EXPAND_ALIASES is non-zero, and quoting
     inhibits alias expansion. */
  if (expand_aliases && quoted == 0)
    {
      result = alias_expand_token (token);
      if (result == RE_READ_TOKEN)
	return (RE_READ_TOKEN);
      else if (result == NO_EXPANSION)
	parser_state &= ~PST_ALEXPNEXT;
    }

  /* If not in Posix.2 mode, check for reserved words after alias
     expansion. */
  if MBTEST(posixly_correct == 0)
#endif
    CHECK_FOR_RESERVED_WORD (token);

  the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
  the_word->word = (char *)xmalloc (1 + token_index);
  the_word->flags = 0;
  strcpy (the_word->word, token);
  if (dollar_present)
    the_word->flags |= W_HASDOLLAR;
  if (quoted)
    the_word->flags |= W_QUOTED;		/*(*/
  if (compound_assignment && token[token_index-1] == ')')
    the_word->flags |= W_COMPASSIGN;
  /* A word is an assignment if it appears at the beginning of a
     simple command, or after another assignment word.  This is
     context-dependent, so it cannot be handled in the grammar. */
  if (assignment (token, (parser_state & PST_COMPASSIGN) != 0))
    {
      the_word->flags |= W_ASSIGNMENT;
      /* Don't perform word splitting on assignment statements. */
      if (assignment_acceptable (last_read_token) || (parser_state & PST_COMPASSIGN) != 0)
	the_word->flags |= W_NOSPLIT;
    }

  if (command_token_position (last_read_token))
    {
      struct builtin *b;
      b = builtin_address_internal (token, 0);
      if (b && (b->flags & ASSIGNMENT_BUILTIN))
	parser_state |= PST_ASSIGNOK;
      else if (STREQ (token, "eval") || STREQ (token, "let"))
	parser_state |= PST_ASSIGNOK;
    }

  yylval.word = the_word;

  result = ((the_word->flags & (W_ASSIGNMENT|W_NOSPLIT)) == (W_ASSIGNMENT|W_NOSPLIT))
		? ASSIGNMENT_WORD : WORD;

  switch (last_read_token)
    {
    case FUNCTION:
      parser_state |= PST_ALLOWOPNBRC;
      function_dstart = line_number;
      break;
    case CASE:
    case SELECT:
    case FOR:
      if (word_top < MAX_CASE_NEST)
	word_top++;
      word_lineno[word_top] = line_number;
      break;
    }

  return (result);
}

/* Return 1 if TOKSYM is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (toksym)
     int toksym;
{
  switch (toksym)
    {
    case '\n':
    case ';':
    case '(':
    case ')':
    case '|':
    case '&':
    case '{':
    case '}':		/* XXX */
    case AND_AND:
    case BANG:
    case DO:
    case DONE:
    case ELIF:
    case ELSE:
    case ESAC:
    case FI:
    case IF:
    case OR_OR:
    case SEMI_SEMI:
    case THEN:
    case TIME:
    case TIMEOPT:
    case UNTIL:
    case WHILE:
    case 0:
      return 1;
    default:
      return 0;
    }
}
    
/* Return the index of TOKEN in the alist of reserved words, or -1 if
   TOKEN is not a shell reserved word. */
int
find_reserved_word (tokstr)
     char *tokstr;
{
  int i;
  for (i = 0; word_token_alist[i].word; i++)
    if (STREQ (tokstr, word_token_alist[i].word))
      return i;
  return -1;
}

#if 0
#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
static void
reset_readline_prompt ()
{
  char *temp_prompt;

  if (prompt_string_pointer)
    {
      temp_prompt = (*prompt_string_pointer)
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

      if (temp_prompt == 0)
	{
	  temp_prompt = (char *)xmalloc (1);
	  temp_prompt[0] = '\0';
	}

      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
}
#endif /* READLINE */
#endif /* 0 */

#if defined (HISTORY)
/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
static int no_semi_successors[] = {
  '\n', '{', '(', ')', ';', '&', '|',
  CASE, DO, ELSE, IF, SEMI_SEMI, THEN, UNTIL, WHILE, AND_AND, OR_OR, IN,
  0
};

/* If we are not within a delimited expression, try to be smart
   about which separators can be semi-colons and which must be
   newlines.  Returns the string that should be added into the
   history entry. */
char *
history_delimiting_chars ()
{
  register int i;

  if (dstack.delimiter_depth != 0)
    return ("\n");
    
  /* First, handle some special cases. */
  /*(*/
  /* If we just read `()', assume it's a function definition, and don't
     add a semicolon.  If the token before the `)' was not `(', and we're
     not in the midst of parsing a case statement, assume it's a
     parenthesized command and add the semicolon. */
  /*)(*/
  if (token_before_that == ')')
    {
      if (two_tokens_ago == '(')	/*)*/	/* function def */
	return " ";
      /* This does not work for subshells inside case statement
	 command lists.  It's a suboptimal solution. */
      else if (parser_state & PST_CASESTMT)	/* case statement pattern */
	return " ";
      else	
	return "; ";				/* (...) subshell */
    }
  else if (token_before_that == WORD && two_tokens_ago == FUNCTION)
    return " ";		/* function def using `function name' without `()' */

  else if (token_before_that == WORD && two_tokens_ago == FOR)
    {
      /* Tricky.  `for i\nin ...' should not have a semicolon, but
	 `for i\ndo ...' should.  We do what we can. */
      for (i = shell_input_line_index; whitespace(shell_input_line[i]); i++)
	;
      if (shell_input_line[i] && shell_input_line[i] == 'i' && shell_input_line[i+1] == 'n')
	return " ";
      return ";";
    }
  else if (two_tokens_ago == CASE && token_before_that == WORD && (parser_state & PST_CASESTMT))
    return " ";

  for (i = 0; no_semi_successors[i]; i++)
    {
      if (token_before_that == no_semi_successors[i])
	return (" ");
    }

  return ("; ");
}
#endif /* HISTORY */

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
static void
prompt_again ()
{
  char *temp_prompt;

  if (interactive == 0 || expanding_alias())	/* XXX */
    return;

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  temp_prompt = *prompt_string_pointer
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

  if (temp_prompt == 0)
    {
      temp_prompt = (char *)xmalloc (1);
      temp_prompt[0] = '\0';
    }

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

#if defined (READLINE)
  if (!no_line_editing)
    {
      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
  else
#endif	/* READLINE */
    {
      FREE (current_decoded_prompt);
      current_decoded_prompt = temp_prompt;
    }
}

int
get_current_prompt_level ()
{
  return ((current_prompt_string && current_prompt_string == ps2_prompt) ? 2 : 1);
}

void
set_current_prompt_level (x)
     int x;
{
  prompt_string_pointer = (x == 2) ? &ps2_prompt : &ps1_prompt;
  current_prompt_string = *prompt_string_pointer;
}
      
static void
print_prompt ()
{
  fprintf (stderr, "%s", current_decoded_prompt);
  fflush (stderr);
}

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:

	\a	bell (ascii 07)
	\d	the date in Day Mon Date format
	\e	escape (ascii 033)
	\h	the hostname up to the first `.'
	\H	the hostname
	\j	the number of active jobs
	\l	the basename of the shell's tty device name
	\n	CRLF
	\r	CR
	\s	the name of the shell
	\t	the time in 24-hour hh:mm:ss format
	\T	the time in 12-hour hh:mm:ss format
	\@	the time in 12-hour hh:mm am/pm format
	\A	the time in 24-hour hh:mm format
	\D{fmt}	the result of passing FMT to strftime(3)
	\u	your username
	\v	the version of bash (e.g., 2.00)
	\V	the release of bash, version + patchlevel (e.g., 2.00.0)
	\w	the current working directory
	\W	the last element of $PWD
	\!	the history number of this command
	\#	the command number of this command
	\$	a $ or a # if you are root
	\nnn	character code nnn in octal
	\\	a backslash
	\[	begin a sequence of non-printing chars
	\]	end a sequence of non-printing chars
*/
#define PROMPT_GROWTH 48
char *
decode_prompt_string (string)
     char *string;
{
  WORD_LIST *list;
  char *result, *t;
  struct dstack save_dstack;
  int last_exit_value;
#if defined (PROMPT_STRING_DECODE)
  int result_size, result_index;
  int c, n, i;
  char *temp, octal_string[4];
  struct tm *tm;  
  time_t the_time;
  char timebuf[128];
  char *timefmt;

  result = (char *)xmalloc (result_size = PROMPT_GROWTH);
  result[result_index = 0] = 0;
  temp = (char *)NULL;

  while (c = *string++)
    {
      if (posixly_correct && c == '!')
	{
	  if (*string == '!')
	    {
	      temp = savestring ("!");
	      goto add_string;
	    }
	  else
	    {
#if !defined (HISTORY)
		temp = savestring ("1");
#else /* HISTORY */
		temp = itos (history_number ());
#endif /* HISTORY */
		string--;	/* add_string increments string again. */
		goto add_string;
	    }
	}
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      strncpy (octal_string, string, 3);
	      octal_string[3] = '\0';

	      n = read_octal (octal_string);
	      temp = (char *)xmalloc (3);

	      if (n == CTLESC || n == CTLNUL)
		{
		  temp[0] = CTLESC;
		  temp[1] = n;
		  temp[2] = '\0';
		}
	      else if (n == -1)
		{
		  temp[0] = '\\';
		  temp[1] = '\0';
		}
	      else
		{
		  temp[0] = n;
		  temp[1] = '\0';
		}

	      for (c = 0; n != -1 && c < 3 && ISOCTAL (*string); c++)
		string++;

	      c = 0;		/* tested at add_string: */
	      goto add_string;

	    case 'd':
	    case 't':
	    case 'T':
	    case '@':
	    case 'A':
	      /* Make the current time/date into a string. */
	      (void) time (&the_time);
	      tm = localtime (&the_time);

	      if (c == 'd')
		n = strftime (timebuf, sizeof (timebuf), "%a %b %d", tm);
	      else if (c == 't')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M:%S", tm);
	      else if (c == 'T')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M:%S", tm);
	      else if (c == '@')
		n = strftime (timebuf, sizeof (timebuf), "%I:%M %p", tm);
	      else if (c == 'A')
		n = strftime (timebuf, sizeof (timebuf), "%H:%M", tm);

	      if (n == 0)
		timebuf[0] = '\0';
	      else
		timebuf[sizeof(timebuf) - 1] = '\0';

	      temp = savestring (timebuf);
	      goto add_string;

	    case 'D':		/* strftime format */
	      if (string[1] != '{')		/* } */
		goto not_escape;

	      (void) time (&the_time);
	      tm = localtime (&the_time);
	      string += 2;			/* skip { */
	      timefmt = xmalloc (strlen (string) + 3);
	      for (t = timefmt; *string && *string != '}'; )
		*t++ = *string++;
	      *t = '\0';
	      c = *string;	/* tested at add_string */
	      if (timefmt[0] == '\0')
		{
		  timefmt[0] = '%';
		  timefmt[1] = 'X';	/* locale-specific current time */
		  timefmt[2] = '\0';
		}
	      n = strftime (timebuf, sizeof (timebuf), timefmt, tm);
	      free (timefmt);

	      if (n == 0)
		timebuf[0] = '\0';
	      else
		timebuf[sizeof(timebuf) - 1] = '\0';

	      if (promptvars || posixly_correct)
		/* Make sure that expand_prompt_string is called with a
		   second argument of Q_DOUBLE_QUOTES if we use this
		   function here. */
		temp = sh_backslash_quote_for_double_quotes (timebuf);
	      else
		temp = savestring (timebuf);
	      goto add_string;
	      
	    case 'n':
	      temp = (char *)xmalloc (3);
	      temp[0] = no_line_editing ? '\n' : '\r';
	      temp[1] = no_line_editing ? '\0' : '\n';
	      temp[2] = '\0';
	      goto add_string;

	    case 's':
	      temp = base_pathname (shell_name);
	      temp = savestring (temp);
	      goto add_string;

	    case 'v':
	    case 'V':
	      temp = (char *)xmalloc (16);
	      if (c == 'v')
		strcpy (temp, dist_version);
	      else
		sprintf (temp, "%s.%d", dist_version, patch_level);
	      goto add_string;

	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more efficient. */
		char t_string[PATH_MAX];
		int tlen;

		temp = get_string_value ("PWD");

		if (temp == 0)
		  {
		    if (getcwd (t_string, sizeof(t_string)) == 0)
		      {
			t_string[0] = '.';
			tlen = 1;
		      }
		    else
		      tlen = strlen (t_string);
		  }
		else
		  {
		    tlen = sizeof (t_string) - 1;
		    strncpy (t_string, temp, tlen);
		  }
		t_string[tlen] = '\0';

#define ROOT_PATH(x)	((x)[0] == '/' && (x)[1] == 0)
#define DOUBLE_SLASH_ROOT(x)	((x)[0] == '/' && (x)[1] == '/' && (x)[2] == 0)
		/* Abbreviate \W as ~ if $PWD == $HOME */
		if (c == 'W' && (((t = get_string_value ("HOME")) == 0) || STREQ (t, t_string) == 0))
		  {
		    if (ROOT_PATH (t_string) == 0 && DOUBLE_SLASH_ROOT (t_string) == 0)
		      {
			t = strrchr (t_string, '/');
			if (t)
			  strcpy (t_string, t + 1);
		      }
		  }
#undef ROOT_PATH
#undef DOUBLE_SLASH_ROOT
		else
		  /* polite_directory_format is guaranteed to return a string
		     no longer than PATH_MAX - 1 characters. */
		  strcpy (t_string, polite_directory_format (t_string));

		/* If we're going to be expanding the prompt string later,
		   quote the directory name. */
		if (promptvars || posixly_correct)
		  /* Make sure that expand_prompt_string is called with a
		     second argument of Q_DOUBLE_QUOTES if we use this
		     function here. */
		  temp = sh_backslash_quote_for_double_quotes (t_string);
		else
		  temp = savestring (t_string);

		goto add_string;
	      }

	    case 'u':
	      if (current_user.user_name == 0)
		get_current_user_info ();
	      temp = savestring (current_user.user_name);
	      goto add_string;

	    case 'h':
	    case 'H':
	      temp = savestring (current_host_name);
	      if (c == 'h' && (t = (char *)strchr (temp, '.')))
		*t = '\0';
	      goto add_string;

	    case '#':
	      temp = itos (current_command_number);
	      goto add_string;

	    case '!':
#if !defined (HISTORY)
	      temp = savestring ("1");
#else /* HISTORY */
	      temp = itos (history_number ());
#endif /* HISTORY */
	      goto add_string;

	    case '$':
	      t = temp = (char *)xmalloc (3);
	      if ((promptvars || posixly_correct) && (current_user.euid != 0))
		*t++ = '\\';
	      *t++ = current_user.euid == 0 ? '#' : '$';
	      *t = '\0';
	      goto add_string;

	    case 'j':
	      temp = itos (count_all_jobs ());
	      goto add_string;

	    case 'l':
#if defined (HAVE_TTYNAME)
	      temp = (char *)ttyname (fileno (stdin));
	      t = temp ? base_pathname (temp) : "tty";
	      temp = savestring (t);
#else
	      temp = savestring ("tty");
#endif /* !HAVE_TTYNAME */
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      if (no_line_editing)
		{
		  string++;
		  break;
		}
	      temp = (char *)xmalloc (3);
	      n = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      i = 0;
	      if (n == CTLESC || n == CTLNUL)
		temp[i++] = CTLESC;
	      temp[i++] = n;
	      temp[i] = '\0';
	      goto add_string;
#endif /* READLINE */

	    case '\\':
	    case 'a':
	    case 'e':
	    case 'r':
	      temp = (char *)xmalloc (2);
	      if (c == 'a')
		temp[0] = '\07';
	      else if (c == 'e')
		temp[0] = '\033';
	      else if (c == 'r')
		temp[0] = '\r';
	      else			/* (c == '\\') */
	        temp[0] = c;
	      temp[1] = '\0';
	      goto add_string;

	    default:
not_escape:
	      temp = (char *)xmalloc (3);
	      temp[0] = '\\';
	      temp[1] = c;
	      temp[2] = '\0';

	    add_string:
	      if (c)
		string++;
	      result =
		sub_append_string (temp, result, &result_index, &result_size);
	      temp = (char *)NULL; /* Freed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  RESIZE_MALLOCED_BUFFER (result, result_index, 3, result_size, PROMPT_GROWTH);
	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }
#else /* !PROMPT_STRING_DECODE */
  result = savestring (string);
#endif /* !PROMPT_STRING_DECODE */

  /* Save the delimiter stack and point `dstack' to temp space so any
     command substitutions in the prompt string won't result in screwing
     up the parser's quoting state. */
  save_dstack = dstack;
  dstack = temp_dstack;
  dstack.delimiter_depth = 0;

  /* Perform variable and parameter expansion and command substitution on
     the prompt string. */
  if (promptvars || posixly_correct)
    {
      last_exit_value = last_command_exit_value;
      list = expand_prompt_string (result, Q_DOUBLE_QUOTES, 0);
      free (result);
      result = string_list (list);
      dispose_words (list);
      last_command_exit_value = last_exit_value;
    }
  else
    {
      t = dequote_string (result);
      free (result);
      result = t;
    }

  dstack = save_dstack;

  return (result);
}

/************************************************
 *						*
 *		ERROR HANDLING			*
 *						*
 ************************************************/

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
int
yyerror (msg)
     const char *msg;
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
  return (0);
}

static char *
error_token_from_token (tok)
     int tok;
{
  char *t;

  if (t = find_token_in_alist (tok, word_token_alist, 0))
    return t;

  if (t = find_token_in_alist (tok, other_token_alist, 0))
    return t;

  t = (char *)NULL;
  /* This stuff is dicy and needs closer inspection */
  switch (current_token)
    {
    case WORD:
    case ASSIGNMENT_WORD:
      if (yylval.word)
	t = savestring (yylval.word->word);
      break;
    case NUMBER:
      t = itos (yylval.number);
      break;
    case ARITH_CMD:
      if (yylval.word_list)
        t = string_list (yylval.word_list);
      break;
    case ARITH_FOR_EXPRS:
      if (yylval.word_list)
	t = string_list_internal (yylval.word_list, " ; ");
      break;
    case COND_CMD:
      t = (char *)NULL;		/* punt */
      break;
    }

  return t;
}

static char *
error_token_from_text ()
{
  char *msg, *t;
  int token_end, i;

  t = shell_input_line;
  i = shell_input_line_index;
  token_end = 0;
  msg = (char *)NULL;

  if (i && t[i] == '\0')
    i--;

  while (i && (whitespace (t[i]) || t[i] == '\n'))
    i--;

  if (i)
    token_end = i + 1;

  while (i && (member (t[i], " \n\t;|&") == 0))
    i--;

  while (i != token_end && (whitespace (t[i]) || t[i] == '\n'))
    i++;

  /* Return our idea of the offending token. */
  if (token_end || (i == 0 && token_end == 0))
    {
      if (token_end)
	msg = substring (t, i, token_end);
      else	/* one-character token */
	{
	  msg = (char *)xmalloc (2);
	  msg[0] = t[i];
	  msg[1] = '\0';
	}
    }

  return (msg);
}

static void
print_offending_line ()
{
  char *msg;
  int token_end;

  msg = savestring (shell_input_line);
  token_end = strlen (msg);
  while (token_end && msg[token_end - 1] == '\n')
    msg[--token_end] = '\0';

  parser_error (line_number, "`%s'", msg);
  free (msg);
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  char *msg;

  if (message)
    {
      parser_error (line_number, "%s", message);
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If the line of input we're reading is not null, try to find the
     objectionable token.  First, try to figure out what token the
     parser's complaining about by looking at current_token. */
  if (current_token != 0 && EOF_Reached == 0 && (msg = error_token_from_token (current_token)))
    {
      parser_error (line_number, _("syntax error near unexpected token `%s'"), msg);
      free (msg);

      if (interactive == 0)
	print_offending_line ();

      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If looking at the current token doesn't prove fruitful, try to find the
     offending token by analyzing the text of the input line near the current
     input line index and report what we find. */
  if (shell_input_line && *shell_input_line)
    {
      msg = error_token_from_text ();
      if (msg)
	{
	  parser_error (line_number, _("syntax error near `%s'"), msg);
	  free (msg);
	}

      /* If not interactive, print the line containing the error. */
      if (interactive == 0)
        print_offending_line ();
    }
  else
    {
      msg = EOF_Reached ? _("syntax error: unexpected end of file") : _("syntax error");
      parser_error (line_number, "%s", msg);
      /* When the shell is interactive, this file uses EOF_Reached
	 only for error reporting.  Other mechanisms are used to
	 decide whether or not to exit. */
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
    }

  last_command_exit_value = EX_USAGE;
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command.) */
static void
discard_parser_constructs (error_p)
     int error_p;
{
}

/************************************************
 *						*
 *		EOF HANDLING			*
 *						*
 ************************************************/

/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* A flag denoting whether or not ignoreeof is set. */
int ignoreeof = 0;

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
static void
handle_eof_input_unit ()
{
  if (interactive)
    {
      /* shell.c may use this to decide whether or not to write out the
	 history, among other things.  We use it only for error reporting
	 in this file. */
      if (EOF_Reached)
	EOF_Reached = 0;

      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (ignoreeof)
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, _("Use \"%s\" to leave the shell.\n"),
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      /* Reset the parsing state. */
	      last_read_token = current_token = '\n';
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again ();
	      return;
	    }
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}

/************************************************
 *						*
 *	STRING PARSING FUNCTIONS		*
 *						*
 ************************************************/

/* It's very important that these two functions treat the characters
   between ( and ) identically. */

static WORD_LIST parse_string_error;

/* Take a string and run it through the shell parser, returning the
   resultant word list.  Used by compound array assignment. */
WORD_LIST *
parse_string_to_word_list (s, flags, whom)
     char *s;
     int flags;
     const char *whom;
{
  WORD_LIST *wl;
  int tok, orig_current_token, orig_line_number, orig_input_terminator;
  int orig_line_count;
  int old_echo_input, old_expand_aliases;
#if defined (HISTORY)
  int old_remember_on_history, old_history_expansion_inhibited;
#endif

#if defined (HISTORY)
  old_remember_on_history = remember_on_history;
#  if defined (BANG_HISTORY)
  old_history_expansion_inhibited = history_expansion_inhibited;
#  endif
  bash_history_disable ();
#endif

  orig_line_number = line_number;
  orig_line_count = current_command_line_count;
  orig_input_terminator = shell_input_line_terminator;
  old_echo_input = echo_input_at_read;
  old_expand_aliases = expand_aliases;

  push_stream (1);
  last_read_token = WORD;		/* WORD to allow reserved words here */
  current_command_line_count = 0;
  echo_input_at_read = expand_aliases = 0;

  with_input_from_string (s, whom);
  wl = (WORD_LIST *)NULL;

  if (flags & 1)
    parser_state |= PST_COMPASSIGN;

  while ((tok = read_token (READ)) != yacc_EOF)
    {
      if (tok == '\n' && *bash_input.location.string == '\0')
	break;
      if (tok == '\n')		/* Allow newlines in compound assignments */
	continue;
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  line_number = orig_line_number + line_number - 1;
	  orig_current_token = current_token;
	  current_token = tok;
	  yyerror (NULL);	/* does the right thing */
	  current_token = orig_current_token;
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }
  
  last_read_token = '\n';
  pop_stream ();

#if defined (HISTORY)
  remember_on_history = old_remember_on_history;
#  if defined (BANG_HISTORY)
  history_expansion_inhibited = old_history_expansion_inhibited;
#  endif /* BANG_HISTORY */
#endif /* HISTORY */

  echo_input_at_read = old_echo_input;
  expand_aliases = old_expand_aliases;

  current_command_line_count = orig_line_count;
  shell_input_line_terminator = orig_input_terminator;

  if (flags & 1)
    parser_state &= ~PST_COMPASSIGN;

  if (wl == &parse_string_error)
    {
      last_command_exit_value = EXECUTION_FAILURE;
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  return (REVERSE_LIST (wl, WORD_LIST *));
}

static char *
parse_compound_assignment (retlenp)
     int *retlenp;
{
  WORD_LIST *wl, *rl;
  int tok, orig_line_number, orig_token_size, orig_last_token, assignok;
  char *saved_token, *ret;

  saved_token = token;
  orig_token_size = token_buffer_size;
  orig_line_number = line_number;
  orig_last_token = last_read_token;

  last_read_token = WORD;	/* WORD to allow reserved words here */

  token = (char *)NULL;
  token_buffer_size = 0;

  assignok = parser_state&PST_ASSIGNOK;		/* XXX */

  wl = (WORD_LIST *)NULL;	/* ( */
  parser_state |= PST_COMPASSIGN;

  while ((tok = read_token (READ)) != ')')
    {
      if (tok == '\n')			/* Allow newlines in compound assignments */
	{
	  if (SHOULD_PROMPT ())
	    prompt_again ();
	  continue;
	}
      if (tok != WORD && tok != ASSIGNMENT_WORD)
	{
	  current_token = tok;	/* for error reporting */
	  if (tok == yacc_EOF)	/* ( */
	    parser_error (orig_line_number, _("unexpected EOF while looking for matching `)'"));
	  else
	    yyerror(NULL);	/* does the right thing */
	  if (wl)
	    dispose_words (wl);
	  wl = &parse_string_error;
	  break;
	}
      wl = make_word_list (yylval.word, wl);
    }

  FREE (token);
  token = saved_token;
  token_buffer_size = orig_token_size;

  parser_state &= ~PST_COMPASSIGN;

  if (wl == &parse_string_error)
    {
      last_command_exit_value = EXECUTION_FAILURE;
      last_read_token = '\n';	/* XXX */
      if (interactive_shell == 0 && posixly_correct)
	jump_to_top_level (FORCE_EOF);
      else
	jump_to_top_level (DISCARD);
    }

  last_read_token = orig_last_token;		/* XXX - was WORD? */

  if (wl)
    {
      rl = REVERSE_LIST (wl, WORD_LIST *);
      ret = string_list (rl);
      dispose_words (rl);
    }
  else
    ret = (char *)NULL;

  if (retlenp)
    *retlenp = (ret && *ret) ? strlen (ret) : 0;

  if (assignok)
    parser_state |= PST_ASSIGNOK;

  return ret;
}

/************************************************
 *						*
 *   SAVING AND RESTORING PARTIAL PARSE STATE   *
 *						*
 ************************************************/

sh_parser_state_t *
save_parser_state (ps)
     sh_parser_state_t *ps;
{
#if defined (ARRAY_VARS)
  SHELL_VAR *v;
#endif

  if (ps == 0)
    ps = (sh_parser_state_t *)xmalloc (sizeof (sh_parser_state_t));
  if (ps == 0)
    return ((sh_parser_state_t *)NULL);

  ps->parser_state = parser_state;
  ps->token_state = save_token_state ();

  ps->input_line_terminator = shell_input_line_terminator;
  ps->eof_encountered = eof_encountered;

  ps->current_command_line_count = current_command_line_count;

#if defined (HISTORY)
  ps->remember_on_history = remember_on_history;
#  if defined (BANG_HISTORY)
  ps->history_expansion_inhibited = history_expansion_inhibited;
#  endif
#endif

  ps->last_command_exit_value = last_command_exit_value;
#if defined (ARRAY_VARS)
  v = find_variable ("PIPESTATUS");
  if (v && array_p (v) && array_cell (v))
    ps->pipestatus = array_copy (array_cell (v));
  else
    ps->pipestatus = (ARRAY *)NULL;
#endif
    
  ps->last_shell_builtin = last_shell_builtin;
  ps->this_shell_builtin = this_shell_builtin;

  ps->expand_aliases = expand_aliases;
  ps->echo_input_at_read = echo_input_at_read;

  return (ps);
}

void
restore_parser_state (ps)
     sh_parser_state_t *ps;
{
#if defined (ARRAY_VARS)
  SHELL_VAR *v;
#endif

  if (ps == 0)
    return;

  parser_state = ps->parser_state;
  if (ps->token_state)
    {
      restore_token_state (ps->token_state);
      free (ps->token_state);
    }

  shell_input_line_terminator = ps->input_line_terminator;
  eof_encountered = ps->eof_encountered;

  current_command_line_count = ps->current_command_line_count;

#if defined (HISTORY)
  remember_on_history = ps->remember_on_history;
#  if defined (BANG_HISTORY)
  history_expansion_inhibited = ps->history_expansion_inhibited;
#  endif
#endif

  last_command_exit_value = ps->last_command_exit_value;
#if defined (ARRAY_VARS)
  v = find_variable ("PIPESTATUS");
  if (v && array_p (v) && array_cell (v))
    {
      array_dispose (array_cell (v));
      var_setarray (v, ps->pipestatus);
    }
#endif

  last_shell_builtin = ps->last_shell_builtin;
  this_shell_builtin = ps->this_shell_builtin;

  expand_aliases = ps->expand_aliases;
  echo_input_at_read = ps->echo_input_at_read;
}

/************************************************
 *						*
 *	MULTIBYTE CHARACTER HANDLING		*
 *						*
 ************************************************/

#if defined (HANDLE_MULTIBYTE)
static void
set_line_mbstate ()
{
  int i, previ, len, c;
  mbstate_t mbs, prevs;
  size_t mbclen;

  if (shell_input_line == NULL)
    return;
  len = strlen (shell_input_line);	/* XXX - shell_input_line_len ? */
  FREE (shell_input_line_property);
  shell_input_line_property = (char *)xmalloc (len + 1);

  memset (&prevs, '\0', sizeof (mbstate_t));
  for (i = previ = 0; i < len; i++)
    {
      mbs = prevs;

      c = shell_input_line[i];
      if (c == EOF)
	{
	  int j;
	  for (j = i; j < len; j++)
	    shell_input_line_property[j] = 1;
	  break;
	}

      mbclen = mbrlen (shell_input_line + previ, i - previ + 1, &mbs);
      if (mbclen == 1 || mbclen == (size_t)-1)
	{
	  mbclen = 1;
	  previ = i + 1;
	}
      else if (mbclen == (size_t)-2)
        mbclen = 0;
      else if (mbclen > 1)
	{
	  mbclen = 0;
	  previ = i + 1;
	  prevs = mbs;
	}
      else
	{
	  /* XXX - what to do if mbrlen returns 0? (null wide character) */
	  int j;
	  for (j = i; j < len; j++)
	    shell_input_line_property[j] = 1;
	  break;
	}

      shell_input_line_property[i] = mbclen;
    }
}
#endif /* HANDLE_MULTIBYTE */
#line 5040 "y.tab.c"

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = yyname[YYTRANSLATE(yychar)];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 352 "./parse.y"
	{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = yystack.l_mark[-1].command;
			  eof_encountered = 0;
			  /* discard_parser_constructs (0); */
			  YYACCEPT;
			}
break;
case 2:
#line 361 "./parse.y"
	{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
break;
case 3:
#line 368 "./parse.y"
	{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  /* discard_parser_constructs (1); */
			  if (interactive)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
break;
case 4:
#line 383 "./parse.y"
	{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
break;
case 5:
#line 393 "./parse.y"
	{ yyval.word_list = make_word_list (yystack.l_mark[0].word, (WORD_LIST *)NULL); }
break;
case 6:
#line 395 "./parse.y"
	{ yyval.word_list = make_word_list (yystack.l_mark[0].word, yystack.l_mark[-1].word_list); }
break;
case 7:
#line 399 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (1, r_output_direction, redir);
			}
break;
case 8:
#line 404 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (0, r_input_direction, redir);
			}
break;
case 9:
#line 409 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_output_direction, redir);
			}
break;
case 10:
#line 414 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_input_direction, redir);
			}
break;
case 11:
#line 419 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (1, r_appending_to, redir);
			}
break;
case 12:
#line 424 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_appending_to, redir);
			}
break;
case 13:
#line 429 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (0, r_reading_until, redir);
			  push_heredoc (yyval.redirect);
			}
break;
case 14:
#line 435 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_reading_until, redir);
			  push_heredoc (yyval.redirect);
			}
break;
case 15:
#line 441 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (0, r_reading_string, redir);
			}
break;
case 16:
#line 446 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_reading_string, redir);
			}
break;
case 17:
#line 451 "./parse.y"
	{
			  redir.dest = yystack.l_mark[0].number;
			  yyval.redirect = make_redirection (0, r_duplicating_input, redir);
			}
break;
case 18:
#line 456 "./parse.y"
	{
			  redir.dest = yystack.l_mark[0].number;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_duplicating_input, redir);
			}
break;
case 19:
#line 461 "./parse.y"
	{
			  redir.dest = yystack.l_mark[0].number;
			  yyval.redirect = make_redirection (1, r_duplicating_output, redir);
			}
break;
case 20:
#line 466 "./parse.y"
	{
			  redir.dest = yystack.l_mark[0].number;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_duplicating_output, redir);
			}
break;
case 21:
#line 471 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (0, r_duplicating_input_word, redir);
			}
break;
case 22:
#line 476 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_duplicating_input_word, redir);
			}
break;
case 23:
#line 481 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (1, r_duplicating_output_word, redir);
			}
break;
case 24:
#line 486 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_duplicating_output_word, redir);
			}
break;
case 25:
#line 491 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection
			    (0, r_deblank_reading_until, redir);
			  push_heredoc (yyval.redirect);
			}
break;
case 26:
#line 498 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection
			    (yystack.l_mark[-2].number, r_deblank_reading_until, redir);
			  push_heredoc (yyval.redirect);
			}
break;
case 27:
#line 505 "./parse.y"
	{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (1, r_close_this, redir);
			}
break;
case 28:
#line 510 "./parse.y"
	{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_close_this, redir);
			}
break;
case 29:
#line 515 "./parse.y"
	{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (0, r_close_this, redir);
			}
break;
case 30:
#line 520 "./parse.y"
	{
			  redir.dest = 0;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_close_this, redir);
			}
break;
case 31:
#line 525 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (1, r_err_and_out, redir);
			}
break;
case 32:
#line 530 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_input_output, redir);
			}
break;
case 33:
#line 535 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (0, r_input_output, redir);
			}
break;
case 34:
#line 540 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (1, r_output_force, redir);
			}
break;
case 35:
#line 545 "./parse.y"
	{
			  redir.filename = yystack.l_mark[0].word;
			  yyval.redirect = make_redirection (yystack.l_mark[-2].number, r_output_force, redir);
			}
break;
case 36:
#line 552 "./parse.y"
	{ yyval.element.word = yystack.l_mark[0].word; yyval.element.redirect = 0; }
break;
case 37:
#line 554 "./parse.y"
	{ yyval.element.word = yystack.l_mark[0].word; yyval.element.redirect = 0; }
break;
case 38:
#line 556 "./parse.y"
	{ yyval.element.redirect = yystack.l_mark[0].redirect; yyval.element.word = 0; }
break;
case 39:
#line 560 "./parse.y"
	{
			  yyval.redirect = yystack.l_mark[0].redirect;
			}
break;
case 40:
#line 564 "./parse.y"
	{
			  register REDIRECT *t;

			  for (t = yystack.l_mark[-1].redirect; t->next; t = t->next)
			    ;
			  t->next = yystack.l_mark[0].redirect;
			  yyval.redirect = yystack.l_mark[-1].redirect;
			}
break;
case 41:
#line 575 "./parse.y"
	{ yyval.command = make_simple_command (yystack.l_mark[0].element, (COMMAND *)NULL); }
break;
case 42:
#line 577 "./parse.y"
	{ yyval.command = make_simple_command (yystack.l_mark[0].element, yystack.l_mark[-1].command); }
break;
case 43:
#line 581 "./parse.y"
	{ yyval.command = clean_simple_command (yystack.l_mark[0].command); }
break;
case 44:
#line 583 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 45:
#line 585 "./parse.y"
	{
			  COMMAND *tc;

			  tc = yystack.l_mark[-1].command;
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = yystack.l_mark[0].redirect;
			    }
			  else
			    tc->redirects = yystack.l_mark[0].redirect;
			  yyval.command = yystack.l_mark[-1].command;
			}
break;
case 46:
#line 601 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 47:
#line 605 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 48:
#line 607 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 49:
#line 609 "./parse.y"
	{ yyval.command = make_while_command (yystack.l_mark[-3].command, yystack.l_mark[-1].command); }
break;
case 50:
#line 611 "./parse.y"
	{ yyval.command = make_until_command (yystack.l_mark[-3].command, yystack.l_mark[-1].command); }
break;
case 51:
#line 613 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 52:
#line 615 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 53:
#line 617 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 54:
#line 619 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 55:
#line 621 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 56:
#line 623 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 57:
#line 625 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 58:
#line 629 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 59:
#line 634 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 60:
#line 639 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 61:
#line 644 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 62:
#line 649 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-8].word, REVERSE_LIST (yystack.l_mark[-5].word_list, WORD_LIST *), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 63:
#line 654 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-8].word, REVERSE_LIST (yystack.l_mark[-5].word_list, WORD_LIST *), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 64:
#line 659 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-7].word, (WORD_LIST *)NULL, yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 65:
#line 664 "./parse.y"
	{
			  yyval.command = make_for_command (yystack.l_mark[-7].word, (WORD_LIST *)NULL, yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 66:
#line 671 "./parse.y"
	{
				  yyval.command = make_arith_for_command (yystack.l_mark[-5].word_list, yystack.l_mark[-1].command, arith_for_lineno);
				  if (word_top > 0) word_top--;
				}
break;
case 67:
#line 676 "./parse.y"
	{
				  yyval.command = make_arith_for_command (yystack.l_mark[-5].word_list, yystack.l_mark[-1].command, arith_for_lineno);
				  if (word_top > 0) word_top--;
				}
break;
case 68:
#line 681 "./parse.y"
	{
				  yyval.command = make_arith_for_command (yystack.l_mark[-3].word_list, yystack.l_mark[-1].command, arith_for_lineno);
				  if (word_top > 0) word_top--;
				}
break;
case 69:
#line 686 "./parse.y"
	{
				  yyval.command = make_arith_for_command (yystack.l_mark[-3].word_list, yystack.l_mark[-1].command, arith_for_lineno);
				  if (word_top > 0) word_top--;
				}
break;
case 70:
#line 693 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 71:
#line 698 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-4].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 72:
#line 703 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 73:
#line 708 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-5].word, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 74:
#line 713 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-8].word, REVERSE_LIST (yystack.l_mark[-5].word_list, WORD_LIST *), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 75:
#line 718 "./parse.y"
	{
			  yyval.command = make_select_command (yystack.l_mark[-8].word, REVERSE_LIST (yystack.l_mark[-5].word_list, WORD_LIST *), yystack.l_mark[-1].command, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 76:
#line 725 "./parse.y"
	{
			  yyval.command = make_case_command (yystack.l_mark[-4].word, (PATTERN_LIST *)NULL, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 77:
#line 730 "./parse.y"
	{
			  yyval.command = make_case_command (yystack.l_mark[-5].word, yystack.l_mark[-2].pattern, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 78:
#line 735 "./parse.y"
	{
			  yyval.command = make_case_command (yystack.l_mark[-4].word, yystack.l_mark[-1].pattern, word_lineno[word_top]);
			  if (word_top > 0) word_top--;
			}
break;
case 79:
#line 742 "./parse.y"
	{ yyval.command = make_function_def (yystack.l_mark[-4].word, yystack.l_mark[0].command, function_dstart, function_bstart); }
break;
case 80:
#line 745 "./parse.y"
	{ yyval.command = make_function_def (yystack.l_mark[-4].word, yystack.l_mark[0].command, function_dstart, function_bstart); }
break;
case 81:
#line 748 "./parse.y"
	{ yyval.command = make_function_def (yystack.l_mark[-2].word, yystack.l_mark[0].command, function_dstart, function_bstart); }
break;
case 82:
#line 753 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 83:
#line 755 "./parse.y"
	{
			  COMMAND *tc;

			  tc = yystack.l_mark[-1].command;
			  /* According to Posix.2 3.9.5, redirections
			     specified after the body of a function should
			     be attached to the function and performed when
			     the function is executed, not as part of the
			     function definition command. */
			  /* XXX - I don't think it matters, but we might
			     want to change this in the future to avoid
			     problems differentiating between a function
			     definition with a redirection and a function
			     definition containing a single command with a
			     redirection.  The two are semantically equivalent,
			     though -- the only difference is in how the
			     command printing code displays the redirections. */
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = yystack.l_mark[0].redirect;
			    }
			  else
			    tc->redirects = yystack.l_mark[0].redirect;
			  yyval.command = yystack.l_mark[-1].command;
			}
break;
case 84:
#line 786 "./parse.y"
	{
			  yyval.command = make_subshell_command (yystack.l_mark[-1].command);
			  yyval.command->flags |= CMD_WANT_SUBSHELL;
			}
break;
case 85:
#line 793 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-3].command, yystack.l_mark[-1].command, (COMMAND *)NULL); }
break;
case 86:
#line 795 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-5].command, yystack.l_mark[-3].command, yystack.l_mark[-1].command); }
break;
case 87:
#line 797 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-4].command, yystack.l_mark[-2].command, yystack.l_mark[-1].command); }
break;
case 88:
#line 802 "./parse.y"
	{ yyval.command = make_group_command (yystack.l_mark[-1].command); }
break;
case 89:
#line 806 "./parse.y"
	{ yyval.command = make_arith_command (yystack.l_mark[0].word_list); }
break;
case 90:
#line 810 "./parse.y"
	{ yyval.command = yystack.l_mark[-1].command; }
break;
case 91:
#line 814 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-2].command, yystack.l_mark[0].command, (COMMAND *)NULL); }
break;
case 92:
#line 816 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-4].command, yystack.l_mark[-2].command, yystack.l_mark[0].command); }
break;
case 93:
#line 818 "./parse.y"
	{ yyval.command = make_if_command (yystack.l_mark[-3].command, yystack.l_mark[-1].command, yystack.l_mark[0].command); }
break;
case 95:
#line 823 "./parse.y"
	{ yystack.l_mark[0].pattern->next = yystack.l_mark[-1].pattern; yyval.pattern = yystack.l_mark[0].pattern; }
break;
case 96:
#line 827 "./parse.y"
	{ yyval.pattern = make_pattern_list (yystack.l_mark[-2].word_list, yystack.l_mark[0].command); }
break;
case 97:
#line 829 "./parse.y"
	{ yyval.pattern = make_pattern_list (yystack.l_mark[-2].word_list, (COMMAND *)NULL); }
break;
case 98:
#line 831 "./parse.y"
	{ yyval.pattern = make_pattern_list (yystack.l_mark[-2].word_list, yystack.l_mark[0].command); }
break;
case 99:
#line 833 "./parse.y"
	{ yyval.pattern = make_pattern_list (yystack.l_mark[-2].word_list, (COMMAND *)NULL); }
break;
case 101:
#line 838 "./parse.y"
	{ yystack.l_mark[-1].pattern->next = yystack.l_mark[-2].pattern; yyval.pattern = yystack.l_mark[-1].pattern; }
break;
case 102:
#line 842 "./parse.y"
	{ yyval.word_list = make_word_list (yystack.l_mark[0].word, (WORD_LIST *)NULL); }
break;
case 103:
#line 844 "./parse.y"
	{ yyval.word_list = make_word_list (yystack.l_mark[0].word, yystack.l_mark[-2].word_list); }
break;
case 104:
#line 853 "./parse.y"
	{
			  yyval.command = yystack.l_mark[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			 }
break;
case 106:
#line 862 "./parse.y"
	{
			  yyval.command = yystack.l_mark[0].command;
			}
break;
case 108:
#line 869 "./parse.y"
	{
			  if (yystack.l_mark[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yystack.l_mark[-2].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yystack.l_mark[-2].command, (COMMAND *)NULL, '&');
			}
break;
case 110:
#line 880 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, AND_AND); }
break;
case 111:
#line 882 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, OR_OR); }
break;
case 112:
#line 884 "./parse.y"
	{
			  if (yystack.l_mark[-3].command->type == cm_connection)
			    yyval.command = connect_async_list (yystack.l_mark[-3].command, yystack.l_mark[0].command, '&');
			  else
			    yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, '&');
			}
break;
case 113:
#line 891 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, ';'); }
break;
case 114:
#line 893 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, ';'); }
break;
case 115:
#line 895 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 118:
#line 903 "./parse.y"
	{ yyval.number = '\n'; }
break;
case 119:
#line 905 "./parse.y"
	{ yyval.number = ';'; }
break;
case 120:
#line 907 "./parse.y"
	{ yyval.number = yacc_EOF; }
break;
case 123:
#line 921 "./parse.y"
	{
			  yyval.command = yystack.l_mark[0].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 124:
#line 927 "./parse.y"
	{
			  if (yystack.l_mark[-1].command->type == cm_connection)
			    yyval.command = connect_async_list (yystack.l_mark[-1].command, (COMMAND *)NULL, '&');
			  else
			    yyval.command = command_connect (yystack.l_mark[-1].command, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 125:
#line 936 "./parse.y"
	{
			  yyval.command = yystack.l_mark[-1].command;
			  if (need_here_doc)
			    gather_here_documents ();
			}
break;
case 126:
#line 944 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, AND_AND); }
break;
case 127:
#line 946 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, OR_OR); }
break;
case 128:
#line 948 "./parse.y"
	{
			  if (yystack.l_mark[-2].command->type == cm_connection)
			    yyval.command = connect_async_list (yystack.l_mark[-2].command, yystack.l_mark[0].command, '&');
			  else
			    yyval.command = command_connect (yystack.l_mark[-2].command, yystack.l_mark[0].command, '&');
			}
break;
case 129:
#line 955 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-2].command, yystack.l_mark[0].command, ';'); }
break;
case 130:
#line 958 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 131:
#line 962 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 132:
#line 964 "./parse.y"
	{
			  if (yystack.l_mark[0].command)
			    yystack.l_mark[0].command->flags |= CMD_INVERT_RETURN;
			  yyval.command = yystack.l_mark[0].command;
			}
break;
case 133:
#line 970 "./parse.y"
	{
			  if (yystack.l_mark[0].command)
			    yystack.l_mark[0].command->flags |= yystack.l_mark[-1].number;
			  yyval.command = yystack.l_mark[0].command;
			}
break;
case 134:
#line 976 "./parse.y"
	{
			  if (yystack.l_mark[0].command)
			    yystack.l_mark[0].command->flags |= yystack.l_mark[-2].number|CMD_INVERT_RETURN;
			  yyval.command = yystack.l_mark[0].command;
			}
break;
case 135:
#line 982 "./parse.y"
	{
			  if (yystack.l_mark[0].command)
			    yystack.l_mark[0].command->flags |= yystack.l_mark[-1].number|CMD_INVERT_RETURN;
			  yyval.command = yystack.l_mark[0].command;
			}
break;
case 136:
#line 988 "./parse.y"
	{
			  ELEMENT x;

			  /* Boy, this is unclean.  `time' by itself can
			     time a null command.  We cheat and push a
			     newline back if the list_terminator was a newline
			     to avoid the double-newline problem (one to
			     terminate this, one to terminate the command) */
			  x.word = 0;
			  x.redirect = 0;
			  yyval.command = make_simple_command (x, (COMMAND *)NULL);
			  yyval.command->flags |= yystack.l_mark[-1].number;
			  /* XXX - let's cheat and push a newline back */
			  if (yystack.l_mark[0].number == '\n')
			    token_to_read = '\n';
			}
break;
case 137:
#line 1009 "./parse.y"
	{ yyval.command = command_connect (yystack.l_mark[-3].command, yystack.l_mark[0].command, '|'); }
break;
case 138:
#line 1011 "./parse.y"
	{ yyval.command = yystack.l_mark[0].command; }
break;
case 139:
#line 1015 "./parse.y"
	{ yyval.number = CMD_TIME_PIPELINE; }
break;
case 140:
#line 1017 "./parse.y"
	{ yyval.number = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
break;
#line 6073 "y.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                yys = yyname[YYTRANSLATE(yychar)];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
