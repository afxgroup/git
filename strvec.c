#include "git-compat-util.h"
#include "strvec.h"
#include "strbuf.h"

const char *empty_strvec[] = { NULL };

void strvec_init(struct strvec *array)
{
	struct strvec blank = STRVEC_INIT;
	memcpy(array, &blank, sizeof(*array));
}

static void strvec_push_nodup(struct strvec *array, const char *value)
{
	if (array->v == empty_strvec)
		array->v = NULL;

	ALLOC_GROW(array->v, array->nr + 2, array->alloc);
	array->v[array->nr++] = value;
	array->v[array->nr] = NULL;
}

const char *strvec_push(struct strvec *array, const char *value)
{
	strvec_push_nodup(array, xstrdup(value));
	return array->v[array->nr - 1];
}

const char *strvec_pushf(struct strvec *array, const char *fmt, ...)
{
	va_list ap;
	struct strbuf v = STRBUF_INIT;

	va_start(ap, fmt);
	strbuf_vaddf(&v, fmt, ap);
	va_end(ap);

	strvec_push_nodup(array, strbuf_detach(&v, NULL));
	return array->v[array->nr - 1];
}

void strvec_pushl(struct strvec *array, ...)
{
	va_list ap;
	const char *arg;

	va_start(ap, array);
	while ((arg = va_arg(ap, const char *)))
		strvec_push(array, arg);
	va_end(ap);
}

void strvec_pushv(struct strvec *array, const char **items)
{
	for (; *items; items++)
		strvec_push(array, *items);
}

const char *strvec_replace(struct strvec *array, size_t idx,
			   const char *replacement)
{
	char *to_free;
	if (idx >= array->nr)
		BUG("index outside of array boundary");
	to_free = (char *)array->v[idx];
	array->v[idx] = xstrdup(replacement);
	free(to_free);
	return array->v[idx];
}

void strvec_remove(struct strvec *array, size_t idx)
{
	if (idx >= array->nr)
		BUG("index outside of array boundary");
	free((char *)array->v[idx]);
	memmove(array->v + idx, array->v + idx + 1,
		(array->nr - idx) * sizeof(char *));
	array->nr--;
}

void strvec_pop(struct strvec *array)
{
	if (!array->nr)
		return;
	free((char *)array->v[array->nr - 1]);
	array->v[array->nr - 1] = NULL;
	array->nr--;
}

void strvec_split(struct strvec *array, const char *to_split)
{
	while (isspace(*to_split))
		to_split++;
	for (;;) {
		const char *p = to_split;

		if (!*p)
			break;

		while (*p && !isspace(*p))
			p++;
		strvec_push_nodup(array, xstrndup(to_split, p - to_split));

		while (isspace(*p))
			p++;
		to_split = p;
	}
}

void strvec_clear(struct strvec *array)
{
	if (array->v != empty_strvec) {
		int i;
		for (i = 0; i < array->nr; i++) {
			if (array->v[i] != NULL) {
				free((char *)array->v[i]);
				array->v[i] = NULL;
			}
		}
		if (array->v != NULL) {
			free(array->v);
			array->v = NULL;
		}
	}
	strvec_init(array);
}

const char **strvec_detach(struct strvec *array)
{
	if (array->v == empty_strvec)
		return xcalloc(1, sizeof(const char *));
	else {
		const char **ret = array->v;
		strvec_init(array);
		return ret;
	}
}
