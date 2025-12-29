#include "git-compat-util.h"
#include "common-init.h"

#ifdef __amigaos4__
__attribute__ ((used)) static const char *version = "$VER: git " G_VERSION " for AmigaOS4 (" G_DATE ")";
__attribute__ ((used)) static const char *sc = "$STACK: 3512000";
#endif

int main(int argc, const char **argv)
{
	int result;

	init_git(argv);
	result = cmd_main(argc, argv);

	/* Not exit(3), but a wrapper calling our common_exit() */
	exit(result);
}
