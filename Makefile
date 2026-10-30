BIN = "bin"

debug: main.c
	mkdir -p bin && \
	gcc \
	-DDEBUG \
	-Wall \
	-Wextra \
	main.c \
	-L/opt/homebrew/lib \
	-I/opt/homebrew/include \
	-lSDL3 \
	-o $(BIN)/frame-terminal

main: main.c
	mkdir -p bin && \
	gcc \
	-Wall \
	-Wextra \
	main.c \
	-L/opt/homebrew/lib \
	-I/opt/homebrew/include \
	-lSDL3 \
	-o $(BIN)/frame-terminal
