#include <kernel/elf.h>

// NOTE: these functions are declared in elf-fn.h

Elf32_Sym_Map elf_sym_map_from_multiboot(
		multiboot_elf_section_header_table_t* elf_sec) {
	Elf32_Sym_Map elf;
	Elf32_Shdr* sh = (Elf32_Shdr*) elf_sec->addr;

	uint32_t shstrtab = sh[elf_sec->shndx].sh_addr;
	for (uint32_t i = 0; i < elf_sec->num; i++) {
		const char *name = (const char *) (shstrtab + sh[i].sh_name);
		if (!strcmp(name, ".strtab")) {
			elf.strtab = (const char *) sh[i].sh_addr;
			elf.strtabsz = sh[i].sh_size;
		}
		if (!strcmp(name, ".symtab")) {
			elf.symtab = (Elf32_Sym*) sh[i].sh_addr;
			elf.symtabsz = sh[i].sh_size;
		}
	}
	return elf;
}

const char* elf_sym_map_lookup(uint32_t addr, Elf32_Sym_Map *elf_sym) {
	for (uint32_t i = 0; i < (elf_sym->symtabsz / sizeof(Elf32_Sym)); i++) {
		if (ELF32_ST_TYPE(elf_sym->symtab[i].st_info) != 0x2)
			continue;

		if ((addr >= elf_sym->symtab[i].st_value)
				&& (addr
						< (elf_sym->symtab[i].st_value
								+ elf_sym->symtab[i].st_size))) {
			const char *name = (const char *) ((uint32_t) elf_sym->strtab
					+ elf_sym->symtab[i].st_name);
			return name;
		}
	}
	return "";
}
