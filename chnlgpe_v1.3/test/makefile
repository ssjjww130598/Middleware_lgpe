cjsondemo : jsonparse.o cJSON.o
	gcc -o cjson_file_demo jsonparse.o cJSON.o -lm
jsonparse.o : cJSON.h
cJSON.o : cJSON.h
.PHONY : clean
clean :
	rm cjsondemo jsonparse.o cJSON.o
