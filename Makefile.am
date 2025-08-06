## Run `./bootstrap` to generate the "Makefile.in" files in this directory and
## the "$SUBDIRS" subdirectories.

SUBDIRS = include src . tests

EXTRA_DIST = Changes ReadMe.md License CMakeLists.txt doc/doxygen.cfg

LIBCI_TEST_SUITE_RUN_REPO_DEFAULT := https://github.com/ci/libci
LIBCI_TEST_SUITE_RUN_REPO ?= $(LIBCI_TEST_SUITE_RUN_REPO_DEFAULT)
LIBCI_TEST_SUITE_RUN_BRANCH ?= run-test-suite

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ci-0.1.pc

maintainer-clean-local:
	rm -f aclocal.m4 config.h.in configure config/*
	-find ${builddir} -name Makefile.in -exec rm -f '{}' ';'

distclean-local:
	rm -fr tests/run-test-suite
	-git worktree prune

.PHONY: bootstrap
bootstrap: maintainer-clean
	./bootstrap
	./configure
	make

test: all
	make -C tests check-TESTS

test-suite: tests/run-test-suite all
	make -C $< test

test-all: test test-suite

tests/run-test-suite:
ifeq ($(LIBCI_TEST_SUITE_RUN_REPO),$(LIBCI_TEST_SUITE_RUN_REPO_DEFAULT))
	  -git branch --track $(LIBCI_TEST_SUITE_RUN_BRANCH) origin/$(LIBCI_TEST_SUITE_RUN_BRANCH)
	  -git worktree prune
	  git worktree add $@ $(LIBCI_TEST_SUITE_RUN_BRANCH)
    else
	  git clone --branch $(LIBCI_TEST_SUITE_RUN_BRANCH) $(LIBCI_TEST_SUITE_RUN_REPO) $@
    endif

docker-build:
	make -C pkg/docker build

docker-dist:
	make -C pkg/docker libci-dist

