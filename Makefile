
DEPS_CXXFLAGS=-I./rebirth
DEPS_CXXFLAGS+=-I./thirdparty/msgflo-cpp/thirdparty/json11 -DHAVE_JSON11
DEPS_CXXFLAGS+=-I./thirdparty/msgflo-cpp/include -DHAVE_MSGFLO
DEPS_LDFLAGS=
DEPS_LDFLAGS+=-L./build/install/lib/ -L./build/msgflo-cpp/thirdparty-install/amqpcpp/lib/

LDFLAGS:=${DEPS_LDFLAGS}
LDLIBS:=-lmsgflo -lmosquitto -lamqpcpp -lev -lpthread
CXXFLAGS:=-std=c++11 -Wall -Werror -Wno-narrowing -Wno-unused-function ${DEPS_CXXFLAGS}
CXXFLAGS+=-g

build: ./test/animation ./test/averager ./participants/animate ./participants/showcolor

./participants/animate: rebirth/animation.hpp
./test/animation: rebirth/animation.hpp rebirth/simulate.hpp
./test/averager: rebirth/averager.hpp

msgflo-cpp:
	mkdir -p build/msgflo-cpp/install
	cd build/msgflo-cpp && cmake -DCMAKE_INSTALL_PREFIX=../install ../../thirdparty/msgflo-cpp
	cd build/msgflo-cpp && make -j4 && make install
	cp build/msgflo-cpp/thirdparty-install/amqpcpp/lib/* build/install/lib/

dependencies: msgflo-cpp

run: ./test/animation
	LD_LIBRARY_PATH=./build/install/lib ./test/animation

check: build
	LD_LIBRARY_PATH=./build/install/lib ./test/averager
	LD_LIBRARY_PATH=./build/install/lib ./test/animation

dev:
	PATH=${PATH}:/home/${USER}/.local/bin/ when-changed arduino/animation.hpp make run

.PHONY: msgflo-cpp check
