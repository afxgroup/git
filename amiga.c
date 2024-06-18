#ifdef __amigaos4__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "git-compat-util.h"

extern int __get_default_file(int file_descriptor, long *file_ptr);

#define NUM_ENTRIES(t) (sizeof(t) / sizeof(t[0]))
#define BZERO ((BPTR)NULL)

static int
__translate_io_error_to_errno(LONG io_err) {
	static const struct {
		LONG io_err;
		LONG _errno;
	} map_table[] =
		{
			{ERROR_NO_FREE_STORE,          ENOMEM},
			{ERROR_TASK_TABLE_FULL,        ENOMEM},
			{ERROR_BAD_TEMPLATE,           EINVAL},
			{ERROR_BAD_NUMBER,             EINVAL},
			{ERROR_REQUIRED_ARG_MISSING,   EINVAL},
			{ERROR_KEY_NEEDS_ARG,          EINVAL},
			{ERROR_TOO_MANY_ARGS,          EINVAL},
			{ERROR_UNMATCHED_QUOTES,       EINVAL},
			{ERROR_LINE_TOO_LONG,          ENAMETOOLONG},
			{ERROR_FILE_NOT_OBJECT,        ENOEXEC},
			{ERROR_OBJECT_IN_USE,          EBUSY},
			{ERROR_OBJECT_EXISTS,          EEXIST},
			{ERROR_DIR_NOT_FOUND,          ENOENT},
			{ERROR_OBJECT_NOT_FOUND,       ENOENT},
			{ERROR_BAD_STREAM_NAME,        EINVAL},
			{ERROR_OBJECT_TOO_LARGE,       EFBIG},
			{ERROR_ACTION_NOT_KNOWN,       ENOSYS},
			{ERROR_INVALID_COMPONENT_NAME, EINVAL},
			{ERROR_INVALID_LOCK,           EBADF},
			{ERROR_OBJECT_WRONG_TYPE,      EFTYPE},
			{ERROR_DISK_NOT_VALIDATED,     EROFS},
			{ERROR_DISK_WRITE_PROTECTED,   EROFS},
			{ERROR_RENAME_ACROSS_DEVICES,  EXDEV},
			{ERROR_DIRECTORY_NOT_EMPTY,    ENOTEMPTY},
			{ERROR_TOO_MANY_LEVELS,        ENAMETOOLONG},
			{ERROR_DEVICE_NOT_MOUNTED,     ENXIO},
			{ERROR_COMMENT_TOO_BIG,        ENAMETOOLONG},
			{ERROR_DISK_FULL,              ENOSPC},
			{ERROR_DELETE_PROTECTED,       EACCES},
			{ERROR_WRITE_PROTECTED,        EACCES},
			{ERROR_READ_PROTECTED,         EACCES},
			{ERROR_NOT_A_DOS_DISK,         EFTYPE},
			{ERROR_NO_DISK,                EACCES},
			{ERROR_IS_SOFT_LINK,           EFTYPE},
			{ERROR_BAD_HUNK,               ENOEXEC},
			{ERROR_NOT_IMPLEMENTED,        ENOSYS},
			{ERROR_LOCK_COLLISION,         EACCES},
			{ERROR_BREAK,                  EINTR},
			{ERROR_NOT_EXECUTABLE,         ENOEXEC}
		};

	unsigned int i;
	int result;

	result = EIO;

	for (i = 0; i < NUM_ENTRIES(map_table); i++) {
		if (map_table[i].io_err == io_err) {
			result = map_table[i]._errno;
			break;
		}
	}

	return (result);
}

static BOOL
string_needs_quoting(const char *string, size_t len) {
	BOOL result = FALSE;
	size_t i;
	char c;

	for (i = 0; i < len; i++) {
		c = (*string++);
		if (c == ' ' || ((unsigned char) c) == 0xA0 || c == '\t' || c == '\n' || c == '\"') {
			result = TRUE;
			break;
		}
	}

	return (result);
}

