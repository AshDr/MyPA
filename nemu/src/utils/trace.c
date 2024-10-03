#include <stdio.h>
#include <utils.h>
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>

static int32_t call_depth = 0;
static FILE *ftrace_log = NULL;

#define ftrace_write(...) \
    do { \
        if (ftrace_log == NULL) { \
            ftrace_log = fopen("/home/ashdr/code/ics2023/nemu/build/ftrace-log.txt", "w"); \
        } \
        assert(ftrace_log != NULL); \
        fprintf(ftrace_log, __VA_ARGS__); \
        fflush(ftrace_log); \
    } while (0)

typedef struct {
    char name[32];
    paddr_t addr;
    Elf32_Word size;
    unsigned char info;
}SymEntry; // symbol entry

SymEntry *g_symbol_table = NULL;
int g_symbol_tbl_size = 0;

static void read_elf_header(int fd, Elf32_Ehdr *ehdr) {
    assert(lseek(fd, 0, SEEK_SET) == 0);
    assert(read(fd, ehdr, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));
    if(strncmp((char*)ehdr->e_ident, "\177ELF", 4) != 0) {
        panic("not an ELF file");
    }
}

static void display_elf_header(Elf32_Ehdr eh) {
    ftrace_write("Storage class \t=");
    switch (eh.e_ident[EI_CLASS]) { 
        case ELFCLASS32: ftrace_write("ELF32\n"); break;
        case ELFCLASS64: ftrace_write("ELF64\n"); break;
        default: ftrace_write("Invalid class\n"); break;
    }
    ftrace_write("Data encoding \t=");
    switch (eh.e_ident[EI_DATA]) {
        case ELFDATA2LSB: ftrace_write("2's complement, little endian\n"); break;
        case ELFDATA2MSB: ftrace_write("2's complement, big endian\n"); break;
        default: ftrace_write("Invalid data encoding\n"); break;
    }
    ftrace_write("OS/ABI\t=");
    switch (eh.e_ident[EI_OSABI]) {
        case ELFOSABI_SYSV: ftrace_write("UNIX System V ABI\n"); break;
        case ELFOSABI_HPUX: ftrace_write("HP-UX\n"); break;
        case ELFOSABI_NETBSD: ftrace_write("NetBSD\n"); break;
        case ELFOSABI_LINUX: ftrace_write("Linux\n"); break;
        case ELFOSABI_SOLARIS: ftrace_write("Sun");break;
        default: ftrace_write("Other\n"); break;
    }
    ftrace_write("File type\t=");
    switch (eh.e_type) {
        case ET_NONE: ftrace_write("No file type\n"); break;
        case ET_REL: ftrace_write("Relocatable file\n"); break;
        case ET_EXEC: ftrace_write("Executable file\n"); break;
        case ET_DYN: ftrace_write("Shared object file\n"); break;
        case ET_CORE: ftrace_write("Core file\n"); break;
        default: ftrace_write("Other\n"); break;
    }
}

static void read_section(int fd, Elf32_Shdr shdr, void *dst) {
    assert(dst != NULL);
    assert(lseek(fd, shdr.sh_offset, SEEK_SET) == shdr.sh_offset);
    assert(read(fd, dst, shdr.sh_size) == shdr.sh_size);
}
static void read_section_header(int fd, Elf32_Ehdr ehdr, Elf32_Shdr shdr_tbl[]) {
    assert(lseek(fd, ehdr.e_shoff, SEEK_SET) == ehdr.e_shoff);
    for(int i = 0; i < ehdr.e_shnum; ++i) {
        assert(read(fd, &shdr_tbl[i], ehdr.e_shentsize) == ehdr.e_shentsize);
    }
    // 把每个section table entry都读到内存中
}

static void display_section_header(int fd, Elf32_Ehdr ehdr, Elf32_Shdr shdr_tbl[]) {
    char sh_str[shdr_tbl[ehdr.e_shstrndx].sh_size]; // 使用一个大小为section name table大小的字节数组，即里面存放的是所有section的名字
    ftrace_write("========================================");
	ftrace_write("========================================\n");
	ftrace_write(" idx offset     load-addr  size       algn"
			" flags      type       section\n");
	ftrace_write("========================================");
	ftrace_write("========================================\n");
    for(int i = 0; i < ehdr.e_shnum; i++) {
		ftrace_write(" %03d ", i);
		ftrace_write("0x%08x ", shdr_tbl[i].sh_offset);
		ftrace_write("0x%08x ", shdr_tbl[i].sh_addr);
		ftrace_write("0x%08x ", shdr_tbl[i].sh_size);
		ftrace_write("%-4d ", shdr_tbl[i].sh_addralign);
		ftrace_write("0x%08x ", shdr_tbl[i].sh_flags);
		ftrace_write("0x%08x ", shdr_tbl[i].sh_type);
		ftrace_write("%s\t", (sh_str + shdr_tbl[i].sh_name));
		ftrace_write("\n");
	}
	ftrace_write("========================================");
	ftrace_write("========================================\n");
	ftrace_write("\n");	/* end of section header table */
}

