CXX=g++-9
CXXFLAGS=-O3 -Wall -Wextra -std=c++17 -I Shared
MAINFILE=main.cpp

HELD_KARP=HeldKarp
MST2_APPROXIMATION=MST2Approximation

OUT_DIR="."
EXT="out"

all: ensure_build_dir algs

algs: ${HELD_KARP} ${MST2_APPROXIMATION}

${HELD_KARP}:
	${CXX} ${CXXFLAGS} ${HELD_KARP}/${MAINFILE} -o ${OUT_DIR}/${HELD_KARP}.${EXT}

${MST2_APPROXIMATION}:
	${CXX} ${CXXFLAGS} ${MST2_APPROXIMATION}/${MAINFILE} -o ${OUT_DIR}/${MST2_APPROXIMATION}.${EXT}

ensure_build_dir:
	mkdir -p ${OUT_DIR}

# report:
# 	cd report; make pdf1

.PHONY: all algs ensure_build_dir
.PHONY: ${HELD_KARP} ${MST2_APPROXIMATION} # report
