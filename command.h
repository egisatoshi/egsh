struct cmd {
    int argc;
    char **argv;
    int capa;
	int type;
    int status; //組み込みコマンドには関係ない
    int pid; //組み込みコマンドには関係ない
    int place; //&がセットされていたらcmdの最後尾から先頭まですべてのcmd->placeにPLACE_BGが代入される。
    struct cmd *next;
};

#define TYPE_BUILTIN 1
#define TYPE_NORMAL 2
#define TYPE_REDIRECTOUT 3 // command > file の場合、fileがこれになる
#define TYPE_REDIRECTIN 4 // command < file の場合、fileがこれになる。
// command1 | command2 < file などは許さない。

#define PLACE_BG 1

struct cmd *parse_command_line(char *cmdline);
void *xmalloc(size_t sz);
void *xrealloc(void *ptr, size_t sz);
void free_cmd(struct cmd *p);


