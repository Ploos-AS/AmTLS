#include <exec/libraries.h>
#include <exec/resident.h>
#include <exec/types.h>
#include <proto/exec.h>

#define STR_(x) #x
#define STR(x) STR_(x)

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

int __attribute__((no_reorder)) _start(void)
{
    return -1;
}

extern const ULONG amtls_auto_init[4];

asm("amtls_romtag:                         \n"
    "       dc.w    " STR(RTC_MATCHWORD) " \n"
    "       dc.l    amtls_romtag           \n"
    "       dc.l    amtls_endcode          \n"
    "       dc.b    " STR(RTF_AUTOINIT) "  \n"
    "       dc.b    " STR(AMTLS_VERSION) " \n"
    "       dc.b    " STR(NT_LIBRARY) "    \n"
    "       dc.b    " STR(AMTLS_PRIORITY) "\n"
    "       dc.l    library_name           \n"
    "       dc.l    library_id             \n"
    "       dc.l    amtls_auto_init        \n"
    "amtls_endcode:                        \n");

static BPTR amtls_do_expunge(struct AmTLSLibraryBase *base)
{
    BPTR seg_list;

    if (base->library.lib_OpenCnt != 0) {
        base->library.lib_Flags |= LIBF_DELEXP;
        return 0;
    }

    seg_list = base->seg_list;
    Remove(&base->library.lib_Node);
    FreeMem((UBYTE *)base - base->library.lib_NegSize,
            base->library.lib_NegSize + base->library.lib_PosSize);
    return seg_list;
}

static struct AmTLSLibraryBase * __attribute__((used))
amtls_init(struct ExecBase *sys_base asm("a6"),
           BPTR seg_list asm("a0"),
           struct AmTLSLibraryBase *base asm("d0"))
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

static struct AmTLSLibraryBase * __attribute__((used))
amtls_open(struct AmTLSLibraryBase *base asm("a6"))
{
    base->library.lib_OpenCnt++;
    base->library.lib_Flags &= (UBYTE)~LIBF_DELEXP;
    return base;
}

static BPTR __attribute__((used))
amtls_close(struct AmTLSLibraryBase *base asm("a6"))
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

static BPTR __attribute__((used))
amtls_expunge(struct AmTLSLibraryBase *base asm("a6"))
{
    return amtls_do_expunge(base);
}

static ULONG __attribute__((used))
amtls_reserved(void)
{
    return 0;
}

static const ULONG amtls_vectors[] = {
    (ULONG)amtls_open,
    (ULONG)amtls_close,
    (ULONG)amtls_expunge,
    (ULONG)amtls_reserved,
    (ULONG)-1
};

const ULONG amtls_auto_init[4] = {
    sizeof(struct AmTLSLibraryBase),
    (ULONG)amtls_vectors,
    0,
    (ULONG)amtls_init
};
