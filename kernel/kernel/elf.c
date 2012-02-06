#include <kernel/elf.h>
#include <mm/vmm.h>
#include <string.h>

// NOTE: these functions are declared in elf-fn.h

uint32_t load_elf_binary(fs_node_t* file) {
	// read header
	Elf32_Ehdr elf_hdr;
	read(file, 0, sizeof(Elf32_Ehdr), (void*) &elf_hdr);

	// do some checkings
	if (memcmp(elf_hdr.e_ident, ELFMAG, SELFMAG) != 0)
		panic("ELF: This is not an elf file.");

	Elf32_Phdr p_hdr;
	for (uint32_t i = 0, off = elf_hdr.e_phoff; i < elf_hdr.e_phnum;
			i++, off += sizeof(Elf32_Phdr)) {
		read(file, off, sizeof(Elf32_Phdr), (void*) &p_hdr);
		if (PF_X == p_hdr.p_type) {
			allocate_range(p_hdr.p_vaddr, p_hdr.p_vaddr + p_hdr.p_memsz,
					PAGE_WRITE | PAGE_USER);
			read(file, p_hdr.p_offset, p_hdr.p_filesz, (void*) p_hdr.p_vaddr);
		}
	}

	return elf_hdr.e_entry;
}

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
