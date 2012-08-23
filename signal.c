#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "signal.h"
#include "pgrp.h"


//メインプロセスはSIGINT,SIGQUIT,SIGTERM,SIGSTPを無視する。
//SIGITTIN,SIGITTOUの処理もしたい。
//SIGCHLDを受けたらwait()する必要もある。

void
signal_for_main(void)
{
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	
	struct sigaction act_child;
	act_child.sa_handler = main_child_handler;
	act_child.sa_flags = 0;
	sigemptyset(&act_child.sa_mask);
	sigaction(SIGCHLD, &act_child, NULL);
}

//子プロセスをwait()してtcsetpgrp()して制御を戻すハンドラ
void
main_child_handler(int sig)
{
	int pgrp;
	struct pgrp *ppgrp;
	int ret_child;
	
	while((pgrp = waitpid(-1, &ret_child, WNOHANG | WUNTRACED)) != 0) {
		if (pgrp == -1) {
			if (errno == ECHILD) break;
			if (errno == EINTR) continue;
			perror("waitpid");
			exit(EXIT_FAILURE);
		} else if (pgrp >0) { //TO DO ret_childで子の状態を調べて端末に表示する。ppgrp->stateの値も変更する。
			if((ppgrp = search_pgrp(head, pgrp)) == NULL) continue;
			if(WIFEXITED(ret_child)) {
				remove_pgrp(head, pgrp);
				//if (tcgetpgrp(1) != getpgrp()) printf("%d\tDONE\n", pgrp); //この作業は子プロセスの終了前にやる。
			}
			else if (WIFSIGNALED(ret_child)) {
				remove_pgrp(head, pgrp);
				printf("\n%d\tDEAD[%d]\n", pgrp, WTERMSIG(ret_child));
			}
			if (WIFSTOPPED(ret_child)) {
				switch_state_pgrp(head, pgrp, STATE_SUSPENDED);
				printf("\n%d\tSUSPENDED[%d]\n", pgrp, WSTOPSIG(ret_child));
			}
		}
	}
	return;
}

//子プロセスグループは止まったり終了したときにtcsetpgrp(1, head->pgrp)して制御端末をmainにもどす必要がある。
void
signal_for_pipeline(void)
{
	struct sigaction act_default;
	act_default.sa_handler = SIG_DFL;
	act_default.sa_flags = 0;
	sigemptyset(&act_default.sa_mask);
	sigaction(SIGCHLD, &act_default, NULL);
	//sigaction(SIGTSTP, &act_default, NULL);
	
	struct sigaction act_term;
	act_term.sa_handler = sub_term_handler;
	act_term.sa_flags = SA_NODEFER;
	/*
	sigfillset(&act_term.sa_mask);
	sigdelset(&act_term.sa_mask, SIGINT);
	sigdelset(&act_term.sa_mask, SIGQUIT);
	sigdelset(&act_term.sa_mask, SIGTERM);
	*/
	sigemptyset(&act_term.sa_mask);
	sigaction(SIGINT, &act_term, NULL);
	sigaction(SIGQUIT, &act_term, NULL);
	sigaction(SIGTERM, &act_term, NULL);
	
	
	struct sigaction act_stop;
	act_stop.sa_handler = sub_stop_handler;
	act_stop.sa_flags = SA_NODEFER;
	/*
	sigfillset(&act_stop.sa_mask);
	sigdelset(&act_stop.sa_mask, SIGTSTP);
	*/
	sigemptyset(&act_stop.sa_mask);
	sigaction(SIGTSTP, &act_stop, NULL);
	
}

//tcsetpgrp(1, head->pgrp)してexit()する。
void
sub_term_handler(int sig)
{
	int pgrp;
	struct sigaction act_default;
	struct sigaction act_term;
	
	pgrp = getpgrp();
	
	tcsetpgrp(1, head->pgrp);
	
	act_default.sa_handler = SIG_DFL;
	act_default.sa_flags = 0;
	sigemptyset(&act_default.sa_mask);
	sigaction(sig, &act_default, &act_term);
	
	killpg(pgrp, sig);
	
	sigaction(sig, &act_term, NULL);
}

void
sub_stop_handler(int sig)
{
	int pgrp;
	struct sigaction act_default;
	struct sigaction act_stop;
	
	pgrp = getpgrp();
	
	tcsetpgrp(1, head->pgrp);
	
	act_default.sa_handler = SIG_DFL;
	act_default.sa_flags = 0;
	sigemptyset(&act_default.sa_mask);
	sigaction(sig, &act_default, &act_stop);
	
	killpg(pgrp, sig);
	
	sigaction(sig, &act_stop, NULL);
}

