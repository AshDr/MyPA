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


static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  printf("ident: %d\n", ehdr.e_ident);
  assert(((uint32_t)ehdr.e_ident == 0x464c457f));
  assert(EXPECT_TYPE == ehdr.e_machine);
  uint32_t phnum = ehdr.e_phnum;
  Elf_Phdr phdr;
  for(int i = 0; i < phnum; i++) {
    ramdisk_read(&phdr, ehdr.e_phoff + i * ehdr.e_phentsize, sizeof(Elf_Phdr));
    if(phdr.p_type == PT_LOAD) {
      ramdisk_read((void *)phdr.p_vaddr, phdr.p_offset, phdr.p_filesz);
      if(phdr.p_filesz < phdr.p_memsz) {
        memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
      }
    }
  }
  return ehdr.e_entry;
  // uint32_t phdr_size = sizeof(Elf_Phdr);
  // Elf_Ehdr *ehdr = malloc(sizeof(Elf_Ehdr));
  // Elf_Phdr *phdr = malloc(phdr_size);
  // ramdisk_read((void*)ehdr, 0, sizeof(Elf_Ehdr));
  // assert(*(uint32_t *)ehdr->e_ident == 0x464c457f);
  // printf("ident: %x\n", ehdr->e_ident);
  // assert(EXPECT_TYPE == ehdr->e_machine);
  // uint32_t phdr_num = ehdr->e_phnum;
  // for(int i = 0; i < phdr_num; i++) {
  //   ramdisk_read((void*)phdr, ehdr->e_phoff + i * phdr_size, phdr_size);
  //   if(phdr->p_type != PT_LOAD) continue;
  //   ramdisk_read((void*)phdr->p_vaddr, phdr->p_offset, phdr->p_filesz);
  //   memset((void*)phdr->p_vaddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
  // }
  // return ehdr->e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

