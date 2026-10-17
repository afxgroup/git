#include "git-compat-util.h"
#include "common-init.h"

#ifdef __amigaos4__
__attribute__ ((used)) static const char *version = "$VER: git " G_VERSION " for AmigaOS4 (" G_DATE ")";
__attribute__ ((used)) static const char *sc = "$STACK: 3512000";

#include <proto/exec.h>
#include <dos/dosextens.h>

static struct Process *amiga_process;
static APTR amiga_old_window_ptr;

static void amiga_restore_requesters(void)
{
	amiga_process->pr_WindowPtr = amiga_old_window_ptr;
}

/*
 * Git probes plenty of paths that are not there, some of them URLs like
 * "https://host/repo". On AmigaOS a path on a volume that does not exist
 * ("https:") pops up a "Please insert volume" requester and blocks the
 * process until somebody answers it. Make dos.library fail instead. Git
 * runs on the Shell's process, so put the old value back at exit.
 */
static void amiga_disable_requesters(void)
{
	amiga_process = (struct Process *)IExec->FindTask(NULL);
	amiga_old_window_ptr = amiga_process->pr_WindowPtr;
	amiga_process->pr_WindowPtr = (APTR)-1;
	atexit(amiga_restore_requesters);
}
#endif

int main(int argc, const char **argv)
{
	int result;

#ifdef __amigaos4__
	amiga_disable_requesters();
#endif
	init_git(argv);
	result = cmd_main(argc, argv);

	/* Not exit(3), but a wrapper calling our common_exit() */
	exit(result);
}
