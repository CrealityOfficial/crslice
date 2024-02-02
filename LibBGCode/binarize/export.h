
#ifndef BGCODE_BINARIZE_EXPORT_H
#define BGCODE_BINARIZE_EXPORT_H

#ifdef BGCODE_BINARIZE_STATIC_DEFINE
#  define BGCODE_BINARIZE_EXPORT
#  define BGCODE_BINARIZE_NO_EXPORT
#else
#  ifndef BGCODE_BINARIZE_EXPORT
#    ifdef bgcode_binarize_EXPORTS
        /* We are building this library */
#      define BGCODE_BINARIZE_EXPORT 
#    else
        /* We are using this library */
#      define BGCODE_BINARIZE_EXPORT 
#    endif
#  endif

#  ifndef BGCODE_BINARIZE_NO_EXPORT
#    define BGCODE_BINARIZE_NO_EXPORT 
#  endif
#endif

#ifndef BGCODE_BINARIZE_DEPRECATED
#  define BGCODE_BINARIZE_DEPRECATED __declspec(deprecated)
#endif

#ifndef BGCODE_BINARIZE_DEPRECATED_EXPORT
#  define BGCODE_BINARIZE_DEPRECATED_EXPORT BGCODE_BINARIZE_EXPORT BGCODE_BINARIZE_DEPRECATED
#endif

#ifndef BGCODE_BINARIZE_DEPRECATED_NO_EXPORT
#  define BGCODE_BINARIZE_DEPRECATED_NO_EXPORT BGCODE_BINARIZE_NO_EXPORT BGCODE_BINARIZE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef BGCODE_BINARIZE_NO_DEPRECATED
#    define BGCODE_BINARIZE_NO_DEPRECATED
#  endif
#endif

#endif /* BGCODE_BINARIZE_EXPORT_H */