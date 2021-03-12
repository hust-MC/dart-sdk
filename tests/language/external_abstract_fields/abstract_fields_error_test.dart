// Copyright (c) 2020, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// Check that abstract instance variable declarations are abstract and
// do not allow more than they should.

// Check that abstract declarations are abstract.
class Abstract1 {
  // The class is not abstract and does not have an implementation of `x`.
  abstract int x;
  //           ^
  // [analyzer] unspecified
  // [cfe] unspecified
}

// Check that class has expected interface.
abstract class Abstract2 {
  // Getter only.
  abstract final int x;
  // Getter and setter.
  abstract covariant String y;

  static void staticTest(Abstract2 a) {
    int x = a.x;

    // Cannot assign to final field.
    a.x = 42;
    //  ^
    // [analyzer] unspecified
    // [cfe] unspecified

    String y = a.y;
    a.y = "ab";

    // Cannot assign something of wrong type, even if covariant.
    a.y = Object();
    //    ^^^^^^^^
    // [analyzer] unspecified
    // [cfe] unspecified
  }

  void test() {
    int x = this.x;

    // Cannot assign to final field.
    this.x = 42;
    //     ^
    // [analyzer] unspecified
    // [cfe] unspecified

    String y = this.y;
    this.y = "ab";

    // Cannot assign something of wrong type, even if covariant.
    this.y = Object();
    //       ^^^^^^^^
    // [analyzer] unspecified
    // [cfe] unspecified
  }
}

class Concrete2 implements Abstract2 {
  final int x;
  String y;
}

void main() {
  Abstract2.staticTest(throw 0);
  Concrete2().test();
}
