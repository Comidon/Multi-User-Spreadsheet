CC = g++
CFLAGS = -pthread
TARGET = server
$(TARGET): main.o server.o serverside_sheet.o
	$(CC) *.o -o $(TARGET) $(CFLAGS)
serverside_sheet.o: serverside_sheet.cpp serverside_sheet.h
	$(CC) -c serverside_sheet.cpp
server.o: server.cpp server.h serverside_sheet.h
	$(CC) -c server.cpp
main.o: main.cpp server.h
	$(CC) -c main.cpp
clean:
	rm -f server *.o