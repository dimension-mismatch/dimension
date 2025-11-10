another : another.o
	ld -arch x86_64 another.o -o another -platform_version macos 14.5 12.0

another.o : another.s
	nasm -f macho64 another.s