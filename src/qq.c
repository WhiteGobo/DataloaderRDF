#include "DataloaderRDF.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cwalk.h>
#include <dirent.h>
#include "Mime2Rdf4C.h"


/** https://www.manpagez.com/man/3/opendir/ **/
#include <dirent.h>

#if defined(WIN32) || defined(_WIN32) ||                                       \
  defined(__WIN32) && !defined(__CYGWIN__)
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif

typedef enum {
	RET_TLDFI_SUCCESS = 0,
	RET_TLDFI_NOT_HANDLED_IRI,
	RET_TLDFI_FAILURE,
} RET_TLDFI;

typedef struct s_Iri2Path Iri2Path;

typedef struct s_DRDFDataloader {
	Iri2Path* iri2path;
} DRDFDataloader;

typedef struct s_Iri2Path {
	char* iri;
	char* path;
	Iri2Path *next;
} Iri2Path;

struct ret_TITD {
	char* directory_path;
	const char* basefilename;
	size_t len_basefilename;
};

static void release_TITD(struct ret_TITD x){
	free(x.directory_path);
}

static int Iri2Path_config_parser(Iri2Path*, Mime2Rdf4C_ParserConfig*);
static char* load_into_memory(const char* filepath);
static int load_local_data(Iri2Path* iri2path, const char* path, const char* baseiri, TripleHandler* handler, void* user);
static int64_t handle_data(const char* data, Mime2Rdf4C_ParserConfig* config,
			TripleHandler* handler, void* user);
/*
static RET_TLDFI try_load_data_file_iri(DRDFDataloader*,
		const char* iri, size_t iri_len, Iri2Path* iri2path,
		TripleHandler* handler, void* user);
		*/
static struct ret_TITD transform_iri_to_directory_path(
		const char* iri, size_t iri_len, Iri2Path* iri2path);
static char* my_join_path(const char* first, const char* second);
static void free_Iri2Path(Iri2Path*);

/**
 * Find last part of path in iri.
 * iri: http://base.iri/some/path/file
 *      |                        |      
 *     iri                     return
 */
static const char* find_url_last_part_of_path(const char* iri);


DRDFDataloader* DRDFDataloader_new(){
	DRDFDataloader* ret = malloc(sizeof(DRDFDataloader));
	if(ret == NULL){
		fprintf(stderr, "CRITICAL ERROR: malloc failed\n");
		return NULL;
	}
	ret->iri2path = NULL;
	return ret;
}

void free_DRDFDataloader(DRDFDataloader* x){
	if (x == NULL) return;
	free_Iri2Path(x->iri2path);
	free(x);
}

static void free_Iri2Path(Iri2Path* x){
	if(x == NULL) return;
	free_Iri2Path(x->next);
	free(x->iri);
	free(x->path);
	free(x);
}

static Iri2Path* Iri2Path_new(const char* path, const char* iri){
	size_t len_path;
	Iri2Path* ret = malloc(sizeof(Iri2Path));
	if (ret == NULL) return NULL;
	ret->iri = malloc(strlen(iri) + 1);
	strcpy(ret->iri, iri);
	len_path = strlen(path);
	if (path[len_path - 1] == SEPARATOR){
		ret->path = malloc(len_path + 1);
		memcpy(ret->path, path, len_path);
		ret->path[len_path] = '\0';
	} else {
		ret->path = malloc(len_path + 2);
		memcpy(ret->path, path, len_path);
		ret->path[len_path] = SEPARATOR;
		ret->path[len_path + 1] = '\0';
	}
	ret->next = NULL;
	return ret;
}

int DRDFDataloader_add_path_iri(DRDFDataloader* x, const char* basepath, const char* baseiri)
{
	if(x == NULL) return 1;
	Iri2Path* first = Iri2Path_new(basepath, baseiri);
	if(first == NULL) return 1;
        if (x->iri2path == NULL){
                x->iri2path = first;
        } else {
                Iri2Path* ip = x->iri2path;
                while(ip->next != NULL){
                        ip = ip->next;
                }
                ip->next = first;
        }	
	return 0;
}

