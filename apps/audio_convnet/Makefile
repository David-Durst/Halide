HALIDE_DIR ?= ../../distrib

.PHONY: build clean test

all: bin/convnet_generator bin/convnet.a #bin/run test

HL_TARGET ?= host
CHANNELS ?= 256
LAYERS ?= 32
BATCH_SIZE ?= 60
SAMPLES ?= 120000
ARM_ARCH ?= false
ARM_ADDR ?= xilinx@192.168.3.1

DURST_PATH=$(HALIDE_DIR)/../durst/
DURST_OBJ=$(HALIDE_DIR)/../durst/ClockworkExporter.o $(HALIDE_DIR)/../durst/CleanVariableNames.o $(HALIDE_DIR)/../durst/AffineIndexingExtractor.o $(HALIDE_DIR)/../durst/VariableExtractor.o

bin/convnet_generator: convnet_generator.cpp $(DURST_OBJ)
	mkdir -p $(@D)
	c++ -g $(HALIDE_DIR)/tools/GenGen.cpp convnet_generator.cpp -I $(HALIDE_DIR)/include -I $(DURST_PATH) -L $(HALIDE_DIR)/bin -lHalide -Wl,-rpath $(HALIDE_DIR)/bin $(DURST_OBJ) -o bin/convnet_generator

bin/convnet.a: bin/convnet_generator
	./bin/convnet_generator -g convnet -o bin -e static_library,registration,stmt,assembly,llvm_assembly,h target=x86-64-linux-avx2-disable_llvm_loop_opt channels=$(CHANNELS) layers=$(LAYERS) batch_size=$(BATCH_SIZE) arm_arch=$(ARM_ARCH)

bin/run: run.cpp bin/convnet.a
	g++ -O3 -march=skylake -mavx2 -I $(HALIDE_DIR)/tools -I $(HALIDE_DIR)/include run.cpp bin/convnet.a -lpthread -ldl -o bin/run 

test: bin/run
	./bin/run $(CHANNELS) $(SAMPLES) $(LAYERS)
	./bin/run $(CHANNELS) $(SAMPLES) $(LAYERS)
	./bin/run $(CHANNELS) $(SAMPLES) $(LAYERS)
	./bin/run $(CHANNELS) $(SAMPLES) $(LAYERS)
	./bin/run $(CHANNELS) $(SAMPLES) $(LAYERS)

bin/convnet_arm.a: bin/convnet_generator
	./bin/convnet_generator -g convnet -o bin -e static_library,registration,stmt,assembly,llvm_assembly,h target=arm-64-linux-disable_llvm_loop_opt channels=$(CHANNELS) batch_size=$(BATCH_SIZE) layers=$(LAYERS)

bin/run_arm: run.cpp bin/convnet_arm.a
	aarch64-linux-gnu-g++ -O3 -I $(HALIDE_DIR)/tools -I $(HALIDE_DIR)/include run.cpp bin/convnet.a -lpthread -ldl -o bin/run_arm

test_arm: bin/run_arm
	scp bin/run_arm $(ARM_ADDR): 
	ssh $(ARM_ADDR) "./run_arm $(CHANNELS) $(SAMPLES) $(LAYERS)"
	ssh $(ARM_ADDR) "./run_arm $(CHANNELS) $(SAMPLES) $(LAYERS)"
	ssh $(ARM_ADDR) "./run_arm $(CHANNELS) $(SAMPLES) $(LAYERS)"
	ssh $(ARM_ADDR) "./run_arm $(CHANNELS) $(SAMPLES) $(LAYERS)"
	ssh $(ARM_ADDR) "./run_arm $(CHANNELS) $(SAMPLES) $(LAYERS)"

clean:
	rm -f bin/*
