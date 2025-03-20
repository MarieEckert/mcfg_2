/* shared.h ; marie config format internal shared function header
 * for MCFG/2
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>

#include <sys/types.h>

#ifdef MCFG_DO_ERROR_MESSAGES
#	include <stdio.h>
#	define ERR_NOTE(e)                                        \
		fprintf(stderr,                                        \
				"ERR_CHECK failed at line %d in file "__FILE__ \
				" (err: %d)\n",                                \
				__LINE__, e)
#else
#	define ERR_NOTE(e)
#endif

#define ERR_CHECK(c, e)  \
	({                   \
		if(!(c)) {       \
			ERR_NOTE(e); \
			return e;    \
		}                \
	})

#define CONCAT(a, b)					  a##b

#define _INTERNAL_PREFIX(name)			  CONCAT(_mcfg_internal_##name, _)
#define INTERNAL_PREFIX(name)			  _INTERNAL_PREFIX(name)

#define _NAMESPACED_DECL(namespace, name) CONCAT(namespace, name)
#define NAMESPACED_DECL(name)			  _NAMESPACED_DECL(INTERNAL_PREFIX(NAMESPACE), name)

#define _SHARED_NAMESPACE				  shared
#define _SHARED_NAMESPACED_DECL(name) \
	_NAMESPACED_DECL(INTERNAL_PREFIX(_SHARED_NAMESPACE), name)

#define strchrnul _SHARED_NAMESPACED_DECL(strchrnul)
char *strchrnul(const char *str, int c);

#define is_string_empty _SHARED_NAMESPACED_DECL(is_string_empty)
bool is_string_empty(char *in);

#define remove_newline _SHARED_NAMESPACED_DECL(remove_newline)
char *remove_newline(char *in);

#define has_newline _SHARED_NAMESPACED_DECL(has_newline)
bool has_newline(char *in);

#define strcpy_until _SHARED_NAMESPACED_DECL(strcpy_until)
char *strcpy_until(char *src, char delimiter);

#define bstrcpy_until _SHARED_NAMESPACED_DECL(bstrcpy_until)
char *bstrcpy_until(char *src, char *src_org, char delimiter);

#define find_prev _SHARED_NAMESPACED_DECL(find_prev)
char *find_prev(char *src, char *src_org, char delimiter);

#endif
