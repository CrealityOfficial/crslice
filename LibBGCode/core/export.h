
#ifndef BGCODE_CORE_EXPORT_H
#define BGCODE_CORE_EXPORT_H

#ifdef BGCODE_CORE_STATIC_DEFINE
#  define BGCODE_CORE_EXPORT
#  define BGCODE_CORE_NO_EXPORT
#else
#  ifndef BGCODE_CORE_EXPORT
#    ifdef bgcode_core_EXPORTS
        /* We are building this library */
#      define BGCODE_CORE_EXPORT 
#    else
        /* We are using this library */
#      define BGCODE_CORE_EXPORT 
#    endif
#  endif

#  ifndef BGCODE_CORE_NO_EXPORT
#    define BGCODE_CORE_NO_EXPORT 
#  endif
#endif

#ifndef BGCODE_CORE_DEPRECATED
#  define BGCODE_CORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef BGCODE_CORE_DEPRECATED_EXPORT
#  define BGCODE_CORE_DEPRECATED_EXPORT BGCODE_CORE_EXPORT BGCODE_CORE_DEPRECATED
#endif

#ifndef BGCODE_CORE_DEPRECATED_NO_EXPORT
#  define BGCODE_CORE_DEPRECATED_NO_EXPORT BGCODE_CORE_NO_EXPORT BGCODE_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef BGCODE_CORE_NO_DEPRECATED
#    define BGCODE_CORE_NO_DEPRECATED
#  endif
#endif

#endif /* BGCODE_CORE_EXPORT_H */
