all:
	g++ -o gs gravity-snapshot.cpp -I.. -Wall -Wextra -Wfatal-errors -Werror=unknown-pragmas -Werror=unused-label -Wshadow -std=c++11 -pedantic -Dcimg_use_vt100 -Dcimg_display=1   -lm -lX11  -lpthread 
