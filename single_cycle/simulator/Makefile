TARGET=single_cycle
$(TARGET):project1.o 
	g++ project1.cpp -o $(TARGET)
project1.o:project1.cpp
	g++ -c project1.cpp
clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
