/*
 *  contextswitch.h
 *  
 *  holds the functions KCCopy and KCSwitch
*/


#include <hardware.h>


KernelContext *KCCopy(KernelContext *kc_in, void *new_pcb_p, void *not_used);

KernelContext *KCSwitch(KernelContext *kc_in,void *curr_pcb_p, void *next_pcb_p);


