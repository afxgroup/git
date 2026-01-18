/* AmigaOS4 Memory Cleanup Patch
 * Add this to git.c to force cleanup of global caches
 */

#ifdef GIT_AMIGAOS4_NATIVE
static void amiga_cleanup_git_memory(void)
{
	/* Force cleanup of Git's global caches */
	
	/* 1. Discard the index */
	discard_index(&the_index);
	
	/* 2. Close all pack files */
	close_object_store(the_repository->objects);
	
	/* 3. Clear config cache */
	git_config_clear();
	
	/* 4. Clear string intern pool if possible */
	/* Note: Git doesn't expose this API, may need to add it */
	
	/* 5. Clear any cached paths */
	invalidate_lstat_cache();
	
	/* 6. Free repository structures */
	repo_clear(the_repository);
}

/* Register cleanup on exit */
void amiga_register_cleanup(void)
{
	atexit(amiga_cleanup_git_memory);
}
#endif
