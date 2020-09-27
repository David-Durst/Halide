include ../support/Makefile.inc

.PHONY: build clean test

build: $(BIN)/$(HL_TARGET)/stencil_chain.a

#STAGES_STR="1010"
#STAGES_STR="1010101010101001"
STAGES_STR="00000000000000000000000000000000"
#STAGES_STR="0000000000000000"
DURST_PATH=$(HALIDE_DISTRIB_PATH)/../durst/
DURST_OBJ=$(HALIDE_DISTRIB_PATH)/../durst/ClockworkExporter.o $(HALIDE_DISTRIB_PATH)/../durst/CleanVariableNames.o $(HALIDE_DISTRIB_PATH)/../durst/AffineIndexingExtractor.o $(HALIDE_DISTRIB_PATH)/../durst/VariableExtractor.o

$(GENERATOR_BIN)/stencil_chain.generator: stencil_chain_generator.cpp $(GENERATOR_DEPS) $(DURST_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I $(DURST_PATH) $(filter %.cpp,$^) $(DURST_OBJ) -o $@ $(LIBHALIDE_LDFLAGS) 

$(BIN)/%/stencil_chain.a: $(GENERATOR_BIN)/stencil_chain.generator
	@mkdir -p $(@D)
	$^ -g stencil_chain -e $(GENERATOR_OUTPUTS) -o $(@D) -f stencil_chain target=$* auto_schedule=false merge_stages=$(STAGES_STR)

$(BIN)/%/stencil_chain_auto_schedule.a: $(GENERATOR_BIN)/stencil_chain.generator
	@mkdir -p $(@D)
	$^ -g stencil_chain -e $(GENERATOR_OUTPUTS) -o $(@D) -f stencil_chain_auto_schedule target=$*-no_runtime auto_schedule=true merge_stages=$(STAGES_STR)

$(BIN)/%/process: process.cpp $(BIN)/%/stencil_chain.a $(BIN)/%/stencil_chain_auto_schedule.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS)

$(BIN)/%/out.png: $(BIN)/%/process
	@mkdir -p $(@D)
	$< $(IMAGES)/rgb.png 10 $@

clean:
	rm -rf $(BIN)

test: $(BIN)/$(HL_TARGET)/out.png
