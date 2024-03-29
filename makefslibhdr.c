/**
 * Helper program to generate fslib_lua.h
 */

#include <stdio.h>

int main(char *argv[])
{
    FILE *f, *inf;
    char line[512];
    memset(line, 0, sizeof(line));

    f = fopen("fslib_lua.h","wt");
    fprintf(f, "#ifndef _FSLIB_LUA_H\n");
    fprintf(f, "#define _FSLIB_LUA_H\n\n");
    fprintf(f, "/* THIS IS A PROGRAMMATICALLY GENERATED FILE. DO NOT EDIT */\n\n");
    fprintf(f, "const char *fslib_lua = \"\\\n");

    inf = fopen("fs.lua", "rt");
    while (fgets(line, sizeof(line)-1, inf)) {
        if (line[strlen(line)-1]=='\n') {
            line[strlen(line)-1]='\0';
        }
        if (line[strlen(line)-1]=='\r') {
            line[strlen(line)-1]='\0';
        }
        fprintf(f, "%s\\r\\n\\\n", line);
        memset(line, 0, sizeof(line));
    }
    fclose(inf);

    fprintf(f, "\";\n\n");
    fprintf(f, "#endif\n");
    fclose(f);
    return 0;
}
