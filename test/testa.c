#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "DataloaderRDF.h"

static char *basepath = NULL;
static char *baseiri = NULL;
static char *loadiri = NULL;
static char *comparedata = NULL;

static int parse_args(int argc, char *argv[]);
static int8_t my_handler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user);

int main(int argc, char *argv[]){
	int err;
	void* cntxt = NULL;
	DRDFDataloader* dataloader;
	if(0 != parse_args(argc, argv)){
		fprintf(stderr, "parsing arguments failed\n");
		exit(EXIT_FAILURE);
	}

	dataloader = DRDFDataloader_new();
	switch(DRDFDataloader_add_path_iri(dataloader, basepath, baseiri)){
		case 0:
			break;
		default:
			fprintf(stderr, "DRDFDataloader_add_path_iri failed "
					"with unkwown error\n");
			exit(EXIT_FAILURE);
	}
	err = DRDFDataloader_load_data(dataloader, loadiri, my_handler, cntxt);
	switch(err){
		case 0:
			break;
		default:
			fprintf(stderr, "DRDFDataloader_load_data failed "
					"with unhandled error\n");
			exit(EXIT_FAILURE);
	}
	free_DRDFDataloader(dataloader);

	fprintf(stderr, "No compare yet\n");
	exit(EXIT_SUCCESS);
}

static struct option parse_options[] = {
	{"basepath", required_argument, NULL, 'P'},
	{"baseiri", required_argument, NULL, 'I'},
	{"iri", required_argument, NULL, 'i'},
	{"comparedata", required_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}
};

static int parse_args(int argc, char *argv[]){
	int err = 0;
	int c = 0;
	int option_index;
	while(c != -1){
		c = getopt_long(argc, argv, "",
				parse_options, &option_index);
		switch(c){
			case -1: //end of arguments
				break;
			case 'P':
				basepath = optarg;
				break;
			case 'I':
				baseiri = optarg;
				break;
			case 'i':
				loadiri = optarg;
				break;
			case 'c':
				comparedata = optarg;
				break;
			default:
				fprintf(stderr, "unrecognized argument\n");
				err = 1;
				break;
		}
	}
	return err;
}

static int8_t my_handler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user)
{
	fprintf(stderr, "%s %s %s\n", subject, predicate, object);
	return 0;
}
