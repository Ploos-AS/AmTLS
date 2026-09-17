#include <exec/libraries.h>
#include <exec/resident.h>
#include <exec/types.h>
#include <dos/dos.h>

#define AMTLS_NAME "amtls.library"
#define AMTLS_VERSION 0
#define AMTLS_REVISION 1
#define AMTLS_PRIORITY 0

struct AmTLSLibraryBase {
    struct Library library;
    BPTR seg_list;
    struct ExecBase *sys_base;
};

static char library_name[] = AMTLS_NAME;
static char library_id[] = "amtls.library 0.1\r\n";

static struct AmTLSLibraryBase *amtls_init(
    struct ExecBase *sys_base __asm__("a6"),
    BPTR seg_list __asm__("a0"),
    struct AmTLSLibraryBase *base __asm__("d0"));
static struct AmTLSLibraryBase *amtls_open(
    struct AmTLSLibraryBase *base __asm__("a6"));
static BPTR amtls_close(struct AmTLSLibraryBase *base __asm__("a6"));
static BPTR amtls_expunge(struct AmTLSLibraryBase *base __asm__("a6"));
static ULONG amtls_reserved(void);

static const APTR amtls_vectors[] = {
    (APTR)amtls_open,
    (APTR)amtls_close,
    (APTR)amtls_expunge,
    (APTR)amtls_reserved,
    (APTR)-1
};

static const ULONG amtls_auto_init[4] = {
    sizeof(struct AmTLSLibraryBase),
    (ULONG)amtls_vectors,
    0,
    (ULONG)amtls_init
};

static const struct Resident amtls_romtag
    __attribute__((used, section(".text"))) = {
        RTC_MATCHWORD,
        (struct Resident *)&amtls_romtag,
        (APTR)(&amtls_romtag + 1),
        RTF_AUTOINIT,
        AMTLS_VERSION,
        NT_LIBRARY,
        AMTLS_PRIORITY,
        library_name,
        library_id,
        (APTR)amtls_auto_init
    };

int __attribute__((no_reorder)) _start(void)
{
    return -1;
}

static void exec_remove(struct ExecBase *sys_base, struct Node *node)
{
    register struct ExecBase *a6 __asm__("a6") = sys_base;
    register struct Node *a1 __asm__("a1") = node;

    __asm__ volatile("jsr -252(a6)"
                     :
                     : "r"(a6), "r"(a1)
                     : "d0", "d1", "a0", "a1", "cc", "memory");
}

static void exec_free_mem(struct ExecBase *sys_base, APTR memory, ULONG size)
{
    register struct ExecBase *a6 __asm__("a6") = sys_base;
    register APTR a1 __asm__("a1") = memory;
    register ULONG d0 __asm__("d0") = size;

    __asm__ volatile("jsr -210(a6)"
                     :
                     : "r"(a6), "r"(a1), "r"(d0)
                     : "d1", "a0", "a1", "cc", "memory");
}

static BPTR amtls_do_expunge(struct AmTLSLibraryBase *base)
{
    BPTR seg_list;
    ULONG total_size;
    UBYTE *allocation;
    struct ExecBase *sys_base;

    if (base->library.lib_OpenCnt != 0) {
        base->library.lib_Flags |= LIBF_DELEXP;
        return 0;
    }

    seg_list = base->seg_list;
    sys_base = base->sys_base;
    total_size = (ULONG)base->library.lib_NegSize +
                 (ULONG)base->library.lib_PosSize;
    allocation = (UBYTE *)base - base->library.lib_NegSize;

    exec_remove(sys_base, &base->library.lib_Node);
    exec_free_mem(sys_base, allocation, total_size);
    return seg_list;
}

static struct AmTLSLibraryBase *
amtls_init(struct ExecBase *sys_base __asm__("a6"),
           BPTR seg_list __asm__("a0"),
           struct AmTLSLibraryBase *base __asm__("d0"))
{
    base->seg_list = seg_list;
    base->sys_base = sys_base;
    base->library.lib_Node.ln_Type = NT_LIBRARY;
    base->library.lib_Node.ln_Name = library_name;
    base->library.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    base->library.lib_Version = AMTLS_VERSION;
    base->library.lib_Revision = AMTLS_REVISION;
    base->library.lib_IdString = (APTR)library_id;
    return base;
}

static struct AmTLSLibraryBase *
amtls_open(struct AmTLSLibraryBase *base __asm__("a6"))
{
    base->library.lib_OpenCnt++;
    base->library.lib_Flags &= (UBYTE)~LIBF_DELEXP;
    return base;
}

static BPTR amtls_close(struct AmTLSLibraryBase *base __asm__("a6"))
{
    if (base->library.lib_OpenCnt != 0) {
        base->library.lib_OpenCnt--;
    }
    if (base->library.lib_OpenCnt == 0 &&
        (base->library.lib_Flags & LIBF_DELEXP)) {
        return amtls_do_expunge(base);
    }
    return 0;
}

static BPTR amtls_expunge(struct AmTLSLibraryBase *base __asm__("a6"))
{
    return amtls_do_expunge(base);
}

static ULONG amtls_reserved(void)
{
    return 0;
}
