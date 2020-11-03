//
// Created by Sascha Roth on 11.11.19.
//

#include <link.h>
#include "Elf64Parser.h"
#include "IElfParser.h"
#include "../tools/LoggingCallback.h"

namespace ElfParser {


    bool Elf64Parser::iterateShdr(const std::function<bool(Elf64_Shdr *shdr)> &callback) {
        log("generateSymbolHooks [-] parsing headers...");
        auto *header = (Elf64_Ehdr *) &addr_[0];

        log("generateSymbolHooks [-] search for symbols");
        for (int k = 0; k < header->e_shnum; k++) {
            auto shdr = &((Elf64_Shdr *) &addr_[header->e_shoff])[k];

            if (!callback(shdr)) {
                return false;
            }
        }
        return true;
    }

    bool Elf64Parser::iterateSymbolTable(const std::vector<int> &types,
                                         const std::function<bool(int, char *, uint64_t,
                                                                  uint8_t)> function) {

        char *dynStrTab = nullptr;
        auto *dynstr = getSectionByName<Elf64_Shdr>(".dynstr");
        if (dynstr == nullptr) {
            log("!! warning !! Could not find dynstr section!");
        } else {
            dynStrTab = (char *) (&addr_[dynstr->sh_offset]);
        }

        char *strTab = nullptr;
        auto *strtab = getSectionByName<Elf64_Shdr>(".strtab");
        if (strtab == nullptr) {
            log("!! warning !! Could not find strtab section!");
        } else {
            strTab = (char *) (&addr_[strtab->sh_offset]);
        }

        return iterateShdr([this, &function, &types, &strTab, &dynStrTab](Elf64_Shdr *shdr) {
            char *symbolName = nullptr;
            for (int type : types) {
                if (shdr->sh_type == type) { // shdr->sh_type == SHT_DYNSYM ||
                    char *tab = nullptr;
                    if (type == SHT_SYMTAB) {
                        tab = strTab;
                    } else if (type == SHT_DYNSYM) {
                        tab = dynStrTab;
                    }
                    if (tab == nullptr) {
                        return true;// can happen, this is no error!
                    }
                    for (size_t entry = 0;
                         (entry + 1) * sizeof(Elf64_Sym) <= shdr->sh_size; entry++) {
                        Elf64_Sym *sym = (Elf64_Sym *) (&addr_[shdr->sh_offset] +
                                                        entry * sizeof(Elf64_Sym));
                        symbolName = tab + sym->st_name;
                        if (tab != nullptr &&
                            sym->st_name != 0 &&
                            sym->st_value != 0 &&
                            sym->st_info ==
                            0x12) { // only symbols that are implemented in our binary
                            //log("Found symbol: <%s> st_value<%lx> st_info<%lx> st_other<%lx>",
                            //    symbolName, sym->st_value, sym->st_info, sym->st_other);

                            if (!function(shdr->sh_type, symbolName,
                                          SymAddrToFileOffset(sym->st_value), sym->st_info)) {
                                return false;
                            }
                        }
                    }
                }
            }
            return true;
        });
    }

    char* Elf64Parser::GetSymbolNameByIndex(unsigned index) {
        char *SymbolName = nullptr;
        std::vector<int> types;
        types.clear();
        types.push_back(SHT_SYMTAB);
        types.push_back(SHT_DYNSYM);

        if (!iterateSymbolTable(types,
                                [&index, &SymbolName](int type,
                                                      char *symbolName,
                                                      uint64_t st_value,
                                                      uint8_t st_info) {
                                    if (index <= 0) {
                                        SymbolName = symbolName;
                                    } else {
                                        index -= 1;
                                    }
                                    return true;
                                })) {
            log("MemLinker [-] Error while iterate symbols, abort...");

        }
        return SymbolName;
    }

    bool Elf64Parser::iterateNeeded(const std::function<bool(char *)> function) {

        auto *dynamic = getSectionByName<Elf64_Shdr>(std::string(".dynamic"));
        if (dynamic == nullptr) {
            log("Could not find dynamic section!");
            return false;
        }
        auto *dynstr = getSectionByName<Elf64_Shdr>(".dynstr");
        if (dynstr == nullptr) {
            log("Could not find dynstr section!");
            return false;
        }
        char *strTab = (char *) (&addr_[dynstr->sh_offset]);

        /// strtab2
        char *rpath = nullptr;
        log("Searching for libraries in dynamic section");

        auto *dyn = (Elf64_Dyn *) (&addr_[(dynamic->sh_offset)]);
        for (; dyn->d_tag != DT_NULL; dyn++) {
            if (dyn->d_tag == DT_NEEDED) {
                if (dyn->d_un.d_val != 0) {
                    rpath = strTab + dyn->d_un.d_val;
                    if (rpath != nullptr) {
                        if (!function(rpath)) {
                            return false;
                        }
                    }

                }
            }
        }
        return true;
    }

