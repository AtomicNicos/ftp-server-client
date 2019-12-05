char* list(int *_argc, char **_argv) {
    return "";
}

char* upload(int *_argc, char **_argv) {
    return "";
}

char* download(int *_argc, char **_argv) {
    return "";
}

char* changeDirectory(int *_argc, char **_argv) {
    return "";
}

char* getHelp(int *_argc, char **_argv) {
    return "";
}

/** Define the functions */
char* (*builtin[]) (int *, char **) = {
    &list,
    &upload,
    &download,
    &changeDirectory,
    &getHelp
};

/** Define the builtins */
char *builtins[] = {
    "list",
    "ul",
    "dl",
    "cd",
    "help"
};