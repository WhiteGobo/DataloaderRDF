CMAKE=cmake
SRC=./
BUILD=build
CMAKE_CONFIGURE_OPT=
CMAKE_BUILD_OPT=--verbose

default: configure build

.PHONY: configure
configure:
	${CMAKE} -S ${SRC} -B ${BUILD} ${CMAKE_CONFIGURE_OPT}

.PHONY: build
build:
	${CMAKE} --build ${BUILD} ${CMAKE_BUILD_OPT}

.PHONY: test
test:
	${MAKE} -C ${BUILD} test ARGS="${ARGS}"

.PHONY: memtest
memtest:
	${MAKE} -C ${BUILD} test ARGS="-T memcheck ${ARGS}"
	#cd ${BUILD} && env ctest -T memcheck --output-on-failure


.PHONY: clean
clean:
	-cd ${BUILD} && make clean

.PHONY: clean-full
clean-full: clean
	-rm -rf ${BUILD}
