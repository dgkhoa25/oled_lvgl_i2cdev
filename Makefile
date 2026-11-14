CROSS_COMPILE ?= /home/fadn/work/BBB/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc

LVGL_DIR_NAME = lvgl
LVGL_DIR      = $(CURDIR)
BUILD_DIR     = $(CURDIR)/build

CFLAGS  = -O2 -Wall -MMD -MP -I$(LVGL_DIR) -I$(LVGL_DIR)/$(LVGL_DIR_NAME) -DLV_CONF_INCLUDE_SIMPLE
LDFLAGS = -static

# Chỉ build phần lõi của LVGL (không build examples/demos)
CSRCS := $(shell find $(LVGL_DIR)/$(LVGL_DIR_NAME)/src -name '*.c')

SRCS = $(CSRCS) $(LVGL_DIR)/main.c $(LVGL_DIR)/oled.c $(LVGL_DIR)/ui.c
OBJS   = $(patsubst $(LVGL_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS   = $(OBJS:.o=.d)
TARGET = $(BUILD_DIR)/oled_lvgl

all: $(TARGET)

$(BUILD_DIR)/%.o: $(LVGL_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)

.PHONY: all clean