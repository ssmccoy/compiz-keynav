.DEFAULT: all

.PHONY: install all

BUILD_DIR := build

all: $(BUILD_DIR)/libkeynav.so

$(BUILD_DIR)/libkeynav.so: $(BUILD_DIR) src/keynav.cpp src/keynav.h
	$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR): CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..

install: all
	$(MAKE) -C $(BUILD_DIR) install


clean:
	rm -r $(BUILD_DIR)
