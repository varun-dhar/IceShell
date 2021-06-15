%{
#include "util.h"
#include "ll.h"
#include <string.h>
#include <unistd.h>

Command cmd;
LL_NEW(cmd.data);
cmd.curr = LL_HEAD(cmd.data);
%}

command:
	command WORD {
		LL_PUSH(cmd.curr->args,strdup($1));
	}
	WORD {
		LL_NEW(cmd.curr->args);
		LL_PUSH(cmd.args,strdup($1));
	}

pipe_expr: 
	pipe_expr '|' command {
		LL_PUSH(cmd.data,strdup($2));
		cmd.curr = LL_TAIL(cmd.data);
	}
	command '|' command {
		LL_PUSH(cmd.data,strdup($1);
		LL_PUSH(cmd.data,strdup($2);
		cmd.curr = LL_TAIL(cmd.data);
	}

redir:
	command '>' WORD {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfile = strdup($2);
		cmd.append = false;
	}

	command NUM '>' WORD {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = $2;
		cmd.outfile = strdup($3);
		cmd.append = false;
	}

	command NUM '>' '&' NUM {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfd = $3;
		cmd.append = false;
	}

	command '>' '&' NUM {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfd = $2;
		cmd.append = false;
	}

	command ">>" WORD {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfile = strdup($2);
		cmd.append = true;
	}

	command NUM ">>" WORD {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = $2;
		cmd.outfile = strdup($3);
		cmd.append = true;
	}

	command NUM ">>" '&' NUM {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfd = $3;
		cmd.append = true;
	}

	command ">>" '&' NUM {
		cmd.infile = NULL;
		cmd.infd = STDIN_FILENO;
		cmd.ostream = STDOUT_FILENO;
		cmd.outfd = $2;
		cmd.append = true;
	}
