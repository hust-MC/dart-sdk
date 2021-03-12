// Copyright (c) 2017, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

class A {}

class B extends A {}

B f1(A a) {
  return a;
  //     ^
  // [analyzer] STATIC_TYPE_WARNING.RETURN_OF_INVALID_TYPE
  // [cfe] A value of type 'A' can't be assigned to a variable of type 'B'.
}

B f2(A a) => a;
//           ^
// [analyzer] STATIC_TYPE_WARNING.RETURN_OF_INVALID_TYPE
// [cfe] A value of type 'A' can't be assigned to a variable of type 'B'.

void main() {
  Object b;
  A a = new B();
  b = f1(a);
  b = f2(a);
}
