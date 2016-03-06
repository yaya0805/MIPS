TARGET=pipeline
$(TARGET):papayaline.o 
	g++ papayaline.cpp -o $(TARGET)
papayaline.o:papayaline.cpp
	g++ -c papayaline.cpp
clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
