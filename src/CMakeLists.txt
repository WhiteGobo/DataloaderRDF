set(includepath "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(DataloaderRDF
	include/DataloaderRDF.h
	qq.c
)

target_include_directories(DataloaderRDF PUBLIC
	"${includepath}/"
)

target_link_libraries(DataloaderRDF PRIVATE
	cwalk
	Mime2Rdf4C
)

set_target_properties(DataloaderRDF PROPERTIES C_VISIBILITY_PRESET hidden)
