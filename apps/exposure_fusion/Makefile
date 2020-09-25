include ../support/Makefile.inc

.PHONY: build clean test

X_MAX_PARAM ?= 1536
Y_MAX_PARAM ?= 2560
RGB_FILE ?= rgba.png
DURST_PATH=$(HALIDE_DISTRIB_PATH)/../durst/
DURST_OBJ=$(HALIDE_DISTRIB_PATH)/../durst/ClockworkExporter.o $(HALIDE_DISTRIB_PATH)/../durst/CleanVariableNames.o $(HALIDE_DISTRIB_PATH)/../durst/AffineIndexingExtractor.o $(HALIDE_DISTRIB_PATH)/../durst/VariableExtractor.o

build: $(BIN)/$(HL_TARGET)/exposure.a

build_auto: $(BIN)/$(HL_TARGET)/exposure_auto_schedule.a

test: $(BIN)/$(HL_TARGET)/out.png

prof: $(BIN)/$(HL_TARGET)/out_prof.png

$(GENERATOR_BIN)/exposure.generator: exposure_generator.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I $(DURST_PATH) $(filter %.cpp,$^) $(DURST_OBJ) -o $@ $(LIBHALIDE_LDFLAGS) 

$(BIN)/%/exposure.a: $(GENERATOR_BIN)/exposure.generator
	@mkdir -p $(@D)
	$< -g exposure -f exposure -o $(BIN)/$* target=$*-no_runtime auto_schedule=false x_max_param=$(X_MAX_PARAM) y_max_param=$(Y_MAX_PARAM) 

$(BIN)/%/exposure_auto_schedule.a: $(GENERATOR_BIN)/exposure.generator
	@mkdir -p $(@D)
	$< -g exposure -f exposure_auto_schedule -o $(BIN)/$* target=$*-no_runtime auto_schedule=true

$(BIN)/%/runtime.a: $(GENERATOR_BIN)/exposure.generator
	@mkdir -p $(@D)
	$< -r runtime -o $(BIN)/$* target=$*

$(BIN)/%/process: process.cpp $(BIN)/%/exposure.a $(BIN)/%/exposure_auto_schedule.a $(BIN)/%/runtime.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN)/$* -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS)

$(BIN)/%/out.png: $(BIN)/%/process
	$< ../images/rgba.png $(BIN)/$*/out.png

$(BIN)/%/out_prof.png: $(BIN)/%/process
	nvprof --normalized-time-unit us $< ../images/$(RGB_FILE) $(BIN)/$*/out.png

clean:
	rm -rf $(BIN)
