#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "utils.h"
#include "fileHandler.h"

int isValidPath(char *path) {
    struct stat* statBuffer = malloc(sizeof(struct stat));
    memset(statBuffer, 0, sizeof(struct stat));
    int status = lstat(path, statBuffer);
    __mode_t mode = statBuffer->st_mode;
    
    free(statBuffer);

    return (status == 0 && (S_ISDIR(mode) || S_ISLNK(mode) || S_ISREG(mode))) ? 1 : 0;
}

sll getLength(char *path) {
    if (isValidPath(path) == 1) {
        sll length;
        FILE *f = fopen(path, "rb");

        if (f) {
            fseek(f, 0, SEEK_END);
            length = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            return length;
        } else
            return -1;
    } else 
        return -1;
}

/** Gets all of the subfiles/folders of { @param path }
 * @param path          The path of the node
 * @param files         An output array of filenames representing the subfiles/folders at the location described by { @param path }
 * @param numberOfFiles An integer (*ptr) representing the number of files found.
 */
void getFiles(const char *path, char *files[FILENAME_MAX + 1], int *numberOfFiles) {
    int count = 0;  // Count the children (sounds worse than it is).
    struct dirent *currentDir;
    struct stat *statBuffer = malloc(sizeof(struct stat));

    if (lstat(path, statBuffer) == 0 && S_ISDIR(statBuffer->st_mode)) { // If it's a dir, then it might have children, otherwise... nope.
        DIR *folder = opendir(path); 
        if (access(path, F_OK) != -1) { // If the program can access the object at the end of the path.
            if (folder) // Check innulability (yes I just invented a word)
                while ((currentDir = readdir(folder))) {    // If children can be read.
                    if (strcmp(currentDir->d_name, ".") && strcmp(currentDir->d_name, "..")) { // And those children don't loop back to self or parent.s
                        files[count] = (char*) malloc(FILENAME_MAX + 1); // Add it to the children's addresses (getting weird looks from the people at the FBI)
                        snprintf(files[count], FILENAME_MAX + 1, "%s", currentDir->d_name);  // Is a basename.
                        count++;
                    }
                }
            free(currentDir);
        }
        closedir(folder);
    } else if (errno < 0)
        printf("ERROR %d : %s\n", errno, strerror(errno));

    free(statBuffer);

    *numberOfFiles = count; // Keep track of the number of children.
}