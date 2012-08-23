#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "command.h"
#include "builtin.h"

//DONE 組み込みコマンドには単独での実行しか許していない。そのことをここで上手く表現する必要がある。
//DONE parse_command_line()が&を解析できるようにしないといけない。
//DEBUG TYPEの設定に失敗している。

#define INIT_ARGV 8
#define IDENT_CHAR_P(c) (isspace((int)c) || ((c) == '|') || ((c) == '>') || ((c) == '<') || ((c) == '&'))

struct cmd*
parse_command_line(char *p)
{
    struct cmd *cmd;
	
    cmd = xmalloc(sizeof(struct cmd));
    cmd->argc = 0;
    cmd->argv = xmalloc(sizeof(char*) * INIT_ARGV);
    cmd->capa = INIT_ARGV;
    cmd->next = NULL;
    while (*p) {
        while (*p && isspace((int)*p))
            *p++ = '\0';
        if (IDENT_CHAR_P(*p))
            break;
        if (*p && ! IDENT_CHAR_P(*p)) {
            if (cmd->capa <= cmd->argc) {
                cmd->capa *= 2;
                cmd->argv = xrealloc(cmd->argv, cmd->capa);
            }
            cmd->argv[cmd->argc] = p;
            cmd->argc++;
        }
        while (*p && ! IDENT_CHAR_P(*p))
            p++;
    }
    if (cmd->capa <= cmd->argc) {
        cmd->capa += 1;
        cmd->argv = xrealloc(cmd->argv, cmd->capa);
    }
    cmd->argv[cmd->argc] = NULL;
    
    if (cmd->argc > 0) {
    	//まず組み込みコマンドかどうかチェックする。
    	if (lookup_builtin(cmd->argv[0]) != NULL) cmd->type = TYPE_BUILTIN;
    	else cmd->type = TYPE_NORMAL; //暫定的にこうしとく。(cmdheadのため)
    	
		if (*p == '|' || *p == '>' || *p == '<' || *p == '&') { //この中で&をいかに処理するか?
			if (cmd == NULL || cmd->argc == 0) goto parse_error;
			cmd->next = parse_command_line(p + 1);
			if (*p == '&') { //&の場合はcmd->next=NULLでないとエラー
				if (cmd->next != NULL && cmd->next->argc != 0) goto parse_error;
				//&がついていることをこう表現する
				cmd->place = PLACE_BG;
			} else { //'|'または'>','<'の場合
				if (cmd->next == NULL || cmd->next->argc == 0) goto parse_error;
				//組み込みコマンドはパイプ、リダイレクトともに禁止。二重チェックしてる。
				if (cmd->type == TYPE_BUILTIN) goto parse_error;
				if (cmd->next->type == TYPE_BUILTIN) goto parse_error;
				//もし&がついていたなら先頭までそれを知らせる。
				if (cmd->next->place == PLACE_BG) cmd->place = PLACE_BG;
			}		
			if (*p == '>') { //特に'>'の場合
				if (cmd->next->argc != 1) goto parse_error;
				cmd->next->type = TYPE_REDIRECTOUT;
			}
			if (*p == '<') { //特に'<'の場合
				if (cmd->next->argc != 1) goto parse_error;
				cmd->next->type = TYPE_REDIRECTIN;
			}
			if (*p == '|') { //特に'|'の場合
				cmd->next->type = TYPE_NORMAL;
			}
			*p = '\0';
		}
    }
	
    return cmd;
	
parse_error:
    if (cmd) free_cmd(cmd);
    return NULL;
}

void
free_cmd(struct cmd *cmd)
{
    if (cmd->next != NULL)
        free_cmd(cmd->next);
    free(cmd->argv);
    free(cmd);
}

void*
xmalloc(size_t sz)
{
    void *p;
	
    p = calloc(1, sz);
    if (!p)
        exit(3);
    return p;
}

void*
xrealloc(void *ptr, size_t sz)
{
    void *p;
	
    if (!ptr) return xmalloc(sz);
    p = realloc(ptr, sz);
    if (!p)
        exit(3);
    return p;
}
