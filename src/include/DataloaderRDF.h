#pragma once

#include <stdint.h>
#include <stdio.h>

#ifndef RDF_TERMTYPE_DEFINED
#define RDF_TERMTYPE_DEFINED
typedef enum {
        URI = 0,
        BNODE = 1,
        TYPEDLITERAL = 2,
        LANGLITERAL = 3
} TERMTYPE;
#endif

#ifndef TRIPLEHANDLER_DEFINED
#define TRIPLEHANDLER_DEFINED
/*
 * Use TERMTYPE for subject_type, object_type and graph_type.
 * If graphid is NULL, the default graph is used.
 */
typedef int8_t TripleHandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user);

#endif //TRIPLEHANDLER_DEFINED
       
typedef struct s_DRDFDataloader DRDFDataloader;

#if defined(_WIN32) || defined(__CYGWIN__)
#define DRDF_EXPORT __declspec(dllexport)
#define DRDF_IMPORT __declspec(dllimport)
#elif __GNUC__ >= 4
#define DRDF_EXPORT __attribute__((visibility("default")))
#define DRDF_IMPORT __attribute__((visibility("default")))
#else
#define DRDF_EXPORT
#define DRDF_IMPORT
#endif

#if defined(DRDF_SHARED)
#if defined(DRDF_EXPORTS)
#define DRDF_PUBLIC DRDF_EXPORT
#else
#define DRDF_PUBLIC DRDF_IMPORT
#endif
#else
#define DRDF_PUBLIC
#endif

#ifdef __cplusplus
extern "C"
{
#endif

DRDFDataloader* DRDFDataloader_new();
int DRDFDataloader_add_path_iri(DRDFDataloader* x, const char* basepath, const char* baseiri);
int DRDFDataloader_load_data(DRDFDataloader* x, const char* iri, TripleHandler* handler, void* user);

void fprintf_DRDFDataloader(FILE*, DRDFDataloader*);
void free_DRDFDataloader(DRDFDataloader*);

#ifdef __cplusplus
} //extern "C"
#endif
