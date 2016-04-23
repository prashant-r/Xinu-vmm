/* paging.h */

//typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

#define DEBUG_MODE_ON true

#define STARTING_PAGE 4096
#define NUM_GLOBAL_PAGE_TABLES 4
#define NUM_GLOBAL_AND_DEVICE_TABLES NUM_GLOBAL_PAGE_TABLES + 1
#define PAGE_SIZE 4096
#define PAGETABLE_ENTRIES_SIZE 1024
#define PAGEDIRECTORY_ENTRIES_SIZE 1024
#define BS_MAX_STORES 8
#define BS_MAX_PAGES 200
#define PAGEFAULT_XNUM 14


typedef enum {DIR, PPTBLCAT1, PPTBLCAT2, VPTBL, PAGE, FREE} frame_type;


typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;



#ifdef DEBUG_MODE_ON
#define LOG(STR, ...) do { kprintf("[ <DEBUG IN FUNCTION %s | PID %d> ]  : ", __func__,  currpid) ; kprintf(#STR, ##__VA_ARGS__) ; kprintf("\n"); } while (0)
#else
#define LOG			  asm("NOP") // This is production/submission version.
#endif


#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES		600	/* number of frames		*/

#define DEVICE_FRAME (576)
#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define LRU 4
#define GLCLOCK 5

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID          0


/* Structure for a frame */
typedef struct _frame_t{
	int32 id;
	uint32 vp_no;
	pid32 pid;
	frame_type type;
	bool8 dirty;
	bsd_t backstore;
	uint32 backstore_offset;
} frame_t;


/* Prototypes for required API calls */
//SYSCALL xmmap(int, bsd_t, int);
//SYSCALL xmunmap(int);

// api specific to lab 5 externs

extern int srlpolicy;

// in paging_tests.c

extern void all_paging_tests();

// in vcreate.c

extern void vcreate_tests();
extern void vcprocA(void);
extern void vcprocB(void);
extern void vcprocC(void);

// in paging_register_setup.c

extern void enable_paging(void);
extern void disable_paging(void);
extern void set_cr0(unsigned int n);
extern unsigned int read_cr0();
extern unsigned int read_cr2(void);
extern void set_cr3(unsigned int n);
extern unsigned int read_cr3(void);
extern void switch_page_directory(unsigned int pd_addr);
extern void flush_tlb();

// in pagefault_handler.c
extern syscall pagefault_handler(void);

// in page_table.c
extern int initialize_global_pagetables(void);
extern int initialize_device_pagetable(void);

// in page_frame.c
extern frame_t * retrieve_new_frame(frame_type type);
extern void initialize_all_frames(void);
extern void initialize_frame(frame_t * frameptr);
extern frame_t frames[NFRAMES];
extern int free_frame(int frame_id);
extern syscall frame_map_check(int pid, int store, int page_offset_in_store, int * pageframe_id );
// in dump32.c
extern void dump32(unsigned long n);


// in page_directory.c
extern void initialize_page_directory(pd_t * page_dir);
extern pd_t * retrieve_new_page_directory(void);
extern int free_page_directory(pid32 pid);

// in pagefault_handler.c
typedef struct __virtu_addr{
  unsigned int page_offset : 12;
  unsigned int page_table_offset : 10;
  unsigned int page_directory_offset : 10;
} virtual_addr;

extern void * addressTranslate ( uint32 address);

// in bs_map.c
extern syscall do_bs_map(int pid, int vp_no, bsd_t bs_id, int npages);
extern syscall bs_map_check(int pid, unsigned int vir_add, int * store, int * page_offset_in_store );
// shorthand helpers
#define FRAMEID_TO_PHYSICALADDR(f)          ((FRAME0 + f) * PAGE_SIZE)
#define FRAMEID_TO_VPAGE(f)					((FRAME0 + f))
#define FRAMEID_IS_VALID(frame_id)          ((frame_id >= 0) && (frame_id < NFRAMES))
#define PA_TO_FRAMEID(PA)                   (VADDRESS_TO_VPAGE(PA) - FRAME0)
#define VADDRESS_TO_VPAGE(va)				((unsigned int) va/ PAGE_SIZE)
#define VPAGE_TO_VADDRESS(vp)       (vp*PAGE_SIZE)
