#include <stdio.h>
#include <list>

using namespace std;

struct change_t {
    uint64_t offset;
    uint8_t old_val;
    uint8_t new_val;
};

struct patch_t {
    char *filename;
    list<struct change_t> *changes;
};

list<struct patch_t> _patches;

void read_patch(struct patch_t *p) {
    FILE *f = fopen(p->filename, "rt");
    if (!f) {
        printf("WARN: unable to open %s for reading.\n", p->filename);
        return;
    }
    // the format is that produced by "cmp -l" unix tool:
    // each line has: <decimal-offset> <octal-old> <octal-new>
    p->changes = new list<struct change_t>();
    while (!feof(f)) {
        struct change_t c;
        if (fscanf(f, "%llu %o %o", &c.offset, &c.old_val, &c.new_val) != 3) {
            // skip invalid lines
            continue;
        }
        c.offset--;
        p->changes->push_back(c);
    }
    fclose(f);
    printf("INFO: read patch from %s. Changes: %u\n", p->filename, p->changes->size());
}

void read_patches(char *ininame, list<patch_t> *li) {
    FILE *f = fopen(ininame, "rt");
    if (!f) {
        printf("WARN: unable to open %s for reading.\n", ininame);
    }
    
    char line[512];
    char filename[512];
    while (!feof(f)) {
        memset(line, 0, sizeof(line));
        if (fgets(line, sizeof(line), f) == NULL) {
            continue;
        }
        memset(filename, 0, sizeof(filename));
        if (sscanf(line, "%s", filename) != 1) {
            continue;
        }

        struct patch_t p;
        memset(&p, 0, sizeof(p));
        p.filename = strdup(filename);
        read_patch(&p);
        if (p.changes) {
            li->push_back(p);
        }
    }
    fclose(f);
}

bool apply_patch(char *filename, list<struct change_t> *li) {
    FILE *f = fopen(filename, "r+b");
    if (!f) {
        printf("ERROR: unable to open %s for reading\n", filename);
        return false;
    }
    
    // step one: verify full compatibility
    bool already_patched(true);
    uint8_t b;
    list<struct change_t>::iterator it;
    for (it = li->begin(); it != li->end(); it++) {
        fseek(f, it->offset, SEEK_SET);
        if (fread(&b, 1, 1, f) != 1) {
            printf("ERROR: unable to read byte at offset %llu\n", it->offset);
            fclose(f);
            return false;
        }
        if (b == it->new_val) {
            continue;
        }
        if (b != it->old_val) {
            printf("WARN: exefile %s is not compatible with the patch. Offset: %llu. Expecting: %02x, Got: %02x\n",
                filename, it->offset, it->old_val, b);
            fclose(f);
            return false;
        }
        already_patched = false;
    }

    // step two: apply patch
    if (already_patched) {
        printf("INFO: file %s is already patched. Do you want to unpatch? (y/N): ", filename);
        char answer[64];
        memset(answer, 0, sizeof(answer));
        fgets(answer, sizeof(answer), stdin);
        if (answer[0] == 'Y' || answer[0] == 'y') {
            for (it = li->begin(); it != li->end(); it++) {
                fseek(f, it->offset, SEEK_SET);
                if (fwrite(&it->old_val, 1, 1, f) != 1) {
                    printf("ERROR: unable to write byte at offset %llu\n", it->offset);
                    fclose(f);
                    return false;
                }
            }
            printf("INFO: file %s was successfully unpatched. %u bytes changed\n", filename, li->size());
            fclose(f);
        }
        else {
            fclose(f);
            printf("INFO: No changes made.\n");
        }
        return true;
    }
    for (it = li->begin(); it != li->end(); it++) {
        fseek(f, it->offset, SEEK_SET); 
        if (fwrite(&it->new_val, 1, 1, f) != 1) {
            printf("ERROR: unable to write byte at offset %llu\n", it->offset);
            fclose(f);
            return false;
        }
    }
    printf("INFO: file %s was successfully patched. %u bytes changed\n", filename, li->size());
    fclose(f);
    return true;
}

int main(int argc, char *argv[]) {
    char *exepath = "..\\..\\PES2021.exe";
    if (argc > 1) {
        exepath = strdup(argv[1]);
        if (strlen(exepath) >= 4 && strcmp(exepath + strlen(exepath)-4, "help") == 0) {
            printf("Usage: %s [exefile]\n", argv[0]);
            return 0;
        }
    }
    printf("INFO: exepath: %s\n", exepath);

    read_patches("patcher.ini", &_patches);
    printf("INFO: available patches to try: %d\n", _patches.size());

    list<struct patch_t>::iterator it;
    for (it = _patches.begin(); it != _patches.end(); it++) {
        printf("INFO: trying patch: %s\n", it->filename);
        if (apply_patch(exepath, it->changes)) {
            break;
        }
    }

    printf("\nPress [Enter] to finish\n");
    getchar();
    return 0;
}
