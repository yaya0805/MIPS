TARGET=CMP
$(TARGET):guava.o 
	g++ guava.cpp -o $(TARGET)
guava.o:guava.cpp
	g++ -c guava.cpp
clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
