CXX=g++-9
CXXFLAGS=-O3 -Wall -Wextra -std=c++17 -I Shared -Wno-return-type
MAINFILE=main.cpp

HELD_KARP=HeldKarp
MST2_APPROXIMATION=MST2Approximation
FARTHEST_INSERTION=FarthestInsertion

OUT_DIR="."
EXT=".out"

all: ensure_build_dir algs

algs: ${HELD_KARP} ${MST2_APPROXIMATION} ${FARTHEST_INSERTION}

${HELD_KARP}:
	${CXX} ${CXXFLAGS} -pthread "${HELD_KARP}/${MAINFILE}" -o "${OUT_DIR}/${HELD_KARP}${EXT}"

${MST2_APPROXIMATION}:
	${CXX} ${CXXFLAGS} "${MST2_APPROXIMATION}/${MAINFILE}" -o "${OUT_DIR}/${MST2_APPROXIMATION}${EXT}"

${FARTHEST_INSERTION}:
	${CXX} ${CXXFLAGS} "${FARTHEST_INSERTION}/${MAINFILE}" -o "${OUT_DIR}/${FARTHEST_INSERTION}${EXT}"

benchmark:
	./benchmark.sh

ensure_build_dir:
	mkdir -p ${OUT_DIR}

# report:
# 	cd report; make pdf1

clear:
	rm "${HELD_KARP}${EXT}" "${MST2_APPROXIMATION}${EXT}" "${FARTHEST_INSERTION}${EXT}"

.PHONY: all algs ensure_build_dir clear benchmark # report
.PHONY: ${HELD_KARP} ${MST2_APPROXIMATION} ${FARTHEST_INSERTION}
