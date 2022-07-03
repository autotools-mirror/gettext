
#if @HAVE_VISIBILITY@ && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __attribute__((__visibility__("default")))
#elif (defined _WIN32 && !defined __CYGWIN__) && defined WOE32DLL && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __declspec(dllexport)
#elif (defined _WIN32 && !defined __CYGWIN__) && defined WOE32DLL
#define LIBINTL_DLL_EXPORTED __declspec(dllimport)
#else
#define LIBINTL_DLL_EXPORTED
#endif
