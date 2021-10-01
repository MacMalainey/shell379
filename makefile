# Compiler options
CC     := gcc
CFLAGS := -Wall -O2

# Path options
BUILD_DIR := build
SRC_DIR   := src

# Project specific options
EXECUTABLE := shell379
SOURCES    := main.c \
			  interface.c \
			  process_manager.c

# Submission options
ARCHIVE   := shell379.zip
ARTIFACTS := makefile
ARTIFACTS += $(SRC_DIR)

# Generated settings
OBJS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))

ifdef $(DEBUG)
CFLAGS += -D DEBUG
endif

$(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

compile: $(OBJS)

link: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJS)

debug: CFLAGS += -DDEBUG
debug: link;

clean:
	rm $(EXECUTABLE) $(ARCHIVE) $(BUILD_DIR)/ -R

compress:
	zip $(ARCHIVE) -r $(ARTIFACTS)
