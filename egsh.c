#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include "egsh.h"


struct pgrp *head;

int
main (int argc, char *argv[])
{
	/*
	int result;
	result = setsid();
	printf("%d" ,result);
	*/
	
	head = malloc(sizeof(struct pgrp));
	head->pgrp = getpgrp();
	head->cmd = argv[0];
	head->state = STATE_RUNNING;
	head->next = NULL;
	
	signal_for_main();
	for (;;) {
		prompt();
	}
    exit(EXIT_SUCCESS);
}

#define BUF_MAX 2048

void
prompt(void)
{
	char buf[BUF_MAX];
	char *cmdline;
	struct cmd *cmd;
	int pid;
	struct pgrp *ppgrp;
	int st;
	
	fprintf(stdout, "$ ");
   	fflush(stdout);
	if (fgets(buf, BUF_MAX, stdin) == NULL) return;
	cmdline = malloc(strlen(buf)+1);
	strcpy(cmdline, buf);
	cmd = parse_command_line(cmdline);
	if (cmd == NULL) {
		fprintf(stderr, "%s: syntax error\n", head->cmd);
		return;
	}
	if (cmd->argc == 0) {
		free_cmd(cmd);
		return;
	}
	if (cmd->type == TYPE_BUILTIN) {//組み込みコマンドはここですぐに実行
		cmd->status = lookup_builtin(cmd->argv[0])->f(cmd->argc, cmd->argv);
		free_cmd(cmd); //cmdをfree()する
		free(cmdline); //cmdlineをfree()する
		if (tcgetpgrp(1) != head->pgrp) { //fgのとき制御端末にもどれるまでまつ。
			pause();
		}
	}
	else { //組み込みコマンドでなかったら
		pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(3);
		}
		if (pid > 0) { //親プロセス
			setpgid(pid, pid);
			ppgrp = create_pgrp(pid, cmdline);
			add_pgrp(head, ppgrp);
			if (cmd->place == PLACE_BG) { //&の有無によって制御端末を変更する。こちらはついている場合。
				;
			}
			else { //ついていない場合
				tcsetpgrp(1, ppgrp->pgrp);
				while(tcgetpgrp(1) == ppgrp->pgrp) { //このプロセスが止まったり終わるまでまつ。
					pause();
				}
			}
		}
		else { //子プロセス
			//シグナルの設定をここですぐにする。
			signal_for_pipeline();
			st = invoke_commands(cmd); //ノーマルなコマンドを実行する。
			free_cmd(cmd); //cmdをfree()する
			free(cmdline); //cmdlineをfree()する
			if (tcgetpgrp(1) != getpgrp()) printf("\n%d\tDONE\n", getpgrp());//exit()していたらとにかくDONE。
			if (tcgetpgrp(1) == getpgrp()) tcsetpgrp(1, head->pgrp); //終わる前に制御端末をもどす。
			exit(EXIT_SUCCESS);
		}
	}
	return;
}

int
invoke_commands(struct cmd *cmdhead)
{
    int st;
    int original_stdin = dup(0);
    int original_stdout = dup(1);
	
    exec_pipeline(cmdhead);
    st = wait_pipeline(cmdhead);
    close(0); dup2(original_stdin, 0); close(original_stdin);
    close(1); dup2(original_stdout, 1); close(original_stdout);
	
    return st;
}

#define HEAD_P(cmd) ((cmd) == cmdhead)
#define REDIRECT_P(cmd) ((cmd->type == TYPE_REDIRECTOUT) || (cmd->type == TYPE_REDIRECTIN))
#define TAIL_P(cmd) (((cmd)->next == NULL) || REDIRECT_P(cmd->next))

void
exec_pipeline(struct cmd *cmdhead)
{
    struct cmd *cmd;
    int fds1[2] = {-1, -1};
    int fds2[2] = {-1, -1};
	
	//このループはパイプの処理のために行われている。
	//cmd1 | cmd2 | cmd 3 > file ならcmd1とcmd2とcmd3について実行される。
	//cmd < file ならcmdについてのみ実行される。
    for (cmd = cmdhead; cmd && !(REDIRECT_P(cmd)); cmd = cmd->next) {
        fds1[0] = fds2[0]; //標準入力をパイプからもらうための準備。
        fds1[1] = fds2[1]; //cmd1 | cmd2 | cmd 3 > file ならcmd2とcmd3のみに必要。
        if (! TAIL_P(cmd)) {
            if (pipe(fds2) < 0) {
                perror("pipe");
                exit(3);
            }
        }
        cmd->pid = fork();
        if (cmd->pid < 0) {
            perror("fork");
            exit(3);
        }
        if (cmd->pid > 0) { /* parent */
            if (fds1[0] != -1) close(fds1[0]);
            if (fds1[1] != -1) close(fds1[1]);
            continue;
        }
        //標準入力をパイプからもらうための作業。
        //cmd1 | cmd2 | cmd 3 > file ならcmd2とcmd3のみに必要。
        if (! HEAD_P(cmd)) {
            close(0); dup2(fds1[0], 0); close(fds1[0]);
            close(fds1[1]);
        }
        //標準入力をリダイレクトからもらうための作業。
        //cmd1 | cmd2 | cmd 3 > file なら実行されることはない。
        //cmd < file ならcmdのみに必要。このcmdについて実行されることがあるのはここだけ。
        if ((cmd->next != NULL) && (cmd->next->type == TYPE_REDIRECTIN)) {
        	redirect_stdin(cmd->next->argv[0]);
        }
        //標準出力をパイプに渡すための作業。
        //cmd1 | cmd2 | cmd 3 > file ならcmd1とcmd2のみに必要。
        if (! TAIL_P(cmd)) {
            close(fds2[0]);
            close(1); dup2(fds2[1], 1); close(fds2[1]);
        }
        //標準出力をリダイレクトするファイルに移すための作業。
        //cmd1 | cmd2 | cmd 3 > file ならcmd3のみに必要。
        if ((cmd->next != NULL) && (cmd->next->type == TYPE_REDIRECTOUT)) {
        	redirect_stdout(cmd->next->argv[0]);
        }
        execvp(cmd->argv[0], cmd->argv);
        fprintf(stderr, "%s: command not found: %s\n", head->cmd, cmd->argv[0]);
        exit(1);
    }
}

void
redirect_stdin(char *path)
{
    int fd;
	
    close(0);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        return;
    }
    if (fd != 0) {
        dup2(fd, 0);
        close(fd);
    }
}

void
redirect_stdout(char *path)
{
    int fd;
	
    close(1);
    fd = open(path, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    if (fd < 0) {
        perror(path);
        return;
    }
    if (fd != 1) {
        dup2(fd, 1);
        close(fd);
    }
}

int
wait_pipeline(struct cmd *cmdhead)
{
    struct cmd *cmd;
	
    for (cmd = cmdhead; cmd && !(REDIRECT_P(cmd)); cmd = cmd->next) {
        waitpid(cmd->pid, &cmd->status, 0);
    }
    return pipeline_tail(cmdhead)->status;
}

struct cmd*
pipeline_tail(struct cmd *cmdhead)
{
    struct cmd *cmd;
	
    for (cmd = cmdhead; !TAIL_P(cmd); cmd = cmd->next)
        ;
    return cmd;
}
