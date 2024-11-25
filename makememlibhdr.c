/**
 * Helper program to generate memlib_lua.h
 */

#include <stdio.h>

int main(char *argv[])
{
    FILE *f, *inf;
    char line[512];
    int i;
    memset(line, 0, sizeof(line));

    f = fopen("memlib_lua.h","wt");
    fprintf(f, "#ifndef _MEMLIB_LUA_H\n");
    fprintf(f, "#define _MEMLIB_LUA_H\n\n");
    fprintf(f, "/* THIS IS A PROGRAMMATICALLY GENERATED FILE. DO NOT EDIT */\n\n");
    fprintf(f, "const char *memlib_lua = \"\\\n");

    inf = fopen("memory.lua", "rt");
    while (fgets(line, sizeof(line)-1, inf)) {
        if (line[strlen(line)-1]=='\n') {
            line[strlen(line)-1]='\0';
        }
        if (line[strlen(line)-1]=='\r') {
            line[strlen(line)-1]='\0';
        }
        for (i=0; i<strlen(line); i++) {
            switch (line[i]) {
                case '\\':
                    fprintf(f, "\\\\");
                    break;
                case '"':
                    fprintf(f, "\\\"");
                    break;
                default:
                    fprintf(f, "%c", line[i]);
            }
        }
        fprintf(f, "\\r\\n\\\n");
        memset(line, 0, sizeof(line));
    }
    fclose(inf);

    fprintf(f, "\";\n\n");
    fprintf(f, "#endif\n");
    fclose(f);
    return 0;
}
