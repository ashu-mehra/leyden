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

#ifndef SHARE_RUNTIME_STUBCODEGENERATOR_HPP
#define SHARE_RUNTIME_STUBCODEGENERATOR_HPP

#include "asm/assembler.hpp"
#include "memory/allocation.hpp"

// All the basic framework for stub code generation/debugging/printing.


// A StubCodeDesc describes a piece of generated code (usually stubs).
// This information is mainly useful for debugging and printing.
// Currently, code descriptors are simply chained in a linked list,
// this may have to change if searching becomes too slow.

class StubCodeDesc: public CHeapObj<mtCode> {
 private:
  static StubCodeDesc* _list;     // the list of all descriptors
  static bool          _frozen;   // determines whether _list modifications are allowed

  StubCodeDesc*        _next;     // the next element in the linked list
  const char*          _group;    // the group to which the stub code belongs
  const char*          _name;     // the name assigned to the stub code
  address              _begin;    // points to the first byte of the stub code    (included)
  address              _end;      // points to the first byte after the stub code (excluded)
  uint                 _disp;     // Displacement relative base address in buffer.
  bool                 _loaded_from_cache;

  friend class StubCodeMark;
  friend class StubCodeGenerator;

public:
  void set_begin(address begin) {
    assert(begin >= _begin, "begin may not decrease");
    assert(_end == nullptr || begin <= _end, "begin & end not properly ordered");
    _begin = begin;
  }

  void set_end(address end) {
    assert(_begin <= end, "begin & end not properly ordered");
    _end = end;
  }

  void set_disp(uint disp) { _disp = disp; }

  void set_loaded_from_cache() { _loaded_from_cache = true; }

 public:
  static StubCodeDesc* first() { return _list; }
  static StubCodeDesc* next(StubCodeDesc* desc)  { return desc->_next; }

  static StubCodeDesc* desc_for(address pc);     // returns the code descriptor for the code containing pc or null

  StubCodeDesc(const char* group, const char* name, address begin, address end = nullptr) {
    assert(!_frozen, "no modifications allowed");
    assert(name != nullptr, "no name specified");
    _next           = _list;
    _group          = group;
    _name           = name;
    _begin          = begin;
    _end            = end;
    _disp           = 0;
    _list           = this;
    _loaded_from_cache = false;
  };

  static void freeze();
  static void unfreeze();

  const char* group() const                      { return _group; }
  const char* name() const                       { return _name; }
  address     begin() const                      { return _begin; }
  address     end() const                        { return _end; }
  uint        disp() const                       { return _disp; }
  int         size_in_bytes() const              { return pointer_delta_as_int(_end, _begin); }
  bool        contains(address pc) const         { return _begin <= pc && pc < _end; }
  bool        loaded_from_cache() const          { return _loaded_from_cache; }
  void        print_on(outputStream* st) const;
  void        print() const;
};

// forward declaration
class StubArchiveData;

// The base class for all stub-generating code generators.
// Provides utility functions.

class StubCodeGenerator: public StackObj {
 private:
  bool _print_code;

 protected:
  MacroAssembler*  _masm;
  StubArchiveData* _archive_data;

  void setup_code_desc(const char* name, address start, address end, bool loaded_from_cache);

 public:
  StubCodeGenerator(CodeBuffer* code, bool print_code) : StubCodeGenerator(code, nullptr, print_code) {}
  StubCodeGenerator(CodeBuffer* code, StubArchiveData* archive_data = nullptr, bool print_code = false);
  ~StubCodeGenerator();

  MacroAssembler* assembler() const              { return _masm; }

  virtual void stub_prolog(StubCodeDesc* cdesc); // called by StubCodeMark constructor
  virtual void stub_epilog(StubCodeDesc* cdesc); // called by StubCodeMark destructor

  void print_stub_code_desc(StubCodeDesc* cdesc);

  enum StubsKind {
    Initial_stubs,       // Stubs used by Runtime, Interpreter and compiled code.
                         // Have to be generated very early during VM startup.

    Continuation_stubs,  // Stubs used by virtual threads.
                         // Generated after GC barriers initialization but before
                         // Interpreter initialization.

