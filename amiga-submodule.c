/*
 * git-submodule.c - C wrapper for git-submodule command on AmigaOS4
 * 
 * This replaces the git-submodule.sh shell script which cannot run on AmigaOS4.
 * It parses arguments and delegates to git-submodule--helper builtin command.
 */

#include "git-compat-util.h"
#include "config.h"
#include "run-command.h"
#include "strvec.h"
#include "parse-options.h"
#include "setup.h"

static const char * const git_submodule_usage[] = {
	N_("git submodule [--quiet] [--cached]"),
	N_("git submodule [--quiet] add [-b <branch>] [-f|--force] [--name <name>] [--reference <repository>] [--depth <depth>] [--] <repository> [<path>]"),
	N_("git submodule [--quiet] status [--cached] [--recursive] [--] [<path>...]"),
	N_("git submodule [--quiet] init [--] [<path>...]"),
	N_("git submodule [--quiet] deinit [-f|--force] (--all| [--] <path>...)"),
	N_("git submodule [--quiet] update [--init] [--remote] [-N|--no-fetch] [-f|--force] [--checkout|--merge|--rebase] [--reference <repository>] [--recursive] [--] [<path>...]"),
	N_("git submodule [--quiet] set-branch (--default|--branch <branch>) [--] <path>"),
	N_("git submodule [--quiet] set-url [--] <path> <newurl>"),
	N_("git submodule [--quiet] summary [--cached|--files] [--summary-limit <n>] [commit] [--] [<path>...]"),
	N_("git submodule [--quiet] foreach [--recursive] <command>"),
	N_("git submodule [--quiet] sync [--recursive] [--] [<path>...]"),
	N_("git submodule [--quiet] absorbgitdirs [--] [<path>...]"),
	NULL
};

static struct option builtin_submodule_options[] = {
	OPT_END()
};

int cmd_main(int argc, const char **argv)
{
	struct child_process cp = CHILD_PROCESS_INIT;
	int i, ret;
	int quiet = 0;
	int cached = 0;
	const char *command = NULL;
	const char *prefix;
	
	prefix = setup_git_directory();
	
	/* Set GIT_PROTOCOL_FROM_USER=0 as done in the shell script */
	setenv("GIT_PROTOCOL_FROM_USER", "0", 1);
	
	/* Skip program name */
	argc--;
	argv++;
	
	/* Parse arguments to find command and global options */
	for (i = 0; i < argc; i++) {
		const char *arg = argv[i];
		
		if (!strcmp(arg, "--")) {
			break;
		}
		
		/* Global options */
		if (!strcmp(arg, "-q") || !strcmp(arg, "--quiet")) {
			quiet = 1;
			continue;
		}
		
		if (!strcmp(arg, "--cached")) {
			cached = 1;
			continue;
		}
		
		/* Check if this is a command */
		if (!strcmp(arg, "add") || !strcmp(arg, "foreach") || 
		    !strcmp(arg, "init") || !strcmp(arg, "deinit") ||
		    !strcmp(arg, "update") || !strcmp(arg, "set-branch") ||
		    !strcmp(arg, "set-url") || !strcmp(arg, "status") ||
		    !strcmp(arg, "summary") || !strcmp(arg, "sync") ||
		    !strcmp(arg, "absorbgitdirs")) {
			command = arg;
			/* Collect remaining arguments after the command */
			i++;
			break;
		}
		
		/* If we hit an option or argument before a command, break */
		if (arg[0] == '-') {
			/* Unknown option, let submodule--helper handle it */
			break;
		} else {
			/* No command found, default to status if no args */
			break;
		}
	}
	
	/* Default to "status" if no command specified and no arguments */
	if (!command) {
		if (i >= argc) {
			command = "status";
		} else {
			/* Invalid usage */
			usage_with_options(git_submodule_usage, builtin_submodule_options);
		}
	}
	
	/* --cached is only accepted by status and summary */
	if (cached && strcmp(command, "status") && strcmp(command, "summary")) {
		usage_with_options(git_submodule_usage, builtin_submodule_options);
	}
	
	/* Build the git submodule--helper command */
	cp.git_cmd = 1;
	cp.no_stdin = 1;
	strvec_push(&cp.args, "submodule--helper");
	strvec_push(&cp.args, command);
	
	/* Add --prefix if we're in a subdirectory */
	if (prefix && *prefix) {
		strvec_push(&cp.args, "--prefix");
		strvec_push(&cp.args, prefix);
	}
	
	/* Add global options */
	if (quiet)
		strvec_push(&cp.args, "--quiet");
	
	/* For status and summary, add --cached if specified */
	if (cached && (!strcmp(command, "status") || !strcmp(command, "summary"))) {
		strvec_push(&cp.args, "--cached");
	}
	
	/* Add remaining arguments */
	for (; i < argc; i++) {
		strvec_push(&cp.args, argv[i]);
	}
	
	/* Debug: print command being executed */
	if (getenv("GIT_SUBMODULE_DEBUG")) {
		fprintf(stderr, "git-submodule: executing:");
		for (i = 0; i < cp.args.nr; i++)
			fprintf(stderr, " %s", cp.args.v[i]);
		fprintf(stderr, "\n");
		fflush(stderr);
	}
	
	/* Execute git submodule--helper */
	ret = run_command(&cp);
	
	/* Debug: print return code */
	if (getenv("GIT_SUBMODULE_DEBUG")) {
		fprintf(stderr, "git-submodule: command returned: %d\n", ret);
		fflush(stderr);
	}
	
	return ret;
}

