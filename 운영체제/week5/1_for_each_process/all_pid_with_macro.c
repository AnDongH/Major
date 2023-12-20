#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

int module_start(void) {
	struct task_struct *task;

	printk(KERN_INFO "Init Module....");
	
	// Init Process 부터 실행된 모든 프로세스 정보 출력
	for_each_process(task) {
		printk("%s[%d] : %ld\n", task->comm, task->pid, task->state);
	}
	return 0;
}

void module_end(void) {
	printk("Module removing...");
}

module_init(module_start);
module_exit(module_end);
