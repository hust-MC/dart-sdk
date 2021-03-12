// Copyright (c) 2017, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import "package:expect/expect.dart";

class A {}

class B extends A {}

void main() {
  A a1 = new B();
  A a2 = new A();
  B b1 = a1; // No error
  Expect.throwsTypeError(() {
    B b2 = a2;
  });
}
