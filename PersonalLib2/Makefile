book_server:book_server.cc 
	g++ -o $@ $^ -std=c++11  -I include -L lib -lmysqlclient -lpthread -ldl -static -I ~/third_part/include -L ~/third_part/lib -lctemplate -I ~/install/include -L ~/install/linux-gcc-8 -ljson_linux-gcc-8_libmt
.PHONY:clean
clean:
	rm -rf book_server
