#ifndef ELF_FN_H
#define ELF_FN_H

// NOTE: this file will be included at the very end of elf.h
#include <fs/fs.h>

typedef struct {
	Elf32_Sym* symtab;
	uint32_t symtabsz;
	const char* strtab;
	uint32_t strtabsz;
} Elf32_Sym_Map;

// returns elf symbol mapping from multiboot struct
Elf32_Sym_Map elf_sym_map_from_multiboot(multiboot_elf_section_header_table_t *mb);

// finds symbol by address
const char *elf_sym_map_lookup(uint32_t addr, Elf32_Sym_Map *elf);

// loads elf from fs_note
uint32_t load_elf_binary(fs_node_t* file);

#endif // ELF_FN_H
