BINARY = webserver
SOURCE_DIR = src
OBJECT_DIR = bin

CC = g++
CFLAGS = -Iinclude/ -std=c++20 -fPIC -g -lpthread -lssl -ljsoncpp

SOURCE_FILES = $(shell find $(SOURCE_DIR)/ -name *.cpp)
OBJECT_FILES = $(patsubst $(SOURCE_DIR)/%.cpp, $(OBJECT_DIR)/%.o, $(SOURCE_FILES))

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

all: $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o $(OBJECT_DIR)/$(BINARY) $(OBJECT_FILES)

lib: $(OBJECT_FILES)
	g++ -shared -o bin/library.o $(filter-out obj/main.o, $(OBJECT_FILES))

clean:
	rm bin/*