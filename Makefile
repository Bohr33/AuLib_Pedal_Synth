# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall	-I/usr/local/include/AuLib/

LFLAGS = -L/usr/local/lib -lAuLib

# Target executable
TARGET = Pedal_Synth

# Source files
SRCS = Pedal_Synth.cpp

# Rule to build the executable
$(TARGET):	
	$(CXX)	$(SRCS)	$(CXXFLAGS)	-o	$(TARGET)	$(LFLAGS)	

# Clean up
clean:	
	rm	-f	$(TARGET)	

rebuild:
	rm -f $(TARGET)
	make
