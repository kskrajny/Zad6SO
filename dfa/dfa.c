#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minix/ds.h>
#include <sys/ioctl.h>
#include <sys/ioc_dfa.h>
#include <stdbool.h>

/* consts */
#define BUF     256         // size of buffer
#define NODES   256         // num of possible nodes

/* variables */
unsigned char node = 0;              // curr node
int it = 0;                 // iterator of curr word
unsigned char in[BUF];               // word to proceed
char out[BUF];              // signs to write      
char accept[NODES];         // accepts nodes
unsigned char func[NODES*NODES];     // move function

/*
 * Function prototypes for the hello driver.
 */
static int dfa_open(devminor_t minor, int access, endpoint_t user_endpt);
static int dfa_close(devminor_t minor);
static ssize_t dfa_read(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t dfa_write(devminor_t minor, u64_t position, endpoint_t endpt,
    cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int dfa_ioctl(devminor_t minor, unsigned long request, endpoint_t ep,
    cp_grant_id_t gid, int flags, endpoint_t user_ep, cdev_id_t id);
static void proceed();

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);

/* Entry points to the hello driver. */
static struct chardriver dfa_tab =
{
    .cdr_open	= dfa_open,
    .cdr_close	= dfa_close,
    .cdr_read	= dfa_read,
    .cdr_write	= dfa_write,
    .cdr_ioctl  = dfa_ioctl
};


static int
dfa_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
    cp_grant_id_t grant, int UNUSED(flags), endpoint_t UNUSED(user_ep),
    cdev_id_t UNUSED(id))    
{           
    /* Process I/O control requests */                               
    int r;                      
    char acc;
    unsigned char move[3];
    char rej;

    switch(request) {
        case DFAIOCRESET:
            node = 0;
            break;
        case DFAIOCADD:
            r = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) move, 3);
            if (r != OK)
                return r;
            func[move[0] + NODES*move[1]] = move[2];
            node = 0;
            break;
        case DFAIOCACCEPT:
            r = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &acc, 1);
            if (r != OK)
                return r;
            accept[acc] = 1;
            break;
        case DFAIOCREJECT:
            r = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &rej, 1);
            if (r != OK)
                return r;
            accept[rej] = 0;
            break;
    }
    return 0;
}

static int dfa_open(devminor_t UNUSED(minor), int UNUSED(access),
    endpoint_t UNUSED(user_endpt))
{
    return OK;
}

static int dfa_close(devminor_t UNUSED(minor))
{
    return OK;
}

static ssize_t dfa_read(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    int ret;
    /* check if current node accepts */
    char sign = (accept[node]) ? 'Y' : 'N';
    /* match size of data to write */
    size = (size > BUF) ? BUF : size;
    memset(out, 0, BUF);
    memset(out, sign, size);
    /* write data */
    if((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes)out, size)) != OK)
        return ret;
    /* Return the number of bytes read. */
    return size;
}

static ssize_t dfa_write(devminor_t UNUSED(minor), u64_t UNUSED(position),
    endpoint_t endpt, cp_grant_id_t grant, size_t size, int UNUSED(flags),
    cdev_id_t UNUSED(id))
{
    int r;
    size = (size > BUF) ? BUF : size;
    r = sys_safecopyfrom(endpt, grant, 0, (vir_bytes)(in), size);
    proceed();
    return (r != OK) ? r : size;
}

void proceed()
{
    while(it < strlen(in)) {
        node = func[node + NODES*in[it]];
        it++;
    }
    memset(in, 0, BUF);
    it = 0;
}

int ds_publish_mem(const char *ds_name, void *vaddr, size_t length, int flags);
static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_mem("func", (void *)func, NODES*NODES, 0);
    ds_publish_mem("accept", (void *)accept, NODES, 0);
    ds_publish_mem("in", (void *)in, BUF, 0);
    ds_publish_mem("node", (void *)&node, 1, 0);
    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    size_t len;
    ds_retrieve_mem("func", func, &len);
    ds_delete_mem("func");
    ds_retrieve_mem("accept", accept, &len);
    ds_delete_mem("accept");
    ds_retrieve_mem("in", in, &len);
    ds_delete_mem("in");
    ds_retrieve_mem("node", &node, &len);
    ds_delete_mem("node");
    return OK;
}

static void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
/* Initialize the hello driver. */
    int do_announce_driver = TRUE;

    switch(type) {
        case SEF_INIT_FRESH:
            memset(accept, 0, NODES);
            memset(func, 0, NODES*NODES);
        break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
        break;

        case SEF_INIT_RESTART:
            break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

int main(void)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /*
     * Run the main loop.
     */
    chardriver_task(&dfa_tab);
    return OK;
}

