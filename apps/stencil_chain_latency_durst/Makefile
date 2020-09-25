include ../support/Makefile.inc

.PHONY: build clean test

build: $(BIN)/$(HL_TARGET)/process

HL_TARGET ?= host-cuda
X_SIZE ?= 100
STENCILS ?= 32
MULADDS ?= 1
STENCIL_SIZE ?= 5

$(GENERATOR_BIN)/stencil_chain.generator: stencil_chain_generator.cpp $(GENERATOR_DEPS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(filter %.cpp,$^) -o $@ $(LIBHALIDE_LDFLAGS) 

$(BIN)/%/stencil_chain.a: $(GENERATOR_BIN)/stencil_chain.generator
	@mkdir -p $(@D)
	$^ -g stencil_chain -e $(GENERATOR_OUTPUTS) -o $(@D) -f stencil_chain target=$* auto_schedule=false stencils=$(STENCILS) muladds=$(MULADDS) stencil_size=$(STENCIL_SIZE) x_size=$(X_SIZE)

$(BIN)/%/stencil_chain_auto_schedule.a: $(GENERATOR_BIN)/stencil_chain.generator
	@mkdir -p $(@D)
	$^ -g stencil_chain -e $(GENERATOR_OUTPUTS) -o $(@D) -f stencil_chain_auto_schedule target=$*-no_runtime auto_schedule=true stencils=$(STENCILS) muladds=$(MULADDS) stencil_size=$(STENCIL_SIZE) x_size=$(X_SIZE)

$(BIN)/%/process: process.cpp $(BIN)/%/stencil_chain.a $(BIN)/%/stencil_chain_auto_schedule.a
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(BIN)/$* -Wall $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS) $(CUDA_LDFLAGS) $(OPENCL_LDFLAGS) $(OPENGL_LDFLAGS)

test: $(BIN)/$(HL_TARGET)/process
	$< $(X_SIZE)

clean:
	rm -rf $(BIN)
