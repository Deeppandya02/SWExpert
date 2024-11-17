all: compile link

compile:
	g++ -c sfml_test.cpp -ID:\SWExpert\SFML-2.5.1\include -DSFML_STATIC

link:
	g++ sfml_test.o -o sfml-app -LD:\SWExpert\SFML-2.5.1\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

clean:
	rm -f main *.o