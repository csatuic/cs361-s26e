CC = gcc
CFLAGS = -O3 -g -Wall -Wextra -fno-omit-frame-pointer -I.

OBJECTS = main.o log_reader.o parser.o processor.o reporter.o utils.o log_processor.o statistics_generator.o
KEEP_OBJS = log_processor.o statistics_generator.o

TARGET = log_analyzer

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(filter-out $(KEEP_OBJS), $(wildcard *.o)) $(TARGET) perf.data*

# To build your hidden objects (run once by you):
# gcc $(CFLAGS) -c log_processor.c -o log_processor.o
# gcc $(CFLAGS) -c statistics_generator.c -o statistics_generator.o
