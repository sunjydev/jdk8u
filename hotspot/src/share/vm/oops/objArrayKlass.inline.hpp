/*
 * Copyright 2010 Sun Microsystems, Inc.  All Rights Reserved.
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
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */

void objArrayKlass::oop_follow_contents(oop obj, int index) {
  if (UseCompressedOops) {
    objarray_follow_contents<narrowOop>(obj, index);
  } else {
    objarray_follow_contents<oop>(obj, index);
  }
}

template <class T>
void objArrayKlass::objarray_follow_contents(oop obj, int index) {
  objArrayOop a = objArrayOop(obj);
  const size_t len = size_t(a->length());
  const size_t beg_index = size_t(index);
  assert(beg_index < len || len == 0, "index too large");

  const size_t stride = MIN2(len - beg_index, ObjArrayMarkingStride);
  const size_t end_index = beg_index + stride;
  T* const base = (T*)a->base();
  T* const beg = base + beg_index;
  T* const end = base + end_index;

  // Push the non-NULL elements of the next stride on the marking stack.
  for (T* e = beg; e < end; e++) {
    MarkSweep::mark_and_push<T>(e);
  }

  if (end_index < len) {
    MarkSweep::push_objarray(a, end_index); // Push the continuation.
  }
}

#ifndef SERIALGC
void objArrayKlass::oop_follow_contents(ParCompactionManager* cm, oop obj,
                                        int index) {
  if (UseCompressedOops) {
    objarray_follow_contents<narrowOop>(cm, obj, index);
  } else {
    objarray_follow_contents<oop>(cm, obj, index);
  }
}

template <class T>
void objArrayKlass::objarray_follow_contents(ParCompactionManager* cm, oop obj,
                                             int index) {
  objArrayOop a = objArrayOop(obj);
  const size_t len = size_t(a->length());
  const size_t beg_index = size_t(index);
  assert(beg_index < len || len == 0, "index too large");

  const size_t stride = MIN2(len - beg_index, ObjArrayMarkingStride);
  const size_t end_index = beg_index + stride;
  T* const base = (T*)a->base();
  T* const beg = base + beg_index;
  T* const end = base + end_index;

  // Push the non-NULL elements of the next stride on the marking stack.
  for (T* e = beg; e < end; e++) {
    PSParallelCompact::mark_and_push<T>(cm, e);
  }

  if (end_index < len) {
    cm->push_objarray(a, end_index); // Push the continuation.
  }
}
#endif // #ifndef SERIALGC