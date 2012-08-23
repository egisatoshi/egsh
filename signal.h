//////////////////////////////////////
//シグナルをその場その場で適切に処理するための関数
/////////////////////////////////////
//メインプロセスはSIGINT,SIGQUIT,SIGTERMを無視する。SIGCHLDを受けたらwait()する必要もある
void signal_for_main(void);
void main_child_handler(int sig);
//子プロセスグループは止まったり終了したときにtcsetpgrp(1, head->pgrp)して制御端末をmainにもどす必要がある。
void signal_for_pipeline(void);
void sub_term_handler(int sig);
void sub_stop_handler(int sig);
