function(DataloaderRDF_read_manifest manifest_path var_testarray_json var_testarray_length var_base_iri)

	file(READ "${manifest_path}" MANIFEST_STRING)
	string(JSON tmp
		GET ${MANIFEST_STRING} sequence
	)
	set(${var_testarray_json} ${tmp})

	string(JSON tmp
		GET ${MANIFEST_STRING} baseIri
	)
	set(${var_base_iri} ${tmp})

	string(JSON ${var_testarray_length}
		LENGTH ${${var_testarray_json}}
	)

	return(PROPAGATE
		${var_testarray_json}
		${var_testarray_length}
		${var_base_iri})
endfunction()
