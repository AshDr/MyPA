#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_LOONGARCH32R__)
# define EXPECT_TYPE EM_LOONGARCH
#else
# error Unsupported ISA __ISA__
#endif

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern int fs_lseek(int fd, size_t offset, int whence);


static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  if(fd == -1) {
    panic("Failed to open file %s", filename);
  }

  uint32_t phdr_size = sizeof(Elf_Phdr);
  Elf_Ehdr *ehdr = malloc(sizeof(Elf_Ehdr));
  Elf_Phdr *phdr = malloc(phdr_size);
  fs_read(fd, ehdr, sizeof(Elf_Ehdr));
  // ramdisk_read((void*)ehdr, 0, sizeof(Elf_Ehdr));
  // printf("ident2: %u\n", ehdr->e_ident);
  assert(*(uint32_t *)ehdr->e_ident == 0x464c457f);
  assert(EXPECT_TYPE == ehdr->e_machine);
  uint32_t phdr_num = ehdr->e_phnum;
  for(int i = 0; i < phdr_num; i++) {
    fs_lseek(fd, ehdr->e_phoff + i * phdr_size, 0);
    fs_read(fd, phdr, phdr_size);
    if(phdr->p_type == PT_LOAD) {
      fs_lseek(fd, phdr->p_offset, 0);
      fs_read(fd, (void *)phdr->p_vaddr, phdr->p_filesz);
      if(phdr->p_filesz < phdr->p_memsz) {
        memset((void *)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
      }
    }
  }
  assert(fs_close(fd) == 0);
  return ehdr->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

