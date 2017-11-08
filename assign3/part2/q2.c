#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "inttypes.h"


// reference - https://stackoverflow.com/questions/2440385/how-to-find-the-physical-address-of-a-variable-from-user-space-in-linux
uintptr_t vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    int offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;
    // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    // add offset within page
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }
            }
        }
        fclose(pagemap);
    }
    return paddr;
}

int main() {
    static int k = 0;
    int* b = &k;
    int* physical = (int*) vtop((uintptr_t) b);
    printf("Virtual address - %p\n", (void *) b);
    printf("Physical address - %p\n", (void *) physical);
    return 0;
}