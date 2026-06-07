#include "git-compat-util.h"
#include "common-init.h"

#ifdef GIT_AMIGAOS4_NATIVE
__attribute__ ((used)) static const char *version = "$VER: git " G_VERSION " for AmigaOS4 (" G_DATE ")";
__attribute__ ((used)) static const char *sc = "$STACK: 3512000";
#endif

int main(int argc, const char **argv)
{
	int result;

	init_git(argv);

#ifdef GIT_AMIGAOS4_NATIVE
	/*
	 * AmigaOS4/clib4: spawnvpe() inherits ALL open file descriptors from
	 * the parent.  The parent communicates which extra fds must be closed
	 * immediately via GIT_AMIGA_CLOSE_FDS (comma-separated fd numbers).
	 * This mirrors the close-after-fork fd cleanup that Linux performs
	 * between fork() and exec() — without it, a child holding the write-end
	 * of its own stdin pipe prevents the PIPE: device from ever signalling
	 * EOF, causing the child to block forever on stdin reads.
	 */
	{
		const char *close_fds_env = getenv("GIT_AMIGA_CLOSE_FDS");
		if (close_fds_env && *close_fds_env) {
			const char *p = close_fds_env;
			unsetenv("GIT_AMIGA_CLOSE_FDS");
			while (*p) {
				int fd = atoi(p);
				if (fd > 2)
					close(fd);
				p = strchr(p, ',');
				if (!p)
					break;
				p++; /* skip comma */
			}
		}
	}
#endif

	result = cmd_main(argc, argv);

	/* Not exit(3), but a wrapper calling our common_exit() */
	exit(result);
}
