include(CTest)

include(tools.cmake)

add_executable(DataloaderRDF_testA
	testa.c
)

target_link_libraries(DataloaderRDF_testA
	DataloaderRDF
)


set(testsuffix "DataloaderRDF::")
set(tmpbase "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(manifest "${tmpbase}/manifest.json")
DataloaderRDF_read_manifest(${manifest} testlist testlist_length base_iri)
math(EXPR range "${testlist_length} - 1")
foreach(x RANGE 0 ${range})
	unset(fileiri_suffix)
	unset(fileiri)
	string(JSON data GET ${testlist} ${x})

	string(JSON id GET ${data} "name")
	string(JSON fileiri_suffix ERROR_VARIABLE tmperror GET ${data} "iri_suffix")
	if (fileiri_suffix)
		set(fileiri "${base_iri}${fileiri_suffix}")
	else()
		string(JSON fileiri GET ${data} "iri")
	endif()
	set(testname "${testsuffix}${id}")
	string(JSON compare GET ${data} "compare")

	add_test(NAME "${testname}" COMMAND DataloaderRDF_testA
		ARGS
		"--comparedata" "${tmpbase}/${compare}"
		"--basepath" "${tmpbase}"
		"--baseiri" "${base_iri}"
		"--iri" "${fileiri}"
	)
	set_property(TEST ${testname} PROPERTY LABELS "CInterfaceRIF")
endforeach()
