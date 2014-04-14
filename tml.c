#include  <stdio.h>
#include  <dlfcn.h>
#include  <execinfo.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <errno.h>
#include  <string.h>

static const int MAX_STACK = 10;

/** logs all the tracing to this file **/
#define  MLEAK_TRACE_FILE  "tml.txt"
#define  ALL_READ   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define  PRINT(FP, FMT...)  fprintf(FP, FMT);
#define  PRINT_E(FMT...)  PRINT(stderr, FMT);

#define  PRINT_BUF  write(trace_fd, buf, sz);

static int g_no_hook = 0;
static int g_no_rhook = 0;

/** pointers to library functions **/
void *(*mallocp)(size_t);
void (*freep)(void *);
void *(*reallocp)(void *, size_t);

static int trace_fd = 0;

/**
 * Initialization function; initializes the library functions which are called
 *   after logging the stack trace.
 *
 */
static void __attribute__((constructor))
ginit(void)
{
	const char *fname = MLEAK_TRACE_FILE;
	if (fname && trace_fd <= 0) {
		trace_fd = open(fname, O_TRUNC|O_CREAT|O_WRONLY, ALL_READ);
		if (trace_fd < 0) {
			PRINT_E("error= %d: %s\n", errno, strerror(errno));
			trace_fd = 2;
		}
	}
	mallocp = (void*(*)(size_t)) dlsym(RTLD_NEXT, "malloc");
	freep   = (void(*)(void *)) dlsym(RTLD_NEXT, "free");
	reallocp= (void*(*)(void *, size_t)) dlsym(RTLD_NEXT, "realloc");
	write(trace_fd, "= Start\n", 8);
}

/**
 * Simple helper to log the required tracing for malloc.
 *
 */
static void
print_details(const void *result, const int size,
	      void *addrs[], const int num_addrs, const char c)
{
	char buf[128] = {0};
	int i = 0, sz = 0;
	sz = snprintf(buf, sizeof(buf), "@ e:(f+a)[0x%x] %c %p 0x%x\t ",
		      addrs[0], c, result, size);
	for (i = 0; i < num_addrs; ++i) {
		sz += snprintf(buf+sz, sizeof(buf), "/%p", addrs[i]);
	}
	sz += snprintf(buf+sz, sizeof(buf), "\n");
	PRINT_BUF;
}

/**
 * malloc tap; logs the required tracing for malloc and calls the library
 *   function with same name.
 */
void *
malloc(int size)
{
	int i = 0;
	size_t sz= 0;
	void *result;
	void *addrs[MAX_STACK];

	if (!mallocp) { ginit(); }

	if (g_no_hook) {
		return (*mallocp)(size);
	}
	g_no_hook = 1;
        sz = backtrace(addrs, MAX_STACK);
	result = (*mallocp)(size);
#if  DEBUG
	PRINT_E("result= %p; addrs[0]= %p; size= %d \tstack= ",
		result, addrs[0],sz);
	for (i = 0; i < sz; ++i) {
		PRINT_E("/%p", addrs[i]);
	}
	PRINT_E("\n");
#endif
	print_details(result, size, addrs+1, sz-3, '+');
	g_no_hook = 0;
	return result;
}

/**
 * free tap; logs the required tracing for malloc and calls the library
 *   function with same name.
 */
void
free(void *ptr)
{
	size_t sz= 0;
	char buf[32] = {0};

	sz = snprintf(buf, sizeof(buf), "@ e:[%p] - %p\n",
		 __builtin_return_address(0), ptr);

	PRINT_BUF;
	(*freep)(ptr);
}

/**
 * realloc tap; logs the required tracing for malloc and calls the library
 *   function with same name.
 */
void *
realloc(void *ptr, size_t size)
{
	int i = 0;
	size_t sz= 0;
	void *result;
	void *addrs[MAX_STACK];
	char buf[32] = {0};

	if (g_no_rhook) {
		return (*reallocp)(ptr, size);
	}
	g_no_rhook = 1;
	result = (*reallocp)(ptr, size);
	sz = snprintf(buf, sizeof(buf), "@ e:[%p] < %p\n",
		 __builtin_return_address(0), ptr);
	PRINT_BUF;

        sz = backtrace(addrs, MAX_STACK);
	print_details(result, size, addrs+1, sz-3, '>');
	g_no_rhook = 0;
	return result;
}
