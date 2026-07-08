BIN = "bin"

main: main.c
	mkdir -p bin && \
	gcc \
	main.c \
	-L/opt/homebrew/lib \
	-I/opt/homebrew/include \
	-lSDL3 \
	-o $(BIN)/terminal