    Compiler_stubs,      // Intrinsics and other stubs used only by compiled code.
                         // Can be generated by compiler (C2/JVMCI) thread based on
                         // DelayCompilerStubsGeneration flag.

    Final_stubs          // The rest of stubs. Generated at the end of VM init.
  };

  static int num_stubs(StubsKind kind);
  static void print_statistics_on(outputStream* st);
  bool find_archive_data(int stubId);
  void load_archive_data(int stubId, const char* stub_name, address* start, address* end, address* entry_address1 = nullptr);
  void load_archive_data(int stubId, const char* stub_name, address* start, address* end, GrowableArray<address>* entries);
  void setup_stub_archive_data(int stubId, address start, address end, address entry_address1 = nullptr, address entry_address2 = nullptr);
  void setup_stub_archive_data(int stubId, address start, address end, GrowableArray<address>* entries);
};

// Used to locate addresses owned by a stub in the _address_array.
class StubAddrIndexInfo {
private:
  // Index of the "start" address in the "address array"
  // Start address is the first address owned by this stub in the address array
  int _start_index;
  // Total number of addresses owned by this stub in the address array
  uint _naddr;

 public:
  StubAddrIndexInfo() : _start_index(-1), _naddr(0) {}
  int start_index() { return _start_index; }
  int count() { return _naddr; }
  // end address is the last address owned by this stub in the address array
  int end_index() { return _start_index + _naddr - 1; }

  void default_init() {
    _start_index = -1;
    _naddr = 0;
  }

  void init_entry(int start_index, int naddr) {
    _start_index = start_index;
    _naddr = naddr;
  }
};

class StubArchiveData : public StackObj {
private:
  // Array of addresses owned by stubs. Each stub adds address to this array.
  // First address added by the stub is the "start" address of the stub.
  // Last address is the "end" address of the stub.
  // Any other entry points of the stub are in between.
  GrowableArray<address> _address_array;
  // Used to locate addresses owned by a stub in the _address_array.
  StubAddrIndexInfo* _index_table;
  // Number of entries in the index_table
  int _index_table_cnt;
  // Pointer to the StubAddrIndexInfo for the stub being loaded
  StubAddrIndexInfo* _current;

public:
  StubArchiveData(StubCodeGenerator::StubsKind kind) : _current(nullptr) {
    _index_table_cnt = StubCodeGenerator::num_stubs(kind);
    _index_table = NEW_C_HEAP_ARRAY(StubAddrIndexInfo, _index_table_cnt, mtCode);
    for (int i = 0; i < _index_table_cnt; i++) {
      _index_table[i].default_init();
    }
  }

  ~StubArchiveData() {
    FREE_C_HEAP_ARRAY(StubAddrIndexInfo, _index_table);
  }

  GrowableArray<address>* stubs_address_array() { return &_address_array; }
  int index_table_count() const { return _index_table_cnt; }
  StubAddrIndexInfo* index_table() { return _index_table; }

  address current_stub_entry_addr(int index) const {
    assert(index < _current->count()-1, "index %d should be less than %d for entry address", index, _current->count()-1);
    return _address_array.at(_current->start_index() + index);
  }

  address current_stub_end_addr() const {
    return _address_array.at(_current->end_index());
  }

  bool find_archive_data(int stubId);
  void load_archive_data(address* start, address* end, address* entry_address1) const;
  void load_archive_data(address* start, address* end, GrowableArray<address>* entries) const;
  void store_archive_data(int stubId, address start, address end, address entry1 = nullptr, address entry2 = nullptr);

  void store_archive_data(int stubId, address start, address end, GrowableArray<address>* entries);

  const StubArchiveData* as_const() { return (const StubArchiveData*)this; }
};

// Stack-allocated helper class used to associate a stub code with a name.
// All stub code generating functions that use a StubCodeMark will be registered
// in the global StubCodeDesc list and the generated stub code can be identified
// later via an address pointing into it.

class StubCodeMark: public StackObj {
 private:
  StubCodeGenerator* _cgen;
  StubCodeDesc*      _cdesc;

 public:
  StubCodeMark(StubCodeGenerator* cgen, const char* group, const char* name);
  ~StubCodeMark();

  StubCodeDesc* stub_code_desc() { return _cdesc; }
};

#endif // SHARE_RUNTIME_STUBCODEGENERATOR_HPP
