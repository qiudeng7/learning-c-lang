DOCKER = docker
BUILD_IMAGE = alpine:3.23
ALPINE_MIRROR = https://dl-cdn.alpinelinux.org/alpine
RAW_IMAGE = simple-http-server:raw
FRAMEWORK_IMAGE = simple-http-server:framework

.PHONY: all raw framework run-raw run-framework clean
all: raw framework
raw:
	$(DOCKER) build --build-arg BUILD_IMAGE=$(BUILD_IMAGE) --build-arg ALPINE_MIRROR=$(ALPINE_MIRROR) --target raw -t $(RAW_IMAGE) .
framework:
	$(DOCKER) build --build-arg BUILD_IMAGE=$(BUILD_IMAGE) --build-arg ALPINE_MIRROR=$(ALPINE_MIRROR) --target framework -t $(FRAMEWORK_IMAGE) .
run-raw:
	$(DOCKER) run --rm -p 127.0.0.1:8080:8080 $(RAW_IMAGE)
run-framework:
	$(DOCKER) run --rm -p 127.0.0.1:8081:8081 $(FRAMEWORK_IMAGE)
clean:
	$(DOCKER) image rm $(RAW_IMAGE) $(FRAMEWORK_IMAGE)