static void read_symbol_table(int fd, Elf32_Ehdr ehdr, Elf32_Shdr shdr_tbl[], int symbol_idx) {
    Elf32_Sym sym_tbl[shdr_tbl[symbol_idx].sh_size / sizeof(Elf32_Sym)];
    read_section(fd, shdr_tbl[symbol_idx], sym_tbl);
    int str_tab_idx = shdr_tbl[symbol_idx].sh_link;
    char str_tab[shdr_tbl[str_tab_idx].sh_size]; // 所有symbol的name
    read_section(fd, shdr_tbl[str_tab_idx], str_tab);
    int sym_cnt = shdr_tbl[symbol_idx].sh_size / sizeof(Elf32_Sym);
    ftrace_write("Symbol count: %d\n", sym_cnt);
    ftrace_write("====================================================\n");
    ftrace_write(" num    value            type size       name\n");
    ftrace_write("====================================================\n");
    int func_cnt = 0;
    for (int i = 0; i < sym_cnt; i++) {
        ftrace_write(" %-3d    %016x %-4d %-10d %s\n",
        i,
        sym_tbl[i].st_value, 
        ELF32_ST_TYPE(sym_tbl[i].st_info),
                sym_tbl[i].st_size,
        str_tab + sym_tbl[i].st_name
        ); // st_name is the index of symbol in strtab
        if (ELF32_ST_TYPE(sym_tbl[i].st_info) == STT_FUNC) {
            func_cnt++;
        }
    }
    ftrace_write("====================================================\n");
    g_symbol_table = (SymEntry *)malloc(sizeof(SymEntry) * func_cnt);
    for(int i = 0; i < sym_cnt; i++) {
        if(ELF32_ST_TYPE(sym_tbl[i].st_info) == STT_FUNC) {
            g_symbol_table[g_symbol_tbl_size].addr = sym_tbl[i].st_value;
            g_symbol_table[g_symbol_tbl_size].size = sym_tbl[i].st_size;
            g_symbol_table[g_symbol_tbl_size].info = sym_tbl[i].st_info;
            strncpy(g_symbol_table[g_symbol_tbl_size].name, str_tab + sym_tbl[i].st_name, 31);
            g_symbol_tbl_size++;
        }
    }
}

void trace_init() {
    ftrace_log = fopen("/home/ashdr/code/ics2023/nemu/build/ftrace-log.txt", "w");
    assert(ftrace_log != NULL);
}

void parse_elf(const char *elf_file) {
    int fd = open(elf_file, O_RDONLY);
    Assert(fd != -1, "Failed to open file %s", elf_file);
    Elf32_Ehdr ehdr;
    read_elf_header(fd, &ehdr);
    display_elf_header(ehdr);
    Elf32_Shdr shdr_tbl[ehdr.e_shnum];
    read_section_header(fd, ehdr, shdr_tbl);
    display_section_header(fd, ehdr, shdr_tbl);
    for(int i = 0; i < ehdr.e_shnum; i++) {
        if(shdr_tbl[i].sh_type == SHT_SYMTAB) {
            read_symbol_table(fd, ehdr, shdr_tbl, i);
        }
    }
    close(fd);
}

static int get_func_idx(paddr_t target) {
    for(int i = 0; i < g_symbol_tbl_size; i++) {
        if(g_symbol_table[i].addr <= target && target < g_symbol_table[i].addr + g_symbol_table[i].size) {
            return i;
        }
    }
    return -1;
}

void trace_func_call(paddr_t pc, paddr_t target) {
    // assert(call_depth >= 0);
    Log("call call_depth: %d", call_depth);
    int funcid = get_func_idx(target);
    ftrace_write(FMT_PADDR ": %*scall[%s@" FMT_PADDR "]\n", pc, call_depth * 2, "", (funcid == -1) ? "???" : g_symbol_table[funcid].name, target);
    ++call_depth;
}

void trace_func_ret(paddr_t pc) {
    Log("ret call_depth: %d", call_depth);
    int funcid = get_func_idx(pc);
    ftrace_write(FMT_PADDR ": %*sret[%s]\n", pc, call_depth * 2, "", (funcid == -1) ? "???" : g_symbol_table[funcid].name);
    --call_depth;
}


// 不匹配的函数调用和返回是因为函数调用尾部优化
// 1.elf的section header 指明了字符串表在elf文件中的那个位置 2.通过偏移量找到字符串表的位置 3.字符串表将symbol table中的标识符的字符串存储了起来 4.根据symbol table中的Size属性找到字符串表中的字符串
// symbol table中还有symbol的虚拟地址、大小、type等信息。 使用地址通过symbol table找到字符串表中的字符串的size，再在字符串表中找到字符串