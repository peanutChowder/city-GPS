# Name: Jacob Feng
# SID: 1591656
# CCID: jfeng4
# CMPUT 275 Wi22
#
# Assignment 2: Navigation System

CC = g++
CFLAGS = -Wall 

run:
	gnome-terminal -- ./server/server 
	python3 client/client.py

clean:
	cd server && make clean
	rm inpipe outpipe
	