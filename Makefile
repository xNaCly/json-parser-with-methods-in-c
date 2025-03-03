CFLAGS := -std=c23 \
	-g \
	-Wall \
	-Wextra \
	-Werror \
	-fdiagnostics-color=always \
	-fsanitize=address,undefined \
	-fno-common \
	-Winit-self \
	-Wfloat-equal \
	-Wundef \
	-Wshadow \
	-Wpointer-arith \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wstrict-overflow=5 \
	-Wwrite-strings \
	-Waggregate-return \
	-Wswitch-default \
	-Wno-discarded-qualifiers \
	-Wno-aggregate-return

FILES := $(shell find . -name "*.c")

build:
	$(CC) $(CFLAGS) $(FILES) -o jsoninc