static struct ret_TITD transform_iri_to_directory_path(
		const char* iri, size_t iri_len, Iri2Path* iri2path)
{
	size_t iri2path_iri_len, path_len;
	struct ret_TITD ret = {
		.basefilename = NULL,
		.directory_path = NULL,
		.len_basefilename = 0,
	};
	iri2path_iri_len = strlen(iri2path->iri);
	//Check if iri is sub of iri in iri2path:
	if (iri_len < iri2path_iri_len) {
		ret.directory_path = NULL;
		return ret;
	}
	if (0 != memcmp(iri, iri2path->iri, iri2path_iri_len)){
		ret.directory_path = NULL;
		return ret;
	}
	//create directory_path
	path_len = strlen(iri2path->path) + strlen(iri) - strlen(iri2path->iri) + 10;

	size_t offset, iri2path_path_len;
	iri2path_path_len = strlen(iri2path->path);
	// eg:
	// iri2path->iri: http://base.iri/
	// iri: http://base.iri/some/path/file
	//      |               |        |      
	//     iri           xstart     xend
	const char* xstart = iri + iri2path_iri_len;
	const char* xend = find_url_last_part_of_path(iri);
	if (xend == NULL){
		return ret;
	}
	ret.basefilename = xend + 1;
	ret.len_basefilename = strlen(ret.basefilename);
	path_len = iri2path_path_len + (xend - iri) + 3;
	ret.directory_path = malloc(path_len);
	memcpy(ret.directory_path, iri2path->path, iri2path_path_len);
	if (xend <= xstart){
		ret.directory_path[iri2path_path_len] = '\0';
	} else {
		char* dst = ret.directory_path + iri2path_path_len;
		//if linux
		memcpy(dst, xstart, xend - xstart + 1);
		dst[xend - xstart + 1] = '\0';
		//if windows:
		//for (const char* x = iri + iri2path_iri_len; x != l; x++){
		//}
	}
	return ret;
}

static char* find_feasible_file(Iri2Path* start_iri2path, const char* iri){

	struct ret_TITD ret_titd;
	size_t liri;
	//find local files
	liri = strlen(iri);
	for (Iri2Path* iri2path = start_iri2path;
			iri2path != NULL;
			iri2path = iri2path->next)
	{
		ret_titd = transform_iri_to_directory_path(iri, liri, iri2path);
		if(ret_titd.directory_path != NULL){
			break;
		}
	}
	if(ret_titd.directory_path == NULL){
		fprintf(stderr, "ERROR DRDFDataloader_load_data(iri not "
				"handled %s)\n", iri);
		return NULL;
	}
	const char* basefilename = ret_titd.basefilename;
	size_t len_basefilename = ret_titd.len_basefilename;
	const char* directory_path = ret_titd.directory_path;


	DIR* searchdir;
	//Iri is handled by this iri2path. Try to find file:
	searchdir = opendir(directory_path);
	if (searchdir == NULL){
		release_TITD(ret_titd);
		return NULL;
	}
	char* path;
	size_t namelen;
	size_t _extlength;
	const char* fileext;
	for (struct dirent* dp = readdir(searchdir);
			dp != NULL;
			dp = readdir(searchdir))
	{
		switch(dp->d_type){
			case DT_REG:
				break;
			default:
				continue;
		}
		namelen = strlen(dp->d_name);
		if (namelen < len_basefilename){
			continue;
		}
		if (0 != memcmp(basefilename, dp->d_name, len_basefilename)){
			continue;
		}
		if (namelen == len_basefilename){
			//if iri is translated exactly to a file always
			//return failure even if no parser can be found.
			if (!cwk_path_get_extension(basefilename, &fileext, &_extlength)){
				fprintf(stderr, "Internal Error (find_feasible_file)\n");
				closedir(searchdir);
				release_TITD(ret_titd);
				return NULL;
			}
			path = my_join_path(directory_path, dp->d_name);
			closedir(searchdir);
			return path;
		}
		//Expect to find extension of file
		if (!cwk_path_get_extension(dp->d_name, &fileext, &_extlength)){
			continue;
		}
		//File extension must be located directly behind basefilename
		if (fileext != dp->d_name + len_basefilename){
			continue;
		}
		path = my_join_path(directory_path, dp->d_name);
		closedir(searchdir);
		release_TITD(ret_titd);
		return path;
	}
	release_TITD(ret_titd);
	closedir(searchdir);
	return NULL;
}


