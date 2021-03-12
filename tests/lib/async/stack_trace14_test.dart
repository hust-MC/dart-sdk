// Copyright (c) 2013, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:expect/expect.dart';
import 'package:async_helper/async_helper.dart';
import 'dart:async';

StackTrace captureStackTrace() {
  try {
    throw 0;
  } catch (e, st) {
    return st;
  }
}

main() {
  StackTrace trace = captureStackTrace();
  asyncStart();
  var f = new Future(() => 499);
  f.then((_) => new Future.error("e", trace)).then<Null>((_) {
    throw "Unreachable";
  }, onError: (e, st) {
    Expect.equals("e", e);
    Expect.identical(trace, st);
    asyncEnd();
  });
}
