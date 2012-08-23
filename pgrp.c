/*
   テスト完了(2007/11/23)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pgrp.h"


struct pgrp *create_pgrp(int pgrp, char *cmd) {
	struct pgrp *ppgrp;
	char *pcmd;
	pcmd = malloc(strlen(cmd)+1);
	if (pcmd == NULL) exit(EXIT_FAILURE);
	strcpy(pcmd, cmd);
	ppgrp = malloc(sizeof(struct pgrp));
	if (ppgrp == NULL) exit(EXIT_FAILURE);
	ppgrp->pgrp = pgrp;
	ppgrp->cmd = pcmd;
	ppgrp->state = STATE_RUNNING;
	ppgrp->next = NULL;
	return ppgrp;
}

struct pgrp *pgrp_tail(struct pgrp* head) {
	struct pgrp *ppgrp;
	for (ppgrp=head; ppgrp->next!=NULL; ppgrp=ppgrp->next)
		;
	return ppgrp;
}

//create_pgrp()はこの関数を呼び出す前に、fork()したあとの親プロセスでしなければならない。
void add_pgrp(struct pgrp* head, struct pgrp* ppgrp) {
	struct pgrp *qpgrp;
	qpgrp = pgrp_tail(head);
	qpgrp->next = ppgrp;
	return;
}

//もしプロセスグループが見つからなかったら1を返す。成功したら0を返す。
int remove_pgrp(struct pgrp* head, int pgrp) {
	struct pgrp *ppgrp;
	struct pgrp *temp;
	ppgrp = head;
	if (ppgrp->next == NULL)
		return 1;
	while (ppgrp->next->pgrp != pgrp && ppgrp->next->next != NULL) {
		ppgrp = ppgrp->next;
	}
	if (ppgrp->next->pgrp != pgrp)
		return 1;
	temp = ppgrp->next->next;
	free(ppgrp->next);
	ppgrp->next = temp;
	return 0;
}

//fg, bgコマンドで使う
struct pgrp *search_pgrp(struct pgrp* head, int pgrp) {
	struct pgrp *ppgrp;
	ppgrp = head;
	while (ppgrp->pgrp != pgrp && ppgrp->next !=NULL) {
		ppgrp = ppgrp->next;
	}
	if (ppgrp->pgrp == pgrp)
		return ppgrp;
	else //見つからなかった場合はNULLを返す
		return NULL;
}

//もしプロセスグループが見つからなかったら1を返す。成功したら0を返す。
int switch_state_pgrp(struct pgrp* head, int pgrp, int state) {
	struct pgrp *ppgrp;
	if ((ppgrp = search_pgrp(head, pgrp)) == NULL)
		return 1;
	ppgrp->state = state;
	return 0;
}

void show_pgrp(struct pgrp* head) {
	struct pgrp *ppgrp;
	char state[16];
	if (head->next == NULL) return;
	ppgrp = head->next;
	printf("PGRP\tSTATE\t\tCMD\n");
	while (ppgrp != NULL) {
		switch (ppgrp->state) {
			case 0:
				strcpy(state, "running  ");
				break;
			case 1:
				strcpy(state, "suspended");
				break;
			case 2:
				strcpy(state, "stopped  ");
				break;
			default:
				strcpy(state, "unknown  ");
				break;
		}
		printf("%d\t%s\t%s\n", ppgrp->pgrp, state, ppgrp->cmd);
		ppgrp = ppgrp->next;
	}
	return;
}



