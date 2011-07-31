#include <elf.h>

elf_t elf_from_multiboot(multiboot_info_t* mb) {
	int i;
	elf_t elf;
	multiboot_elf_section_header_table_t esh_tab = mb->u.elf_sec;
	elf_section_header_t* esh = (elf_section_header_t*) esh_tab.addr;

	uint32_t shstrtab = esh[esh_tab.shndx].addr;
	for (i = 0; i < mb->u.elf_sec.num; i++) {
		const char *name = (const char *) (shstrtab + esh[i].name);
		if (!strcmp((void*) name, ".strtab")) {
			elf.strtab = (const char *) esh[i].addr;
			elf.strtabsz = esh[i].size;
		}
		if (!strcmp((void*) name, ".symtab")) {
			elf.symtab = (elf_symbol_t*) esh[i].addr;
			elf.symtabsz = esh[i].size;
		}
	}
	return elf;
}

const char *elf_lookup_symbol(uint32_t addr, elf_t *elf) {
	int i;

	for (i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++) {
		if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2)
			continue;

		if ((addr >= elf->symtab[i].value)
				&& (addr < (elf->symtab[i].value + elf->symtab[i].size))) {
			const char *name = (const char *) ((uint32_t) elf->strtab
					+ elf->symtab[i].name);
			return name;
		}
	}
	return "";
}
