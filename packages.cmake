include(FetchContent)

FetchContent_Declare(
	Mime2Rdf4C
	GIT_REPOSITORY https://github.com/WhiteGobo/Mime2Rdf4C.git
	#GIT_TAG 8b244ca1826c024a641d98f0bb52e3c649e79972
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(Mime2Rdf4C)

FetchContent_Declare(
        cwalk
        GIT_REPOSITORY https://github.com/likle/cwalk.git
	GIT_TAG v1.2.9
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(cwalk)
