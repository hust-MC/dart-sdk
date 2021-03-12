// Copyright (c) 2017, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

class A {}

class B extends A {}

void main() {
  A a = new B();
  B Function(A) f = (A a) {
    return a;
    //     ^
    // [analyzer] STATIC_TYPE_WARNING.RETURN_OF_INVALID_TYPE_FROM_CLOSURE
    // [cfe] A value of type 'A' can't be assigned to a variable of type 'B'.
  };
}
