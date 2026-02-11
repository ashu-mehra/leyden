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

#include "cds/aotLinkedClassTable.hpp"
#include "cds/cdsConfig.hpp"
#include "cds/serializeClosure.hpp"
#include "oops/array.hpp"

AOTLinkedClassTable AOTLinkedClassTable::_instance;

void AOTLinkedClassTable::serialize(SerializeClosure* soc) {
  soc->do_ptr((void**)&_builtin_loader_classes);
  soc->do_int(&_non_javabase_classes_start);
  soc->do_int(&_platform_classes_start);
  soc->do_int(&_app_classes_start);
}


void AOTLinkedClassTable::write_classes() {
  write_classes(nullptr, true, list);
  int non_javabase_classes_start = list.length();
  write_classes(nullptr, false, list);
  int plat_classes_start = list.length();
  write_classes(systemdictionary::java_platform_loader(), false, list);
  int app_classes_start = list.length();
  write_classes(systemdictionary::java_system_loader(), false, list);
  if (list.length() == 0) {
    table->set_builtin_loader_classes(nullptr);
  } else {
    table->set_builtin_loader_classes(archiveutils::archive_array(&list));
  }
  if (log_is_enabled(info, aot, link)) {
    resourcemark rm;
    log_info(aot, link)("builtin_loader boundaries:");
    log_info(aot, link)(" non_javabase_classes_start=%d", non_javabase_classes_start);
    log_info(aot, link)(" plat_classes_start=%d", plat_classes_start);
    log_info(aot, link)(" app_classes_start=%d", app_classes_start);
    log_info(aot, link)("total=%d", list.length());
  }

  table->set_loader_boundaries(non_javabase_classes_start, plat_classes_start, app_classes_start);
}

void AOTLinkedClassTable::write_classes(GrowableArray<Klass*>* class_list, bool is_javabase) {
  int count = 0;
  for (int i = 0; i < class_list->length(); i++) {
    InstanceKlass* ik = _sorted_candidates->at(i);
    if (ik->class_loader() != class_loader) {
      continue;
    }
    if ((ik->module() == ModuleEntryTable::javabase_moduleEntry()) != is_javabase) {
      continue;
    }

    class_list.append(ArchiveBuilder::current()->get_buffered_addr(ik));
    count += 1;
  }

  if (count != 0) {
    const char* category = class_category_name(class_list.last());
    log_info(aot, link)("wrote %d class(es) for category %s", count, category);
  }
}