    Elf64Parser::Elf64Parser(const char *addr) : IElfParser(addr) {}

    template<class K>
    K *Elf64Parser::getSectionByName(const std::string &name) {
        auto *header = (Elf64_Ehdr *) &addr_[0];


        //shdrs[shstrtabIndex].sh_offset
        for (int k = 0; k < header->e_shnum; k++) {
            auto shdr = &((Elf64_Shdr *) (&addr_[header->e_shoff]))[k];
            uint64_t off = (((Elf64_Shdr *) &addr_[header->e_shoff]))[header->e_shstrndx].sh_offset;
            char *data = (char *) (&addr_[off]);
            std::string sectionName(data + shdr->sh_name);
            //log("Found section <%s> off:<%lx> info<%lx> addr<%lx>", sectionName.c_str(),
            //   shdr->sh_offset, shdr->sh_info, shdr->sh_addr);
            //log("shdr %d %p %d %s", off, data, shdr->sh_name, sectionName.c_str());
            if (sectionName == name) {
                //Elf64_Dyn *dynamic = reinterpret_cast<Elf64_Dyn *>(&_data[shdr.sh_offset]);

                return (K *) shdr;
            }
        }
        return nullptr;
    }

    uint64_t Elf64Parser::SymAddrToFileOffset(uint64_t va) {
        uint64_t offset = 0;
        auto *header = (Elf64_Ehdr *) &addr_[0];
        auto *seg = (Elf64_Phdr *) (addr_ + header->e_phoff);
        for (int i = 0; i < header->e_phnum; i++) {
            if (seg[i].p_type != PT_LOAD)
                continue;

            if (va >= seg[i].p_vaddr && va < seg[i].p_vaddr + seg[i].p_memsz) {
                offset = seg[i].p_offset + (va - seg[i].p_vaddr);
            }
        }
        return offset;
    }

    bool Elf64Parser::IterateRelocationTable(const std::vector<int> &types,
                                             const std::function<bool(int type, char *symbolName,
                                                                      uint64_t st_value,
                                                                      uint8_t st_info)> function) {
        log("Search for relocations");

        auto *header = (Elf64_Ehdr *) &addr_[0];


        // rela relocations
        char *relaTab = nullptr;
        auto *rela = getSectionByName<Elf64_Shdr>(".rela.dyn");
        if (rela == nullptr) {
//            return false;
        } else {
            relaTab = (char *) (&addr_[rela->sh_offset]);
        }
        if (relaTab != nullptr) {

            unsigned int i, idx;
            for (idx = 0; idx < rela->sh_size / rela->sh_entsize; idx++) {
                Elf64_Rela *reltab = &((Elf64_Rela *) (addr_ +
                                                       rela->sh_offset))[idx];
                function(0, "rela", reltab->r_offset, 0);
//                        int result = elf_do_reloc(hdr, reltab, section);
//                        // On error, display a message and return
//                        if (result == ELF_RELOC_ERR) {
//                            ERROR("Failed to relocate symbol.\n");
//                            return ELF_RELOC_ERR;
//                        }
                elf_do_reloc(header, reltab, rela);

            }
        }


        return false;
    }

    enum RelocationKind {
        kRelocAbsolute = 0,
        kRelocRelative,
        kRelocCopy,
        kRelocSymbol,
        kRelocMax
    };

    static void count_relocation(RelocationKind kind) {

    }

#define ELF_RELOC_ERR -1

