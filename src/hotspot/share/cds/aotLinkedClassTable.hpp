/*
 * Copyright (c) 2024, 2025, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_CDS_AOTLINKEDCLASSTABLE_HPP
#define SHARE_CDS_AOTLINKEDCLASSTABLE_HPP

#include "utilities/globalDefinitions.hpp"

template <typename T> class Array;
class InstanceKlass;
class SerializeClosure;

// Classes to be bulk-loaded, in the "linked" state, at VM bootstrap.
//
// AOTLinkedClassTable is produced by AOTClassLinker when an AOTCache is assembled.
//
// AOTLinkedClassTable is consumed by AOTLinkedClassBulkLoader when an AOTCache is used
// in a production run.
//
class AOTLinkedClassTable {
  static AOTLinkedClassTable _instance;

  Array<InstanceKlass*>* _builtin_loader_classes;
  int _non_javabase_classes_start;
  int _platform_classes_start;
  int _app_classes_start;

public:
  AOTLinkedClassTable() :
    _builtin_loader_classes(nullptr), _non_javabase_classes_start(0), _platform_classes_start(0), _app_classes_start(0) {}
  static AOTLinkedClassTable* get() {
    return &_instance;
  }

  Array<InstanceKlass*>* builtin_loader_classes() const { return _builtin_loader_classes; }
  void set_builtin_loader_classes(Array<InstanceKlass*>* value) { _builtin_loader_classes = value; }

  int non_javabase_classes_start() const { return _non_javabase_classes_start; }
  int platform_classes_start() const { return _platform_classes_start; }
  int app_classes_start() const { return _app_classes_start; }
  void set_loader_boundaries(int non_javabase_classes_start, int plat_classes_start, int app_classes_start) {
    _non_javabase_classes_start = non_javabase_classes_start;
    _platform_classes_start = plat_classes_start;
    _app_classes_start = app_classes_start;
  }

  void serialize(SerializeClosure* soc);
};

#endif // SHARE_CDS_AOTLINKEDCLASSTABLE_HPP
