#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

int main(int argc, char *argv[]); 

static void print_tree(const char *path, int depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[PATH_MAX];

    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "tree: no se pudo abrir '%s': %s\n", path, strerror(errno));
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construir ruta completa
        if (snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name) >= (int)sizeof(fullpath)) {
            fprintf(stderr, "tree: ruta demasiado larga: %s/%s\n", path, entry->d_name);
            continue;
        }

        // lstat para NO seguir enlaces simbólicos
        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "tree: no se pudo obtener info de '%s': %s\n", fullpath, strerror(errno));
            continue;
        }

        // Indentación por profundidad
        for (int i = 0; i < depth; i++) {
            printf("    ");
        }

        // Mostrar nombre
        if (S_ISDIR(st.st_mode)) {
            printf("carpeta %s/\n", entry->d_name);
            // Recursión solo en directorios reales (no symlink)
            print_tree(fullpath, depth + 1);
        } else if (S_ISLNK(st.st_mode)) {
            printf("enlace %s\n", entry->d_name);
            // No seguir symlinks recursivamente
        } else {
            printf("archivo %s\n", entry->d_name);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *root = ".";

    // Uso: tree <ruta> (opcional)
    if (argc > 2) {
        fprintf(stderr, "Uso: tree [ruta]\n");
        return 1;
    }
    if (argc == 2) {
        root = argv[1];
    }

    struct stat st;
    if (lstat(root, &st) == -1) {
        fprintf(stderr, "tree: '%s': %s\n", root, strerror(errno));
        return 1;
    }

    // Mostrar raíz
    printf("%s\n", root);

    if (S_ISDIR(st.st_mode)) {
        print_tree(root, 1);
    } else {
        // Si es archivo, solo se muestra ese elemento
        // (comportamiento razonable si pasan una ruta no-directorio)
    }

    return 0;
}
