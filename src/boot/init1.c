void putstr(const char *str, int row, int col, int color) {
    unsigned char *vram = (unsigned char *)0xb8000;
    int offset = row * 160 + 2 * col;
    for(int i = 0; str[i]; i++) {
        vram[offset++] = str[i];
        vram[offset++] = (unsigned char)color; 
    }
}

void init1() {
    char str[] = "Hello World!";
    putstr(str, 1, 0, 1);
}