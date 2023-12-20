#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

struct birthday {
  int day;
  int month;
  int year;
  struct list_head list;
};

static LIST_HEAD(birthday_list);

struct birthday *createBirthday(int day, int month, int year) {

  /* 1. TODO: 생일을 위한 메모리를 할당하고, 인자들을 채워 생일을 완성하세요. */
  struct birthday *n;
  n = kmalloc(sizeof(struct birthday), GFP_KERNEL);
  n->day = day;
  n->month = month;
  n->year = year;
  return n;  
}

int simple_init(void) {
  printk("INSTALL MODULE: bdlist\n");

  /* 2. TODO: 생일 목록을 하나씩 생성하는대로 연결리스트에 연결시키세요(노드 삽입). */
  struct birthday *node = createBirthday(12, 4, 1999);
  list_add_tail(&node->list, &birthday_list);
  node = createBirthday(6, 6, 2000);
  list_add_tail(&node->list, &birthday_list);
  node = createBirthday(7, 7, 2001);  
  /* 3. TODO: 완성된 연결리스트를 탐색하는 커널 함수를 사용하여 출력하세요. */
  struct birthday *cursor;
  list_for_each_entry(cursor, &birthday_list, list){
    printk("Day %d.%d.%d\n", cursor->day, cursor->month, cursor->year); 
  } 
  return 0;
}

/* 모듈을 제거할 때는 생성한 연결 리스트도 하나씩 제거하며 끝내도록 하세요. */
void simple_exit(void) {
  /* 제거를 하기 전에 리스트가 "비어있을 경우""에 대한 예외처리 */
  if(list_empty(&birthday_list)) {
    printk("List is Empty\n");
    return;
  }

  /* 4. TODO: 이제 본격적으로 연결리스트를 탐색하면서 하나씩 제거하도록 하시면 됩니다. */
  struct birthday *cur_node;
  struct list_head *cursor, *next;
  list_for_each_safe(cursor, next, &birthday_list){
    cur_node = list_entry(cursor, struct birthday, list);
    printk("Removing %d.%d.%d\n", cur_node->day, cur_node->month, cur_node->year);
    list_del(cursor);
    kfree(cur_node); 
  } 
  /* 다만, 제거를 하면서 연결리스트를 탐색하면 문제가 생길 것 같은데 어떤 방법으로 해결 가능한지 생각해보세요. */

  printk("REMOVE MODULE: bdlist\n");
  return; 
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("make a list of birthdays and print");
MODULE_AUTHOR("안동현_2018037356"); // 이름_학번 형식으로 적어서 제출하세요.
