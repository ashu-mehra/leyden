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
#include "code/SCCache.hpp"
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

void StubCodeGenerator::setup_code_desc(const char* name, address start, address end, bool loaded_from_cache) {
  StubCodeDesc* cdesc = new StubCodeDesc("StubRoutines", name, start, end);
  cdesc->set_disp(uint(start - _masm->code_section()->outer()->insts_begin()));
  if (loaded_from_cache) {
    cdesc->set_loaded_from_cache();
  }
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

bool StubCodeGenerator::find_archive_data(int stubId) {
  if (_archive_data == nullptr) {
    return false;
  }
  return _archive_data->find_archive_data(stubId);
}

void StubCodeGenerator::load_archive_data(int stubId, const char* stub_name, address* start, address* end, address* entry_address1) {
#ifdef ASSERT
  assert(find_archive_data(stubId), "archive data does not exist");
#endif
  assert(_archive_data != nullptr, "archive data is not set");
  _archive_data->as_const()->load_archive_data(start, end, entry_address1);
  assert(*start != nullptr, "failed to load start address of the stub %d", stubId);
  assert(*end != nullptr, "failed to load end address of the stub %d", stubId);
  setup_code_desc(stub_name, *start, *end, true);
}

void StubCodeGenerator::load_archive_data(int stubId, const char* stub_name, address* start, address* end, GrowableArray<address> *entries) {
#ifdef ASSERT
  assert(find_archive_data(stubId), "archive data does not exist");
#endif
  assert(_archive_data != nullptr, "archive data is not set");
  _archive_data->as_const()->load_archive_data(start, end, entries);
  assert(*start != nullptr, "failed to load start address of the stub %d", stubId);
  assert(*end != nullptr, "failed to load end address of the stub %d", stubId);
  setup_code_desc(stub_name, *start, *end, true);
}

void StubCodeGenerator::setup_stub_archive_data(int stubId, address start, address end, address entry_address1, address entry_address2) {
  if (_archive_data == nullptr) {
    return;
  }
  _archive_data->store_archive_data(stubId, start, end, entry_address1, entry_address2);
  SCCache::add_stub_address(start);
  if (entry_address1 != nullptr) {
    SCCache::add_stub_address(entry_address1);
  }
  if (entry_address2 != nullptr) {
    SCCache::add_stub_address(entry_address2);
  }
}
 
void StubCodeGenerator::setup_stub_archive_data(int stubId, address start, address end, GrowableArray<address> *entries) {
  if (_archive_data == nullptr) {
    return;
  }
  _archive_data->store_archive_data(stubId, start, end, entries);
  SCCache::add_stub_address(start);
  if (entries != nullptr) {
    int len = entries->length();
    for (int i = 0; i < len; i++) {
      SCCache::add_stub_address(entries->at(i));
    }
  }
}
 
int StubCodeGenerator::num_stubs(StubsKind kind) {
  switch (kind) {
  case StubsKind::Initial_stubs:
    return StubRoutines::initial_stubs_cnt();
    break;
  case StubsKind::Continuation_stubs:
    return StubRoutines::continuation_stubs_cnt();
    break;
  case StubsKind::Compiler_stubs:
    return StubRoutines::compiler_stubs_cnt();
    break;
  case StubsKind::Final_stubs:
    return StubRoutines::final_stubs_cnt();
    break;
  default:
    ShouldNotReachHere();
  }
}

void StubCodeGenerator::print_statistics_on(outputStream* st) {
  st->print_cr("StubRoutines Stubs:");
  st->print_cr("  Initial stubs:         %d", StubRoutines::initial_stubs_cnt());
  st->print_cr("  Continuation stubs:    %d", StubRoutines::continuation_stubs_cnt());
  st->print_cr("  Compiler stubs:        %d", StubRoutines::compiler_stubs_cnt());
  st->print_cr("  Final stubs:           %d", StubRoutines::final_stubs_cnt());

  int emitted = 0;
  int loaded_from_cache = 0;

  StubCodeDesc* scd = StubCodeDesc::first();
  while (scd != nullptr) {
    if (!strcmp(scd->group(), "StubRoutines")) {
      emitted += 1;
      if (scd->loaded_from_cache()) {
	loaded_from_cache += 1;
      }
    }
    scd = StubCodeDesc::next(scd);
  }
  st->print_cr("Total stubroutines stubs emitted: %d (generated=%d, loaded from cache=%d)", emitted, emitted - loaded_from_cache, loaded_from_cache);
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

bool StubArchiveData::find_archive_data(int stubId) {
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

void StubArchiveData::load_archive_data(address* start, address* end, address* entry_address1) const {
  assert(start != nullptr, "start address cannot be null");
  assert(end != nullptr, "end address cannot be null");
  *start = current_stub_entry_addr(0);
  *end = current_stub_end_addr();
  if (entry_address1 != nullptr) {
    *entry_address1 = current_stub_entry_addr(1);
  }
}

void StubArchiveData::load_archive_data(address* start, address* end, GrowableArray<address>* entries) const {
  assert(start != nullptr, "start address cannot be null");
  assert(end != nullptr, "end address cannot be null");
  *start = current_stub_entry_addr(0);
  *end = current_stub_end_addr();
  if (entries != nullptr) {
    assert(entries->length() == 0, "should only pass empty array for returned entries");
    int count = _current->count();
    for (int i = 1; i < count - 1; i++) {
      entries->append(current_stub_entry_addr(i));
    }
  }
}

void StubArchiveData::store_archive_data(int stubId, address start, address end, address entry_address1, address entry_address2) {
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  assert(start != nullptr, "start address cannot be null");
  assert(end != nullptr, "end address cannot be null");
  int start_addr_index = _address_array.length();
  _address_array.append(start);
  if (entry_address1 != nullptr) {
    _address_array.append(entry_address1);
  }
  if (entry_address2 != nullptr) {
    assert(entry_address1 != nullptr, "entry_address1 cannot be null if entry_address2 is not null");
    _address_array.append(entry_address2);
  }
  _address_array.append(end);
  int count = _address_array.length() - start_addr_index;
  _index_table[index].init_entry(start_addr_index, count);
}

void StubArchiveData::store_archive_data(int stubId, address start, address end, GrowableArray<address>* entries) {
  int index = StubRoutines::stubId_to_index(stubId);
  assert(index >= 0 && index < _index_table_cnt, "invalid index %d for table count %d", index, _index_table_cnt);
  assert(start != nullptr, "start address cannot be null");
  assert(end != nullptr, "end address cannot be null");
  int start_addr_index = _address_array.length();
  _address_array.append(start);
  if (entries != nullptr) {
    int len = entries->length();
    for (int i = 0; i < len; i++) {
      _address_array.append(entries->at(i));
    }
  }
  _address_array.append(end);
  int count = _address_array.length() - start_addr_index;
  _index_table[index].init_entry(start_addr_index, count);
}
