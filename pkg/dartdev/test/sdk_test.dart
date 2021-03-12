// Copyright (c) 2020, the Dart project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:io';

import 'package:dartdev/src/sdk.dart';
import 'package:test/test.dart';

void main() {
  group('Sdk', _sdk);
}

void _sdk() {
  test('sdkPath', () {
    expectDirectoryExists(Sdk().sdkPath);
  });

  test('dart binary', () {
    expectFileExists(Sdk().dart);
  });

  test('analysis_server_snapshot', () {
    expectFileExists(Sdk().analysisServerSnapshot);
  });

  test('dartfmt', () {
    expectFileExists(Sdk().dartfmt);
  });

  test('pub', () {
    expectFileExists(Sdk().pub);
  });
}

void expectFileExists(String path) {
  expect(File(path).existsSync(), isTrue);
}

void expectDirectoryExists(String path) {
  expect(Directory(path).existsSync(), isTrue);
}
