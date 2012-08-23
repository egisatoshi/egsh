struct builtin {
    char *name;
    int (*f)(int argc, char *argv[]);
};


///////////////////////////////////////
//組み込みコマンドを扱うための関数
////////////////////////////////////////
struct builtin* lookup_builtin(char *name);
int builtin_cd(int argc, char *arg[]);
int builtin_exit(int argc, char *arg[]);
int builtin_fg(int argc, char *arg[]);
int builtin_bg(int argc, char *arg[]);
int builtin_jobs(int argc, char *arg[]);

