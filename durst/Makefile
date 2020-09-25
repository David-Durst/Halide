CXX=g++
CXXFLAGS=-g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg  -lpthread -ldl -std=c++11 # -I $(BARVINOK_PATH) -I $(OPT_PATH) -I $(CLOCKWORK_PATH)
LINKFLAGS=-L $(OPT_LIB_PATH) -lclkwrk -L $(ISL_PATH) -lclkwrk -lbarvinok -lisl -lntl -lgmp -lpolylibgmp -lpthread
ODIR=obj

all: clockwork aetherling
.PHONY: all clean

%.o: %.cpp %.h
	$(CXX) -c -o $@ $< $(CXXFLAGS)

ClockworkExporter.o : ClockworkExporter.cpp ClockworkExporter.h AffineIndexingExtractor.h VariableExtractor.h CleanVariableNames.h RDomExtractor.h
	$(CXX) -c -o $@ $< $(CXXFLAGS) 

AetherlingExporter.o : AetherlingExporter.cpp ClockworkExporter.h IndexingExtractor.h VariableExtractor.h
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clockwork: ClockworkExporter.o ClockworkExporterExamples.o AffineIndexingExtractor.o VariableExtractor.o CleanVariableNames.o # durst_memory.o
	$(CXX) -o $@ $^ $(CXXFLAGS) # $(LINKFLAGS)

aetherling: AetherlingExporter.o AetherlingExporterExamples.o VariableExtractor.o IndexingExtractor.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f *.o aetherling clockwork