static void
build_arg_string(char *const argv[], char *arg_string) {
	BOOL first_char = TRUE;
	size_t i, j, len;
	char *s;

	/* The first argv[] element is skipped; it does not contain part of
	       the command line but holds the name of the program to be run. */
	for (i = 1; argv[i] != NULL; i++) {
		s = (char *) argv[i];

		len = strlen(s);
		if (len > 0) {
			if (first_char)
				first_char = FALSE;
			else
				(*arg_string++) = ' ';

			if ((*s) != '\"' && string_needs_quoting(s, len)) {
				(*arg_string++) = '\"';

				for (j = 0; j < len; j++) {
					if (s[j] == '\"' || s[j] == '*') {
						(*arg_string++) = '*';
						(*arg_string++) = s[j];
					} else if (s[j] == '\n') {
						(*arg_string++) = '*';
						(*arg_string++) = 'N';
					} else {
						(*arg_string++) = s[j];
					}
				}

				(*arg_string++) = '\"';
			} else {
				memcpy(arg_string, s, len);
				arg_string += len;
			}
		}
	}
}

static size_t
count_extra_escape_chars(const char *string, size_t len) {
	size_t count = 0;
	size_t i;
	char c;

	for (i = 0; i < len; i++) {
		c = (*string++);
		if (c == '\"' || c == '*' || c == '\n')
			count++;
	}

	return (count);
}

static size_t
get_arg_string_length(char *const argv[]) {
	size_t result = 0;
	size_t i, len = 0;
	char *s;

	/* The first argv[] element is skipped; it does not contain part of
	       the command line but holds the name of the program to be run. */
	for (i = 1; argv[i] != NULL; i++) {
		s = (char *) argv[i];

		len = strlen(s);
		if (len > 0) {
			if ((*s) != '\"') {
				if (string_needs_quoting(s, len))
					len += 1 + count_extra_escape_chars(s, len) + 1;
			}

			if (result == 0)
				result = len;
			else
				result = result + 1 + len;
		}
	}

	return (result);
}

int
amiga_spawnvpe(const char *file, const char **argv, char **deltaenv, const char *dir, int fhin, int fhout, int fherr) {
	int ret = -1;
	char *arg_string = NULL;
	size_t arg_string_len = 0;
	size_t parameter_string_len = 0;
	IDOS->Printf("amiga_spawnvpe = fhin = %ld - fhout = %ld - fherr = %ld\n", fhin, fhout, fherr);
	errno = 0;

	parameter_string_len = get_arg_string_length((char *const *) argv);
	if (parameter_string_len > _POSIX_ARG_MAX) {
		errno = E2BIG;
		return ret;
	}

	arg_string = malloc(parameter_string_len + 1);
	if (arg_string == NULL) {
		errno = ENOMEM;
		return ret;
	}

	if (parameter_string_len > 0) {
		build_arg_string((char *const *) argv, &arg_string[arg_string_len]);
		arg_string_len += parameter_string_len;
	}

	/* Add a NUL, to be nice... */
	arg_string[arg_string_len] = '\0';

	char finalpath[PATH_MAX] = {0};
	snprintf(finalpath, PATH_MAX - 1, "%s %s", file, arg_string);

	BPTR in = BZERO, out = BZERO, err = BZERO;
	int in_err = -1, out_err = -1, err_err = -1;

	if (fhin >= 0) {
		in_err = __get_default_file(fhin, &in);
	}
	if (fhout >= 0) {
		out_err = __get_default_file(fhout, &out);
	}
	if (fherr >= 0) {
		err_err = __get_default_file(fherr, &err);
	}

	//IDOS->Printf("finalpath = %s - in_err = %ld - out_err = %ld - err_err = %ld\n", finalpath, in_err, out_err, err_err);
	ret = IDOS->SystemTags(finalpath,
			 SYS_Input, in_err == 0 ? in : 0,
			 SYS_Output, out_err == 0 ? out : 0,
			 SYS_Error, err_err == 0 ? err : 0,
			 SYS_UserShell, TRUE,
			 SYS_Asynch, TRUE,
			 NP_Child, TRUE,
			 //NP_CurrentDir, dir != NULL ? IDOS->Lock(dir, SHARED_LOCK) : 0,
			 NP_Name, argv[0],
			 TAG_DONE);

	if (ret != 0) {
		IDOS->Printf("Error executing %s\n", finalpath);
		/* SystemTags failed. Clean up file handles */
		if (in_err == 0) IDOS->Close(in);
		if (out_err == 0) IDOS->Close(out);
		if (err_err == 0) IDOS->Close(err);
		errno = __translate_io_error_to_errno(IDOS->IoErr());
	}
	else {
		ret = IDOS->IoErr();
	}
	return ret;
}
#endif