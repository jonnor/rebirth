
#DEPS_CXXFLAGS=-I./thirdparty/json11 -DHAVE_JSON11
CXXFLAGS:=-Wall -Werror -Wno-narrowing ${DEPS_CXXFLAGS}
CXXFLAGS+=-g

build: ./test/animation

run: ./test/animation
	./test/animation

dev:
	PATH=${PATH}:/home/${USER}/.local/bin/ when-changed arduino/animation.hpp make run


