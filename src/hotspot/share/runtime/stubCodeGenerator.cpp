/*
 * Copyright (c) 1997, 2023, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "asm/macroAssembler.hpp"
#include "asm/macroAssembler.inline.hpp"
#include "code/codeCache.hpp"
#include "compiler/disassembler.hpp"
#include "oops/oop.inline.hpp"
#include "prims/forte.hpp"
#include "prims/jvmtiExport.hpp"
#include "runtime/stubCodeGenerator.hpp"
#include "runtime/stubRoutines.hpp"


// Implementation of StubCodeDesc

StubCodeDesc* StubCodeDesc::_list = nullptr;
bool          StubCodeDesc::_frozen = false;

StubCodeDesc* StubCodeDesc::desc_for(address pc) {
  StubCodeDesc* p = _list;
  while (p != nullptr && !p->contains(pc)) {
    p = p->_next;
  }
  return p;
}

void StubCodeDesc::freeze() {
  assert(!_frozen, "repeated freeze operation");
  _frozen = true;
}

void StubCodeDesc::unfreeze() {
  assert(_frozen, "repeated unfreeze operation");
  _frozen = false;
}

void StubCodeDesc::print_on(outputStream* st) const {
  st->print("%s", group());
  st->print("::");
  st->print("%s", name());
  st->print(" [" INTPTR_FORMAT ", " INTPTR_FORMAT "] (%d bytes)", p2i(begin()), p2i(end()), size_in_bytes());
}

void StubCodeDesc::print() const { print_on(tty); }

// Implementation of StubCodeGenerator

StubCodeGenerator::StubCodeGenerator(CodeBuffer* code, StubArchiveData* archive_data, bool print_code) {
  _masm = new MacroAssembler(code);
  _archive_data = archive_data;
  _print_code = PrintStubCode || print_code;
}

StubCodeGenerator::~StubCodeGenerator() {
#ifndef PRODUCT
  CodeBuffer* cbuf = _masm->code();
  CodeBlob*   blob = CodeCache::find_blob(cbuf->insts()->start());
  if (blob != nullptr) {
    blob->use_remarks(cbuf->asm_remarks());
    blob->use_strings(cbuf->dbg_strings());
  }
#endif
}

void StubCodeGenerator::setup_code_desc(const char* name, address start, address end) {
  StubCodeDesc* cdesc = new StubCodeDesc("StubRoutines", name, start, end);
  cdesc->set_disp(uint(start - _masm->code_section()->outer()->insts_begin()));
  print_stub_code_desc(cdesc);
  // copied from ~~StubCodeMark()
  Forte::register_stub(cdesc->name(), cdesc->begin(), cdesc->end());
  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated(cdesc->name(), cdesc->begin(), cdesc->end());
  }
}

void StubCodeGenerator::stub_prolog(StubCodeDesc* cdesc) {
  // default implementation - do nothing
}

void StubCodeGenerator::stub_epilog(StubCodeDesc* cdesc) {
  print_stub_code_desc(cdesc);
}

void StubCodeGenerator::print_stub_code_desc(StubCodeDesc* cdesc) {
  LogTarget(Debug, stubs) lt;
  if (lt.is_enabled()) {
    LogStream ls(lt);
    cdesc->print_on(&ls);
    ls.cr();
  }

  if (_print_code) {
#ifndef PRODUCT
    // Find the assembly code remarks in the outer CodeBuffer.
    AsmRemarks* remarks = &_masm->code_section()->outer()->asm_remarks();
#endif
    ttyLocker ttyl;
    tty->print_cr("- - - [BEGIN] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
    cdesc->print_on(tty);
    tty->cr();
    Disassembler::decode(cdesc->begin(), cdesc->end(), tty
                         NOT_PRODUCT(COMMA remarks COMMA cdesc->disp()));
    tty->print_cr("- - - [END] - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
    tty->cr();
  }
}

int StubCodeGenerator::num_stubs(StubsKind kind) {
  switch (kind) {
  case StubsKind::Initial_stubs:
    return StubRoutines::StubID::last_initial_stub + 1;
    break;
  case StubsKind::Continuation_stubs:
    return StubRoutines::StubID::last_continuation_stub - StubRoutines::StubID::last_initial_stub;
    break;
  case StubsKind::Compiler_stubs:
    return StubRoutines::StubID::last_compiler_stub - StubRoutines::StubID::last_continuation_stub;
    break;
  case StubsKind::Final_stubs:
    return StubRoutines::StubID::last_final_stub - StubRoutines::StubID::last_compiler_stub;
    break;
  default:
    ShouldNotReachHere();
  }
}

// Implementation of CodeMark

StubCodeMark::StubCodeMark(StubCodeGenerator* cgen, const char* group, const char* name) {
  _cgen  = cgen;
  _cdesc = new StubCodeDesc(group, name, _cgen->assembler()->pc());
  _cgen->stub_prolog(_cdesc);
  // define the stub's beginning (= entry point) to be after the prolog:
  _cdesc->set_begin(_cgen->assembler()->pc());
}

StubCodeMark::~StubCodeMark() {
  if (!_cdesc->loaded_from_cache()) {
    _cgen->assembler()->flush();
    _cdesc->set_end(_cgen->assembler()->pc());
    assert(StubCodeDesc::_list == _cdesc, "expected order on list");
  #ifndef PRODUCT
    address base = _cgen->assembler()->code_section()->outer()->insts_begin();
    address head = _cdesc->begin();
    _cdesc->set_disp(uint(head - base));
  #endif
  }
  _cgen->stub_epilog(_cdesc);
  Forte::register_stub(_cdesc->name(), _cdesc->begin(), _cdesc->end());

  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated(_cdesc->name(), _cdesc->begin(), _cdesc->end());
  }
}

bool StubArchiveData::load_archive_data_for(int stubId) {
  assert(_index_table != nullptr, "sanity check");
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  StubAddrIndexInfo index_info = _index_table[index];
  int start_addr_index = index_info.start_index();
  if (start_addr_index == -1) {
    return false;
  }
  _current = &_index_table[index];
  return true;
}

void StubArchiveData::store_archive_data(int stubId, address start, address end) {
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  int start_addr_index = _address_array.length();
  _address_array.append(start);
  _address_array.append(end);
  _index_table[index].init_entry(start_addr_index, 2);
}

void StubArchiveData::store_archive_data(int stubId, address start, address entry_address_1, address end) {
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  int start_addr_index = _address_array.length();
  _address_array.append(start);
  _address_array.append(entry_address_1);
  _address_array.append(end);
  _index_table[index].init_entry(start_addr_index, 3);
}

void StubArchiveData::store_archive_data(int stubId, address start, address entry_address_1, address entry_address_2, address end) {
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  int start_addr_index = _address_array.length();
  _address_array.append(start);
  _address_array.append(entry_address_1);
  _address_array.append(entry_address_2);
  _address_array.append(end);
  _index_table[index].init_entry(start_addr_index, 4);
}
