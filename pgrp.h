/*
 テスト完了(2007/11/23)
 */
struct pgrp { //リスト型の構造でプロセスグループを管理する
	int pgrp;
	char *cmd;
	int state;
	struct pgrp *next;
};

extern struct pgrp *head;

#define STATE_RUNNING 0
#define STATE_SUSPENDED 1
#define STATE_ITTOU 2
#define STATE_ITTIN 3

struct pgrp *create_pgrp(int pgrp, char *cmd);
struct pgrp *pgrp_tail(struct pgrp* head);
void add_pgrp(struct pgrp* head, struct pgrp* ppgrp);
int remove_pgrp(struct pgrp* head, int pgrp);
struct pgrp *search_pgrp(struct pgrp* head, int pgrp);
int switch_state_pgrp(struct pgrp* head, int pgrp, int state);
void show_pgrp(struct pgrp* head);
