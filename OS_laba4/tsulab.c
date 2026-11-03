#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/atomic.h>

#define NR_WORKS 5
#define MAX_ACTIVE 2
#define MIN_STAGES 3
#define MAX_STAGES 6

struct lab_work {
	struct work_struct work;
	unsigned int id;
	unsigned int stages;
	unsigned int completed;
};

static struct workqueue_struct *tsulab_wq;
static struct lab_work items[NR_WORKS];
static atomic_t stop_all = ATOMIC_INIT(0);

static void lab_work_handler(struct work_struct *work)
{
	struct lab_work *item = container_of(work, struct lab_work, work);
	unsigned int stage;

	for (stage = 1; stage <= item->stages; stage++) {
		if (atomic_read(&stop_all)) {
			pr_info("tsulab: work %u cancelled before stage %u/%u\n", item->id, stage, item->stages);
			return;
		}

		pr_info("tsulab: work %u: stage %u/%u\n", item->id, stage, item->stages);
		msleep(1000);
		item->completed = stage;
	}

	pr_info("tsulab: work %u comleted (%u/%u)\n", item->id, item->completed, item->stages);
}

static int __init tsulab_init(void)
{
	int i;

	tsulab_wq = alloc_workqueue("tsulab_wq", WQ_UNBOUND, MAX_ACTIVE);
	if (!tsulab_wq) {
		pr_err("tsulab: cannot create workqueue\n");
		return -ENOMEM;
	}

	atomic_set(&stop_all, 0);

	for (i = 0; i < NR_WORKS; i++) {
		items[i].id = i;
		items[i].stages = MIN_STAGES + (get_random_u32() % (MAX_STAGES - MIN_STAGES + 1));
		items[i].completed = 0;

		INIT_WORK(&items[i].work, lab_work_handler);

		if (!queue_work(tsulab_wq, &items[i].work))
			pr_warn("tsulab: work %d was already queue\n", i);
	}

	pr_info("tsulab: loaded; queued %d works, max_active=%d\n", NR_WORKS, MAX_ACTIVE);

	return 0;
}

static void __exit tsulab_exit(void)
{
	int i;

	pr_info("tsulab: unloading; set stop flag\n");
	atomic_set(&stop_all, 1);

	for (i = 0; i < NR_WORKS; i++)
		cancel_work_sync(&items[i].work);

	for (i = 0; i < NR_WORKS; i++) {
		if (items[i].completed == items[i].stages) {
			pr_info("tsulab: final work %u: completed %u/%u\n", items[i].id, items[i].completed, items[i].stages);
		} else {
			pr_info("tsulab: final work %u: cancelled/stopped %u/%u\n", items[i].id, items[i].completed, items[i].stages);
		}
	}

	destroy_workqueue(tsulab_wq);

	pr_info("tsulab: unloaded\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Long works in workqueue demo");
MODULE_AUTHOR("SVYATOSLAV");
