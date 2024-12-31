#include "vms.h"
#include "mmu.h"
#include <bits/stdc++.h>
#include "pages.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;
int page_reference_count[MAX_PAGES] = {0};
int referenced[256];

/* A debugging helper that will print information about the pointed to PTE
   entry. */
static void print_pte_entry(uint64_t *entry);

void checkReference(void *page_table)
{
    referenced[vms_get_page_index(page_table)] -= 1;
    if (referenced[vms_get_page_index(page_table)] == 0)
    {
        uint64_t *table_casted = (uint64_t *)page_table;
        for (int i = 0; i < NUM_PTE_ENTRIES; i++)
        {
            uint64_t *pte = &table_casted[i];

            if (vms_pte_custom(pte))
            {
                vms_pte_custom_clear(pte);
                vms_pte_valid_set(pte);
            }
        }
    }
}

void *recurseCopyOnWrite(void *virtual_address, int level, void *page_table)
{
    if (level == -1)
    {
        void *new_page = vms_new_page();
        memmove(new_page, page_table, PAGE_SIZE);
        return new_page;
    }
    uint64_t pte = *vms_page_table_pte_entry(page_table, virtual_address, level);
    uint64_t *new_page = (uint64_t *)vms_new_page();
    int index = vms_page_table_index(virtual_address, level);
    memmove(new_page, page_table, PAGE_SIZE);
    vms_pte_custom_clear(&pte);
    vms_pte_valid_set(&pte);
    new_page[index] = pte;
    checkReference(page_table);
    void *corresponding_page = recurseCopyOnWrite(virtual_address, level - 1, vms_ppn_to_page(vms_pte_get_ppn(&pte)));
    vms_pte_set_ppn(&new_page[index], vms_page_to_ppn(corresponding_page));
    return new_page;
}

void page_fault_handler(void *virtual_address, int level, void *page_table)
{
    uint64_t pte = *vms_page_table_pte_entry(page_table, virtual_address, level);

    if (vms_pte_custom(&pte))
    {
        cout << "whatever" << endl;
        recurseCopyOnWrite(virtual_address, level, page_table);
    }
    else
    {
        exit(EFAULT);
    }
}

void *recurseCopy(int level, void *page_table)
{
    uint64_t *curr_page = (uint64_t *)page_table;
    uint64_t *new_page = (uint64_t *)vms_new_page();
    if (level == -1)
    {
        memmove(new_page, curr_page, PAGE_SIZE);
        return new_page;
    }

    for (int i = 0; i < NUM_PTE_ENTRIES; i++)
    {
        uint64_t pte = curr_page[i];
        uint64_t new_pte = pte;
        bool isValid = vms_pte_valid(&pte);
        if (isValid)
        {
            void *corresponding_page_table = vms_ppn_to_page(vms_pte_get_ppn(&pte));
            void *new_corresponding_page = recurseCopy(level - 1, corresponding_page_table);
            vms_pte_set_ppn(&new_pte, vms_page_to_ppn(new_corresponding_page));
        }
        new_page[i] = new_pte;
    }
    return new_page;
}

void *vms_fork_copy(void)
{
    void *l2 = vms_get_root_page_table();
    return recurseCopy(2, l2);
}

bool recurseModify(int level, void *page_table)
{
    if (level == -1)
    {
        return 0;
    }
    uint64_t *table_casted = (uint64_t *)page_table;

    referenced[vms_get_page_index(page_table)] += 1;

    bool state = 0;
    for (int i = 0; i < NUM_PTE_ENTRIES; i++)
    {
        if (!vms_pte_valid(table_casted))
        {
            table_casted += 1;
            continue;
        }
        if (vms_pte_write(table_casted))
        {
            vms_pte_custom_set(table_casted);
            vms_pte_valid_clear(table_casted);
            state = 1;
        }

        bool myState = recurseModify(level - 1, vms_ppn_to_page(vms_pte_get_ppn(table_casted)));

        if (myState)
        {
            vms_pte_custom_set(table_casted);
            vms_pte_valid_clear(table_casted);
            state |= myState;
        }
        table_casted += 1;
    }
    return state;
};

void *vms_fork_copy_on_write(void)
{
    void *l2 = vms_get_root_page_table();
    recurseModify(2, l2);
    return l2;
}

static void print_pte_entry(uint64_t *entry)
{
    const char *dash = "-";
    const char *custom = dash;
    const char *write = dash;
    const char *read = dash;
    const char *valid = dash;
    if (vms_pte_custom(entry))
    {
        custom = "C";
    }
    if (vms_pte_write(entry))
    {
        write = "W";
    }
    if (vms_pte_read(entry))
    {
        read = "R";
    }
    if (vms_pte_valid(entry))
    {
        valid = "V";
    }

    printf("PPN: 0x%lX Flags: %s%s%s%s\n",
           vms_pte_get_ppn(entry),
           custom, write, read, valid);
}
