#include <stdlib.h>
#include <stdio.h>

char *read_file(char *path, size_t *ret_size) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, sizeof(char), size, file);
    if (read < size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    if (ret_size != NULL) {
        *ret_size = size;
    }

    buffer[size] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return -1;
    }

    size_t size = 0;
    char *contents = read_file(argv[1], &size);

    if (!contents) {
        fprintf(stderr, "Error: failed to read %s\n", argv[1]);
        free(contents);
        return -1;
    }

    printf("%s:\n%s", argv[1], contents);

    free(contents);
    return 0;
}