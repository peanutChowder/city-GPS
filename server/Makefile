# Name: Jacob Feng
# SID: 1591656
# CCID: jfeng4
# CMPUT 275 Wi22
#
# Assignment 2: Navigation System

CC = g++
CFLAGS = -Wall 

server: server.o digraph.o dijkstra.o
	${CC} -o server server.o digraph.o dijkstra.o

server.o: digraph.o server.cpp 
	${CC} -c server.cpp

digraph.o: wdigraph.h digraph.h digraph.cpp
	${CC} -c digraph.cpp

dijkstra.o: dijkstra.h dijkstra.cpp
	${CC} -c dijkstra.cpp

clean: 
	rm *.o server