int DRDFDataloader_load_data(DRDFDataloader* x, const char* iri, TripleHandler* handler, void* user)
{

	char* path;
	path = find_feasible_file(x->iri2path, iri);
	if(path == NULL){
		//find feasible should print what the error is
		return 1;
	}
	int ret = 1;
	switch(load_local_data(x->iri2path, path, iri, handler, user)){
		case RET_TLDFI_SUCCESS:
			ret = 0;
			break;
		case RET_TLDFI_FAILURE:
		case RET_TLDFI_NOT_HANDLED_IRI:
		default:
			fprintf(stderr, "ERROR DRDFDataloader failed to load "
					"data for uri %s from %s\n", iri, path);
			ret = 1;
	}
	return ret;
}

static char* load_into_memory(const char* filepath){
        char *ret;
        long fsize;
        FILE *f = fopen(filepath, "rb");
        if (f == NULL) return NULL;
        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        rewind(f);
        //fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

        ret = malloc(fsize + 1);
        fread(ret, fsize, 1, f);
        ret[fsize] = 0;
        fclose(f);
        return ret;
}

static int Iri2Path_config_parser(Iri2Path* start_iri2path, Mime2Rdf4C_ParserConfig* parser)
{
	int err;
	for (Iri2Path* iri2path = start_iri2path;
			iri2path != NULL;
			iri2path = iri2path->next)
	{
		err = Mime2Rdf4C_enable_LoadDocumentCallback_for_relativefiles(
				parser, iri2path->iri, iri2path->path);
		if(err != 0){
			return 1;
		}
	}
	return 0;
}

static int load_local_data(Iri2Path* iri2path, const char* path, const char* baseiri, TripleHandler* handler, void* user)
{
	int err;
	char *tmpinput;
	const char* ext;
	Mime2Rdf4C_ParserConfig* config;
	const char* found_ext;
	size_t _ext_length;
	if (!cwk_path_get_extension(path, &found_ext, &_ext_length)){
		//shouldnt be feasible file if extension isnt recognized
		fprintf(stderr, "INTERNAL ERROR load_local_data(extension)\n");
		return 1;
	}
	config = Mime2Rdf4C_get_parser_from_ext(found_ext + 1);
	if (config == NULL){
		fprintf(stderr, "INTERNAL ERROR load_local_data(extension2)\n");
		return 2;
	}

	err = Iri2Path_config_parser(iri2path, config);
	if(err != 0){
		fprintf(stderr, "INTERNAL ERROR DRDF load_local_data "
				"(add iri2path)\n");
		return 4;
	}
	err = Mime2Rdf4C_set_baseiri(config, baseiri);
	if(err != 0){
		fprintf(stderr, "INTERNAL ERROR DRDFload_local_data(baseiri)\n");
		return 5;
	}

	tmpinput = load_into_memory(path);
	if (tmpinput == NULL){
		fprintf(stderr, "INTERNAL ERROR load_local_data(file)\n");
		return 3;
	}

	err = handle_data(tmpinput, config, handler, user);
	free_Mime2Rdf4CParserConfig(config);
	free(tmpinput);
	if (err == 0){
		return 0;
	} else {
		return 4;
	}
}

static int64_t handle_data(const char* data, Mime2Rdf4C_ParserConfig* config,
			TripleHandler* handler, void* user)
{
	int64_t err;
	err = Mime2Rdf4C_parse(data, handler, user, config);
	return err;
}

static char* my_join_path(const char* first, const char* second){
	char* ret;
	size_t lfirst, lsecond, lbuffer, tmpi;
	lfirst = strlen(first);
	lsecond = strlen(second);
	lbuffer = lfirst + lsecond + 3;
	ret = malloc(lbuffer + 3);
	if (ret == NULL){
		return NULL;
	}
	tmpi = cwk_path_join(first, second, ret, lbuffer);
	return ret;
}

static const char* find_url_last_part_of_path(const char* iri){
	const char* ret = NULL;
	if (iri == NULL) return NULL;
	for(const char* x=iri; x[0] != '\0'; x++){
		switch(*x){
			case '/':
				ret = x;
				break;
			case '?': //reached query
			case '#':
				return ret;
		}
	}
	return ret;
}


void fprintf_DRDFDataloader(FILE* f, DRDFDataloader* data){
	if(f == NULL) return;
	if(data == NULL){
		fprintf(f, "(NULL)");
		return;
	}
	fprintf(f, "DRDFDataloader[");
	for (Iri2Path* x=data->iri2path; x!=NULL; x=x->next){
		fprintf(f, "%s=>%s;", x->iri, x->path);
	}
	fprintf(f, "]");
}
