
B=build
CXX=g++
CXX_FLAGS=-Wall -Werror -std=c++17
CC=cc
CC_FLAGS=-Wall -Werror -std=c99 -g


CXX_FILES=${wildcard *.cxx}
CXX_O_FILES=${addprefix $B/,${subst .cxx,.o,${CXX_FILES}}}

C_FILES=${wildcard *.c}
C_O_FILES=${addprefix $B/,${subst .c,.o,${C_FILES}}}

LINK=${firstword ${patsubst %.cxx,${CXX},${CXX_FILES} ${patsubst %.c,${CC},${C_FILES}}}}
LINK_FLAGS=

FUN_FILES=${wildcard *.main}
TESTS=${subst .main,.test,${FUN_FILES}}
OK_FILES=${subst .main,.ok,${FUN_FILES}}
OUT_FILES=${subst .main,.dep,${FUN_FILES}}
DIFF_FILES=${subst .main,.diff,${FUN_FILES}}
RESULT_FILES=${subst .main,.result,${FUN_FILES}}

all : $B/main

test : Makefile ${TESTS}

$B/main: ${CXX_O_FILES} ${C_O_FILES}
	@mkdir -p build
	${LINK} -o $@ ${LINK_FLAGS} ${CXX_O_FILES} ${C_O_FILES}

${CXX_O_FILES} : $B/%.o: %.cxx Makefile
	@mkdir -p build
	${CXX} -MMD -MF $B/$*.d -c -o $@ ${CXX_FLAGS} $*.cxx

${C_O_FILES} : $B/%.o: %.c Makefile
	@mkdir -p build
	${CC} -MMD -MF $B/$*.d -c -o $@ ${CC_FLAGS} $*.c

${TESTS}: %.test : Makefile %.result
	echo "$* ... $$(cat $*.result) [$$(cat $*.time)]"

${RESULT_FILES}: %.result : Makefile %.diff
	echo "unknown" > $@
	((test -s $*.diff && echo "fail") || echo "pass") > $@


${DIFF_FILES}: %.diff : Makefile %.dep %.ok
	@echo "no diff" > $@
	-diff $*.dep $*.ok > $@ 2>&1

${OUT_FILES}: %.dep : Makefile $B/main %.main
	@echo "failed to run" > $@
	-time --quiet -f '%e' -o $*.time timeout 10 $B/main $*.main > $@

-include $B/*.d

clean:
	rm -rf build
	rm -f *.diff *.result *.dep *.time
