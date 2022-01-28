CC:=gcc
CFLAGS:=-std=c99 -Wall -Wextra -Isrc $(GSL_PATH:%=-I%/include) -gdwarf-4 --coverage
LDFLAGS:=$(GSL_PATH:%=-L%/lib) -lgsl -lgslcblas -lm

PROJECTS:=$(shell find src/* -type d)
CLIENTS:=$(filter-out src/server src/tests% src/clients src/clients/d_%,$(PROJECTS))
CLIENTS:=$(CLIENTS:src/clients/%=%)
SERVER:=server

SRC:=$(filter-out src/tests/% src/clients/d_%, $(shell find src/ -type f -name *.c))
TSRC:=$(filter-out src/clients% src/server/server.c src/clients/d_%, $(shell find src/ -type f -name *.c))
OBJ:=$(SRC:%.c=build/obj/%.o)
obj:=$(TSRC:%.c=build/obj/%.o)
DEP:=$(SRC:%=build/dep/%.d)
dep:=$(TSRC:%=build/dep/%.d)

DEPFLAGS= -MT $@ -MMD -MP -MF

.PHONY: all build install clean test

all: build

build: $(CLIENTS:%=build/%.so) $(SERVER:%=build/%)

build/obj/src/%.o: src/%.c
	@echo Compilation source client $*.c
	mkdir -p build/obj/src/$(*D)
	mkdir -p build/dep/src/$(*D)
	rm -f build/obj/src/$*.gcda
	$(CC) $(DEPFLAGS) build/dep/src/$*.d $(CFLAGS) -Isrc/$(*D) -fPIC -c $< -o $@ 

build/obj/src/clients/%.o: src/clients/%.c
	@echo Compilation source client clients/$*.c
	mkdir -p build/obj/src/clients/$(*D)
	mkdir -p build/dep/src/clients/$(*D)
	rm -f build/obj/src/clients/$*.gcda
	$(CC) $(DEPFLAGS) build/dep/src/clients/$*.d $(CFLAGS) -Isrc/clients/$(*D) -Isrc/clients -fPIC -c $< -o $@ 

build/%.so: $(OBJ)
	@echo "Édition des liens client " $@
	$(CC) $(CFLAGS) build/obj/src/*.o build/obj/src/clients/*.o build/obj/src/clients/$*/*.o -shared -o $@ $(LDFLAGS) -fPIC 


build/server: $(OBJ)
	@echo "Édition des liens server " $@
	rm -rf $(find build -name *.gcda)
	$(CC) $(CFLAGS) build/obj/src/*.o build/obj/src/server/*.o -o $@ $(LDFLAGS) -ldl 

build/obj/src/tests/%.o: src/tests/%.c
	@echo Compilation source Tclient tests/$*.c
	mkdir -p build/obj/src/tests/$(*D)
	mkdir -p build/dep/src/tests/$(*D)
	rm -f build/obj/src/tests/$*.gcda
	$(CC) $(DEPFLAGS) build/dep/src/tests/$*.d $(CFLAGS) -Isrc/$(*D) -Isrc/tests/$(*D) -Isrc/clients -fPIC -c $< -o $@

build/alltests: $(obj)
	@echo "Édition des liens tests " $@
	rm -rf $(find build -name *.gcda)
	$(CC) $(CFLAGS) -Isrc $(obj) -o $@ $(LDFLAGS) -ldl 

test: build/alltests

coverage:
	lcov -c -d . -o coverage.info
	genhtml	coverage.info -o coverage

install:
	@echo "Installation"
	cp build/server install/
	[ -e build/alltests ] && cp build/alltests install/ || true
	cp build/*.so install/
	
clean:
	@echo "Nettoyage"
	rm -rf build install/*
	rm -rf coverage

$(DEBUG).SILENT:

$(DEP) $(dep):
include $(wildcard $(DEP) $(dep))
