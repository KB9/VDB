CORE_SRC_DIR = src/core/
CORE_BIN_DIR = bin/core/
UI_SRC_DIR = src/ui/
UI_BIN_DIR = bin/ui/

core:
	cd $(CORE_SRC_DIR) && make all
	mkdir -p $(CORE_BIN_DIR)
	mv $(CORE_SRC_DIR)vdb.so $(CORE_BIN_DIR)

clean:
	rm $(CORE_BIN_DIR)vdb.so