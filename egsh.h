#include "command.h"
#include "builtin.h"
#include "pgrp.h"
#include "signal.h"

////////////////////////////////////
//コマンド実行の流れを記述するための関数
////////////////////////////////////
void prompt(void);
int invoke_commands(struct cmd *cmdhead);
void exec_pipeline(struct cmd *cmdhead);
void redirect_stdout(char *path);
void redirect_stdin(char *path);
int wait_pipeline(struct cmd *cmdhead);
struct cmd* pipeline_tail(struct cmd *cmdhead);