    int Elf64Parser::elf_do_reloc(Elf64_Ehdr *hdr, Elf64_Rela *rela, Elf64_Shdr *reltab) {
//        Elf32_Shdr *target = elf_section(hdr, reltab->sh_info);
        Elf64_Shdr *target = reltab;
        Elf64_Addr sym_addr = 0;

        ELF64_R_TYPE (rela->r_info);
        unsigned symbol = ELF64_R_SYM(rela->r_info);

        char *addr = (char *) (hdr + rela->r_offset);
//        char **ref = (char **) (addr + rela->r_offset);
        // Symbol value
//        int symval = 0;
//        if (ELF64_R_SYM(rela->r_info) != SHN_UNDEF) {
//            symval = elf_get_symval(hdr, reltab->sh_link, ELF64_R_SYM(rela->r_info));
//            if (symval == ELF_RELOC_ERR) return ELF_RELOC_ERR;
//        }

//        unsigned sym = ELFW(R_SYM)(rela->r_info);

//        ElfW(Addr) reloc = static_cast<ElfW(Addr)>(rel->r_offset + si->load_bias);
//        ElfW(Sym) *s;
        // get symbol
//        s = soinfo_do_lookup(si, sym_name, &lsi, needed);
        // support undefined symbols?
//        sym_addr = static_cast<Elf64_Addr>(s->st_value + lsi->load_bias);

        // Relocate based on type
        log("Relocation Type (%d).\n", ELF64_R_TYPE(rela->r_info));
        switch (ELF64_R_TYPE(rela->r_info)) {
            case R_AARCH64_JUMP_SLOT:
                count_relocation(kRelocAbsolute);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO JMP_SLOT %16lx <- %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                *reinterpret_cast<Elf64_Addr *>(addr) = (sym_addr + rela->r_addend);
                break;
            case R_AARCH64_GLOB_DAT:
                count_relocation(kRelocAbsolute);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO GLOB_DAT %16lx <- %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                *reinterpret_cast<Elf64_Addr *>(addr) = (sym_addr + rela->r_addend);
                break;
            case R_AARCH64_ABS64:
                count_relocation(kRelocAbsolute);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO ABS64 %16lx <- %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                *reinterpret_cast<Elf64_Addr *>(addr) += (sym_addr + rela->r_addend);
                break;
            case R_AARCH64_ABS32:
                count_relocation(kRelocAbsolute);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO ABS32 %16lx <- %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                if ((static_cast<Elf64_Addr>(INT32_MIN) <=
                     (*reinterpret_cast<Elf64_Addr *>(addr) + (sym_addr + rela->r_addend))) &&
                    ((*reinterpret_cast<Elf64_Addr *>(addr) + (sym_addr + rela->r_addend)) <=
                     static_cast<Elf64_Addr>(UINT32_MAX))) {
                    *reinterpret_cast<Elf64_Addr *>(addr) += (sym_addr + rela->r_addend);
                } else {
//                    DL_ERR("0x%016lx out of range 0x%016lx to 0x%016lx",
//                           (*reinterpret_cast<Elf64_Addr *>(addr) + (sym_addr + rela->r_addend)),
//                           static_cast<Elf64_Addr>(INT32_MIN),
//                           static_cast<Elf64_Addr>(UINT32_MAX));
                    return -1;
                }
                break;
            case R_AARCH64_ABS16:
                count_relocation(kRelocAbsolute);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO ABS16 %16lx <- %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                if ((static_cast<Elf64_Addr>(INT16_MIN) <=
                     (*reinterpret_cast<Elf64_Addr *>(addr) + (sym_addr + rela->r_addend))) &&
                    ((*reinterpret_cast<Elf64_Addr *>(addr) + (sym_addr + rela->r_addend)) <=
                     static_cast<Elf64_Addr>(UINT16_MAX))) {
                    *reinterpret_cast<Elf64_Addr *>(addr) += (sym_addr + rela->r_addend);
                } else {
//                    DL_ERR("0x%016lx out of range 0x%016lx to 0x%016lx",
//                           (*reinterpret_cast<Elf64_Addr*>(addr) + (sym_addr + rela->r_addend)),
//                           static_cast<Elf64_Addr>(INT16_MIN),
//                           static_cast<Elf64_Addr>(UINT16_MAX));
                    return -1;
                }
                break;
            case R_AARCH64_PREL64:
                count_relocation(kRelocRelative);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO REL64 %16lx <- %16lx - %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           rela->r_offset,
//                           sym_name);
                *reinterpret_cast<Elf64_Addr *>(addr) +=
                        (sym_addr + rela->r_addend) - rela->r_offset;
                break;
            case R_AARCH64_PREL32:
                count_relocation(kRelocRelative);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO REL32 %16lx <- %16lx - %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           rela->r_offset, sym_name);
                if ((static_cast<Elf64_Addr>(INT32_MIN) <=
                     (*reinterpret_cast<Elf64_Addr *>(addr) +
                      ((sym_addr + rela->r_addend) - rela->r_offset))) &&
                    ((*reinterpret_cast<Elf64_Addr *>(addr) +
                      ((sym_addr + rela->r_addend) - rela->r_offset)) <=
                     static_cast<Elf64_Addr>(UINT32_MAX))) {
                    *reinterpret_cast<Elf64_Addr *>(addr) += ((sym_addr + rela->r_addend) -
                                                              rela->r_offset);
                } else {
//                    DL_ERR("0x%016lx out of range 0x%016lx to 0x%016lx",
//                           (*reinterpret_cast<Elf64_Addr*>(addr) + ((sym_addr + rela->r_addend) - rela->r_offset)),
//                           static_cast<Elf64_Addr>(INT32_MIN),
//                           static_cast<Elf64_Addr>(UINT32_MAX));
                    return -1;
                }
                break;
            case R_AARCH64_PREL16:
                count_relocation(kRelocRelative);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO REL16 %16lx <- %16lx - %16lx %s\n",
//                           reloc,
//                           (sym_addr + rela->r_addend),
//                           rela->r_offset, sym_name);
                if ((static_cast<Elf64_Addr>(INT16_MIN) <=
                     (*reinterpret_cast<Elf64_Addr *>(addr) +
                      ((sym_addr + rela->r_addend) - rela->r_offset))) &&
                    ((*reinterpret_cast<Elf64_Addr *>(addr) +
                      ((sym_addr + rela->r_addend) - rela->r_offset)) <=
                     static_cast<Elf64_Addr>(UINT16_MAX))) {
                    *reinterpret_cast<Elf64_Addr *>(addr) += ((sym_addr + rela->r_addend) -
                                                              rela->r_offset);
                } else {
//                    DL_ERR("0x%016lx out of range 0x%016lx to 0x%016lx",
//                           (*reinterpret_cast<Elf64_Addr*>(addr) + ((sym_addr + rela->r_addend) - rela->r_offset)),
//                           static_cast<Elf64_Addr>(INT16_MIN),
//                           static_cast<Elf64_Addr>(UINT16_MAX));
                    return -1;
                }
                break;
            case R_AARCH64_RELATIVE:
                count_relocation(kRelocRelative);
//                MARK(rela->r_offset);
//                if (sym) {
//                    DL_ERR("odd RELATIVE form...");
//                    return -1;
//                }
//                TRACE_TYPE(RELO, "RELO RELATIVE %16lx <- %16lx\n",
//                           reloc,
//                           (si->base + rela->r_addend));
                log("R_AARCH64_RELATIVE addr <%p>", addr);
                *((Elf64_Addr *)(addr) ) = *((Elf64_Addr *)((char *) addr_ + rela->r_addend));
                break;
            case R_AARCH64_COPY:
                log("R_AARCH64_COPY is not supported!");
                /*if ((si->flags & FLAG_EXE) == 0) {
                    *//*
                      * http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044d/IHI0044D_aaelf.pdf
                      *
                      * Section 4.7.1.10 "Dynamic relocations"
                      * R_AARCH64_COPY may only appear in executable objects where e_type is
                      * set to ET_EXEC.
                      *
                      * FLAG_EXE is set for both ET_DYN and ET_EXEC executables.
                      * We should explicitly disallow ET_DYN executables from having
                      * R_AARCH64_COPY relocations.
                      *//*
                    DL_ERR("%s R_AARCH64_COPY relocations only supported for ET_EXEC", si->name);
                    return -1;
                }
                count_relocation(kRelocCopy);
//                MARK(rela->r_offset);
//                TRACE_TYPE(RELO, "RELO COPY %16lx <- %ld @ %16lx %s\n",
//                           reloc,
//                           s->st_size,
//                           (sym_addr + rela->r_addend),
//                           sym_name);
                if (reloc == (sym_addr + rela->r_addend)) {
                    Elf64_Sym *src = soinfo_do_lookup(NULL, sym_name, &lsi, needed);
                    if (src == NULL) {
                        DL_ERR("%s R_AARCH64_COPY relocation source cannot be resolved", si->name);
                        return -1;
                    }
                    if (lsi->has_DT_SYMBOLIC) {
                        DL_ERR("%s invalid R_AARCH64_COPY relocation against DT_SYMBOLIC shared "
                               "library %s (built with -Bsymbolic?)", si->name, lsi->name);
                        return -1;
                    }
                    if (s->st_size < src->st_size) {
                        DL_ERR("%s R_AARCH64_COPY relocation size mismatch (%ld < %ld)",
                               si->name, s->st_size, src->st_size);
                        return -1;
                    }
                    memcpy((void *) addr, (void *) (src->st_value + lsi->load_bias), src->st_size);
                } else {
                    DL_ERR("%s R_AARCH64_COPY relocation target cannot be resolved", si->name);
                    return -1;
                }*/
                break;


                // https://android.googlesource.com/platform/bionic/+/865119e/linker/linker.cpp
            default:
                // Relocation type not supported, display error and return
                log("Unsupported Relocation Type (%d).\n", ELF64_R_TYPE(rela->r_info));
                return ELF_RELOC_ERR;
        }
        return 1;
    }


}
