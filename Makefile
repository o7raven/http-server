all:
		gcc main.c -lws2_32 -DDEBUG -g3 -Wall -Wextra -Wpedantic -Werror
 