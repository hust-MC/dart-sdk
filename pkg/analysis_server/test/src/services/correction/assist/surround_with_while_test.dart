// Copyright (c) 2018, the Dart project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:analysis_server/src/services/correction/assist.dart';
import 'package:analyzer_plugin/utilities/assist/assist.dart';
import 'package:test_reflective_loader/test_reflective_loader.dart';

import 'assist_processor.dart';

void main() {
  defineReflectiveSuite(() {
    defineReflectiveTests(SurroundWithWhileTest);
  });
}

@reflectiveTest
class SurroundWithWhileTest extends AssistProcessorTest {
  @override
  AssistKind get kind => DartAssistKind.SURROUND_WITH_WHILE;

  Future<void> test_twoStatements() async {
    await resolveTestUnit('''
main() {
// start
  print(0);
  print(1);
// end
}
''');
    await assertHasAssist('''
main() {
  while (condition) {
    print(0);
    print(1);
  }
}
''');
    assertLinkedGroup(0, ['condition']);
    assertExitPosition(after: '  }');
  }
}
