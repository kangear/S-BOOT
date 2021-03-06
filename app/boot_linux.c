#include <atag.h>
#include <string.h>
#include <stdio.h>
#include <configs.h>
#include <sboot.h>

static struct Atag *pCurTag; /* used to point at the current ulTag */

void (*theKernel)(int, int, unsigned int);

//const char *CmdLine = "root=/dev/nfs console=ttySAC0,115200 mem=65536K init=/linuxrc";   
const char *CmdLine = "root=/dev/nfs nfsroot=192.168.1.249:/home/hongwang/mkrootfs/rootfs ip=192.168.1.252:192.168.1.249:192.168.1.1:255.255.255.0:hwlee.net:eth0:off console=ttySAC0,115200 init=/linuxrc mem=65536K console=tty1 fbcon=rotate:2";

static void setup_core_tag(void *pStartAddr)
{
    pCurTag = (struct Atag *)pStartAddr;      /* start at given address */

    pCurTag->stHdr.ulTag = ATAG_CORE;            /* start with the stCore ulTag */
    pCurTag->stHdr.nSize = TAG_SIZE(TagCore); /* nSize the ulTag */

    pCurTag->u.stCore.ulFlags = 1;               /* ensure read-only */
    pCurTag->u.stCore.nPageSize = 4096;     /* systems pagenSize (4k) */
    pCurTag->u.stCore.ulRootDev = 0;     /* zero (typicaly overidden from commandline )*/

    pCurTag = TAG_NEXT(pCurTag);              /* move pointer to next ulTag */
}

static void setup_mem_tag(unsigned int start, unsigned int len)
{
    pCurTag->stHdr.ulTag = ATAG_MEM;             /* Memory ulTag */
    pCurTag->stHdr.nSize = TAG_SIZE(TagMem32);  /* nSize ulTag */

    pCurTag->u.stMem.ulStart = start;     /* Start of memory area (physical address) */
    pCurTag->u.stMem.nSize = len;               /* Length of area */

    pCurTag = TAG_NEXT(pCurTag);              /* move pointer to next ulTag */
}

static void setup_cmdline_tag(const char * line)
{
    int linelen = strlen(line);

    if(!linelen)
        return;                  /* do not insert a ulTag for an empty commandline */

    pCurTag->stHdr.ulTag = ATAG_CMDLINE;         /* Commandline ulTag */
    pCurTag->stHdr.nSize = (sizeof(struct TagHeader) + linelen + 1+4) >> 2;

    strcpy(pCurTag->u.stCmdLine.cCmdLine,line); /* place commandline into ulTag */

    pCurTag = TAG_NEXT(pCurTag);              /* move pointer to next ulTag */
}

static void setup_end_tag(void)
{
    pCurTag->stHdr.ulTag = ATAG_NONE;            /* Empty ulTag ends list */
    pCurTag->stHdr.nSize = 0;                   /* zero length */
}

int boot_linux()
{
    setup_core_tag( (void *)DRAM_TAGS_START );   /* standard stCore ulTag 4k pagenSize */
    setup_mem_tag( DRAM_ADDR_START, DRAM_TOTAL_SIZE);    /* 64Mb at 0x10000000 */
    setup_cmdline_tag( CmdLine );   
    setup_end_tag();                    /* end of ulTags */
    
    nand_init();

    printf("Before nand_read\n\r");
    nand_read((unsigned char *)DRAM_KERNEL_START, NAND_KERNEL_START, NAND_KERNEL_SZIE);
    printf("After nand_read\n\r");
    
    theKernel = (void (*)(int, int, unsigned int))DRAM_KERNEL_START;

    theKernel(0, S3C2440_MATHINE_TYPE, DRAM_TAGS_START);

    return 0;
}

int boot_linux_serial()
{

    printf("Before GtSerialLoad\n\r");
    GtSerialLoad((void *)DRAM_KERNEL_START);
    printf("After GtSerialLoad\n\r");

    return 0;
}

int boot_linux_ram()
{
    setup_core_tag( (void *)DRAM_TAGS_START );  /* standard stCore ulTag 4k pagenSize    DRAM_TAGS_START=0x30000100 */ 
    setup_mem_tag( DRAM_ADDR_START, DRAM_TOTAL_SIZE);    /* 64Mb at 0x10000000 */
    setup_cmdline_tag( CmdLine );   
    setup_end_tag();                    /* end of ulTags */
   
    //printf("\n\r Jump to Kernel ... \n");
    theKernel = (void (*)(int, int, unsigned int))DRAM_KERNEL_START;       //DRAM_KERNEL_START=0x33800000

    theKernel(0, S3C2440_MATHINE_TYPE, DRAM_TAGS_START);

    return 0;
}


