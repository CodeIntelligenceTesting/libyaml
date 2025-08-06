DOCKER_IMAGE ?= ciio/libci-dev

build:
	docker build -t $(DOCKER_IMAGE) .

run: build
	docker run -it --rm $(DOCKER_IMAGE) bash

prepare-git:
	rm -rf output/libci.git
	git clone ../../.git output/libci.git

libci-dist: libci-dist-ci

libci-dist-ci: prepare-git
	docker run --rm -u $$(id -u) \
	  -v"$$PWD/output:/output" \
	  -v"$$PWD/scripts:/scripts" \
	  $(DOCKER_IMAGE) /scripts/libci-dist.sh

clean:
	rm -rf output/libci.git
	rm -rf output/ci-*.*
