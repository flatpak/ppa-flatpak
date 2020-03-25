#ifndef __VAR___FLATPAK_VARIANTS_GV__H__
#define __VAR___FLATPAK_VARIANTS_GV__H__
/* generated code for flatpak-variants.gv */
#include <string.h>
#include <glib.h>

/********** Basic types *****************/

typedef struct {
 gconstpointer base;
 gsize size;
} VarRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarVariantRef;

#define VAR_METADATA_TYPESTRING "a{sv}"
#define VAR_METADATA_TYPEFORMAT ((const GVariantType *) VAR_METADATA_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarMetadataRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarMetadataEntryRef;

#define VAR_CHECKSUM_TYPESTRING "ay"
#define VAR_CHECKSUM_TYPEFORMAT ((const GVariantType *) VAR_CHECKSUM_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarChecksumRef;

#define VAR_REF_INFO_TYPESTRING "(taya{sv})"
#define VAR_REF_INFO_TYPEFORMAT ((const GVariantType *) VAR_REF_INFO_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarRefInfoRef;

#define VAR_REF_INFO_INDEXOF_COMMIT_SIZE 0

#define VAR_REF_INFO_INDEXOF_CHECKSUM 1

#define VAR_REF_INFO_INDEXOF_METADATA 2

#define VAR_REF_MAP_ENTRY_TYPESTRING "(s(taya{sv}))"
#define VAR_REF_MAP_ENTRY_TYPEFORMAT ((const GVariantType *) VAR_REF_MAP_ENTRY_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarRefMapEntryRef;

#define VAR_REF_MAP_ENTRY_INDEXOF_REF 0

#define VAR_REF_MAP_ENTRY_INDEXOF_INFO 1

#define VAR_REF_MAP_TYPESTRING "a(s(taya{sv}))"
#define VAR_REF_MAP_TYPEFORMAT ((const GVariantType *) VAR_REF_MAP_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarRefMapRef;

#define VAR_SUMMARY_TYPESTRING "(a(s(taya{sv}))a{sv})"
#define VAR_SUMMARY_TYPEFORMAT ((const GVariantType *) VAR_SUMMARY_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarSummaryRef;

#define VAR_SUMMARY_INDEXOF_REF_MAP 0

#define VAR_SUMMARY_INDEXOF_METADATA 1

#define VAR_COLLECTION_MAP_TYPESTRING "a{sa(s(taya{sv}))}"
#define VAR_COLLECTION_MAP_TYPEFORMAT ((const GVariantType *) VAR_COLLECTION_MAP_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarCollectionMapRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarCollectionMapEntryRef;

#define VAR_RELATED_TYPESTRING "(say)"
#define VAR_RELATED_TYPEFORMAT ((const GVariantType *) VAR_RELATED_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarRelatedRef;

#define VAR_RELATED_INDEXOF_REF 0

#define VAR_RELATED_INDEXOF_COMMIT 1

#define VAR_ARRAYOF_RELATED_TYPESTRING "a(say)"
#define VAR_ARRAYOF_RELATED_TYPEFORMAT ((const GVariantType *) VAR_ARRAYOF_RELATED_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarArrayofRelatedRef;

#define VAR_COMMIT_TYPESTRING "(a{sv}aya(say)sstayay)"
#define VAR_COMMIT_TYPEFORMAT ((const GVariantType *) VAR_COMMIT_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarCommitRef;

#define VAR_COMMIT_INDEXOF_METADATA 0

#define VAR_COMMIT_INDEXOF_PATENT 1

#define VAR_COMMIT_INDEXOF_RELATED 2

#define VAR_COMMIT_INDEXOF_SUBJECT 3

#define VAR_COMMIT_INDEXOF_BODY 4

#define VAR_COMMIT_INDEXOF_TIMESTAMP 5

#define VAR_COMMIT_INDEXOF_ROOT_CONTENTS 6

#define VAR_COMMIT_INDEXOF_ROOT_METADATA 7

#define VAR_CACHE_DATA_TYPESTRING "(tts)"
#define VAR_CACHE_DATA_TYPEFORMAT ((const GVariantType *) VAR_CACHE_DATA_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarCacheDataRef;

#define VAR_CACHE_DATA_INDEXOF_INSTALLED_SIZE 0

#define VAR_CACHE_DATA_INDEXOF_DOWNLOAD_SIZE 1

#define VAR_CACHE_DATA_INDEXOF_METADATA 2

#define VAR_CACHE_TYPESTRING "a{s(tts)}"
#define VAR_CACHE_TYPEFORMAT ((const GVariantType *) VAR_CACHE_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarCacheRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarCacheEntryRef;

#define VAR_SPARSE_CACHE_TYPESTRING "a{sa{sv}}"
#define VAR_SPARSE_CACHE_TYPEFORMAT ((const GVariantType *) VAR_SPARSE_CACHE_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarSparseCacheRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarSparseCacheEntryRef;

#define VAR_COMMITS_CACHE_TYPESTRING "aay"
#define VAR_COMMITS_CACHE_TYPEFORMAT ((const GVariantType *) VAR_COMMITS_CACHE_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarCommitsCacheRef;

#define VAR_ARRAYOFSTRING_TYPESTRING "as"
#define VAR_ARRAYOFSTRING_TYPEFORMAT ((const GVariantType *) VAR_ARRAYOFSTRING_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarArrayofstringRef;

#define VAR_DEPLOY_DATA_TYPESTRING "(ssasta{sv})"
#define VAR_DEPLOY_DATA_TYPEFORMAT ((const GVariantType *) VAR_DEPLOY_DATA_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarDeployDataRef;

#define VAR_DEPLOY_DATA_INDEXOF_ORIGIN 0

#define VAR_DEPLOY_DATA_INDEXOF_COMMIT 1

#define VAR_DEPLOY_DATA_INDEXOF_SUBPATHS 2

#define VAR_DEPLOY_DATA_INDEXOF_INSTALLED_SIZE 3

#define VAR_DEPLOY_DATA_INDEXOF_METADATA 4

#define VAR_RATINGS_TYPESTRING "a{ss}"
#define VAR_RATINGS_TYPEFORMAT ((const GVariantType *) VAR_RATINGS_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarRatingsRef;

typedef struct {
 gconstpointer base;
 gsize size;
} VarRatingsEntryRef;

#define VAR_CONTENT_RATING_TYPESTRING "(sa{ss})"
#define VAR_CONTENT_RATING_TYPEFORMAT ((const GVariantType *) VAR_CONTENT_RATING_TYPESTRING)

typedef struct {
 gconstpointer base;
 gsize size;
} VarContentRatingRef;

#define VAR_CONTENT_RATING_INDEXOF_RATING_TYPE 0

#define VAR_CONTENT_RATING_INDEXOF_RATINGS 1

#endif
