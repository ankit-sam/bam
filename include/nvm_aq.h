#ifndef __NVM_AQ_H__
#define __NVM_AQ_H__
// #ifndef __CUDACC__
// #define __device__
// #define __host__
// #endif

#include <nvm_types.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/*
 * Shared admin queue-pair descriptor.
 */
struct local_admin
{
    pthread_mutex_t     mutex;      // Mutex for this shared memory segment
    nvm_dma_t*          qmem;       // Primary process queue memory
    nvm_dma_t*          shared_qmem;// Secondary process queue memory
    nvm_queue_t         acq;        // Admin completion queue (ACQ)
    volatile uint32_t*  acq_db1;    // Pointer to secondary acq doorbell register (NB! write only)
    volatile void*      acq_vaddr1; // Virtual address to start of acq for secondary process
    nvm_queue_t         asq;        // Admin submission queue (ASQ)
    volatile uint32_t*  asq_db1;    // Pointer to secondary asq doorbell register (NB! write only)
    volatile void*      asq_vaddr1; // Virtual address to start of asq for secondary process
    uint64_t            timeout;    // Controller timeout
    uint32_t            n_qps;      // Number of IO qpairs
    uint8_t             qids[128];  // QID available or used

};


/*
 * Create admin queue pair
 *
 * Take exclusive ownership of an NVM controller. This function resets the 
 * controller and configures NVM admin queues. 
 *
 * Returns a reference handle that can be used for admin RPC calls.
 */
int nvm_aq_create(nvm_aq_ref* ref, 
                  const nvm_ctrl_t* ctrl, 
                  const nvm_dma_t* dma_window);


/*
 * Configure admin queue pair
 *
 * This function resets the controller and configures NVM admin queues.
 * This doesn't take exclusive ownership of an NVM controller.
 * The shared memory segment for admin qpair must be passed.
 *
 * Returns a reference handle that can be used for admin RPC calls.
 */
int nvm_aq_create_new(nvm_aq_ref* handle,
		      const nvm_ctrl_t* ctrl, const nvm_dma_t* window,
		      struct local_admin *admin);


/*
 * Share already configured admin queue pair
 *
 * Returns a reference handle that can be used for admin RPC calls.
 */
int nvm_aq_share(nvm_aq_ref* handle, const nvm_ctrl_t* ctrl,
		 const nvm_dma_t* window, struct local_admin *admin);


/*
 * Destroy admin queues and references.
 *
 * Send NVM abort command to controller and deallocate admin queues.
 *
 * After calling this function, all admin queue references are invalid.
 * This also means that remote references will no longer be valid.
 *
 * This function will also work for unbinding remote references.
 */
void nvm_aq_destroy(nvm_aq_ref ref);



//int nvm_tcp_rpc_enable(nvm_aq_ref ref, uint16_t port, nvm_rpc_cb_t filter, void* data);
//int nvm_tcp_rpc_disable(nvm_aq_ref ref, uint16_t port);



#ifdef __DIS_CLUSTER__


/*
 * Callback function invoked whenever a remote NVM admin command is received.
 * Should indicate whether or not a remote admin command is accepted and can
 * be enqueued by using the return value.
 *
 * The remote command can also be modified if necessary.
 */
typedef bool (*nvm_dis_rpc_cb_t)(nvm_cmd_t* cmd, uint32_t dis_adapter, uint32_t dis_node_id);



/*
 * Enable remote admin commands.
 * Allows remote processes to relay NVM admin commands to the local process.
 */
int nvm_dis_rpc_enable(nvm_aq_ref ref,               // NVM admin queue-pair reference
                       uint32_t dis_adapter,         // Local adapter to enable interrupt on
                       nvm_dis_rpc_cb_t filter);     // Filter callback (can be NULL)



/*
 * Disable remote admin commands.
 * Stop processing admin commands from remote processes.
 */
void nvm_dis_rpc_disable(nvm_aq_ref ref, uint32_t dis_adapter);

#endif /* __DIS_CLUSTER__ */




#endif /* #ifdef __NVM_AQ_H__ */
