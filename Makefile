
DEPS_CXXFLAGS=
DEPS_CXXFLAGS+=-I./thirdparty/msgflo-cpp/thirdparty/json11 -DHAVE_JSON11
DEPS_CXXFLAGS+=-I./thirdparty/msgflo-cpp/include -DHAVE_MSGFLO
CXXFLAGS:=-Wall -Werror -Wno-narrowing ${DEPS_CXXFLAGS}
CXXFLAGS+=-g

build: ./test/animation

run: ./test/animation
	./test/animation

dev:
	PATH=${PATH}:/home/${USER}/.local/bin/ when-changed arduino/animation.hpp make run


