#include <ipc/mailbox.h>
#include <utility.h>

mailbox_t* mailbox_slots;           // mailbox slots
spinlock_t ms_lock;                 // mailbox slots lock

void mailbox_init()
{
    uint32_t mailbox_pages = ceil_division(MAX_MAILBOX_SLOTS * sizeof(mailbox_t), virt_mem_get_page_size());
	ms_lock = 0;

	// allocate enough virtual space right after the kernel for the mailbox slots
	mailbox_slots = alloc_perm();
	for(uint32_t i = 0; i < mailbox_pages - 1; i++)
		alloc_perm();

    // mark all processes and threads as empty and ready for use
	for(uint32_t i = 0; i < MAX_MAILBOX_SLOTS; i++)
    {
		mailbox_slots[i].type = MAILBOX_NONE;
        mailbox_slots[i].mid = i;
    }
}

mailbox_t* mailbox_create_static(mid_t mid, mailbox_type_t type, void* owner)
{
    acquire_spinlock(&ms_lock);

    if(mailbox_slots[mid].type)
    {
        release_spinlock(&ms_lock);
		return 0;
    }

	mailbox_t* mbox = &mailbox_slots[mid];
	mbox->type = type;

    release_spinlock(&ms_lock);

    mbox->owner = owner;
    
    if(type > __MAILBOX_ABOVE_SEMAPHORE__)
    {
        semaphore_init(&mbox->lock.sem_rdy, 1);
	    semaphore_init(&mbox->lock.sem_recv, 0);
    }
    else
    {
        mbox->lock.msg_lock = 0;
    }

    return mbox;
}

mailbox_t* mailbox_create(mailbox_type_t type, void* owner)
{
    mailbox_t* mbox = 0;
	for(uint32_t i = 0; i < MAX_MAILBOX_SLOTS; i++)
		if(mbox = mailbox_create_static(i, type, owner))
			return mbox;

	return 0;
}

mailbox_t* get_mailbox(mid_t mid)
{
    if(!mailbox_slots[mid].type)
        return 0;

    return &mailbox_slots[mid];
}