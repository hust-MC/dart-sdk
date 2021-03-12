// Copyright (c) 2013, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import "package:async_helper/async_helper.dart";
import "package:expect/expect.dart";

void testZLibInflate_regress10026() {
  test(data, expect) {
    asyncStart();
    var controller = new StreamController(sync: true);
    controller.stream
        .transform(zlib.decoder)
        .transform(utf8.decoder)
        .fold(new StringBuffer(), (buffer, s) {
      buffer.write(s);
      return buffer;
    }).then((out) {
      Expect.equals(out.toString(), expect);
      asyncEnd();
    });
    controller.add(data);
    controller.close();
  }

  // Generated by using 'gzip -c | od -v -tu1 -An -w12' and adding commas.
  test([
    31,
    139,
    8,
    8,
    238,
    42,
    167,
    81,
    0,
    3,
    116,
    101,
    120,
    116,
    46,
    116,
    120,
    116,
    0,
    125,
    84,
    79,
    175,
    147,
    64,
    16,
    63,
    183,
    159,
    98,
    196,
    139,
    38,
    165,
    244,
    249,
    212,
    52,
    20,
    136,
    70,
    77,
    188,
    168,
    7,
    189,
    120,
    156,
    178,
    67,
    153,
    20,
    118,
    113,
    119,
    161,
    109,
    140,
    223,
    221,
    97,
    105,
    251,
    170,
    47,
    154,
    54,
    41,
    51,
    195,
    254,
    254,
    49,
    52,
    123,
    162,
    76,
    233,
    79,
    29,
    65,
    237,
    219,
    166,
    152,
    103,
    151,
    31,
    66,
    85,
    204,
    103,
    153,
    103,
    223,
    80,
    241,
    225,
    136,
    109,
    215,
    16,
    188,
    55,
    45,
    178,
    206,
    146,
    169,
    59,
    151,
    121,
    75,
    30,
    161,
    172,
    209,
    58,
    242,
    121,
    212,
    251,
    42,
    94,
    71,
    144,
    20,
    151,
    73,
    237,
    125,
    23,
    211,
    143,
    158,
    135,
    60,
    122,
    103,
    180,
    39,
    237,
    227,
    145,
    45,
    130,
    114,
    170,
    242,
    200,
    211,
    209,
    39,
    35,
    235,
    230,
    138,
    243,
    8,
    70,
    99,
    75,
    121,
    52,
    48,
    29,
    58,
    99,
    253,
    205,
    225,
    3,
    43,
    95,
    231,
    138,
    6,
    46,
    41,
    14,
    197,
    2,
    88,
    179,
    103,
    108,
    98,
    87,
    98,
    67,
    249,
    221,
    25,
    199,
    249,
    147,
    24,
    24,
    185,
    207,
    148,
    165,
    115,
    145,
    12,
    182,
    70,
    157,
    224,
    231,
    124,
    54,
    219,
    98,
    185,
    223,
    89,
    211,
    107,
    21,
    151,
    166,
    49,
    54,
    133,
    167,
    213,
    74,
    62,
    47,
    54,
    50,
    108,
    209,
    238,
    88,
    167,
    176,
    26,
    139,
    14,
    149,
    98,
    189,
    59,
    87,
    149,
    104,
    137,
    43,
    108,
    185,
    57,
    165,
    16,
    125,
    233,
    72,
    195,
    87,
    212,
    46,
    90,
    64,
    244,
    145,
    154,
    129,
    60,
    151,
    8,
    159,
    169,
    39,
    233,
    92,
    27,
    11,
    120,
    107,
    69,
    227,
    2,
    156,
    220,
    26,
    59,
    178,
    92,
    109,
    36,
    206,
    95,
    243,
    153,
    226,
    33,
    200,
    9,
    102,
    82,
    120,
    189,
    90,
    117,
    199,
    91,
    5,
    175,
    168,
    5,
    236,
    189,
    249,
    67,
    200,
    61,
    181,
    155,
    127,
    88,
    168,
    170,
    48,
    49,
    86,
    145,
    141,
    45,
    42,
    238,
    93,
    10,
    119,
    225,
    126,
    97,
    195,
    180,
    97,
    189,
    95,
    0,
    166,
    3,
    59,
    246,
    164,
    2,
    247,
    229,
    240,
    253,
    250,
    229,
    122,
    29,
    206,
    143,
    137,
    197,
    138,
    74,
    99,
    209,
    179,
    17,
    25,
    218,
    104,
    154,
    32,
    222,
    180,
    164,
    24,
    225,
    89,
    139,
    199,
    248,
    86,
    244,
    243,
    41,
    213,
    75,
    188,
    255,
    17,
    39,
    32,
    87,
    219,
    23,
    223,
    23,
    139,
    15,
    201,
    63,
    180,
    254,
    50,
    19,
    158,
    194,
    67,
    22,
    147,
    183,
    17,
    84,
    190,
    89,
    18,
    158,
    187,
    44,
    116,
    50,
    109,
    244,
    60,
    27,
    21,
    73,
    45,
    132,
    227,
    90,
    212,
    119,
    143,
    150,
    91,
    90,
    50,
    232,
    138,
    111,
    53,
    59,
    80,
    161,
    9,
    114,
    69,
    206,
    227,
    182,
    97,
    87,
    75,
    72,
    222,
    192,
    150,
    160,
    119,
    114,
    89,
    25,
    11,
    220,
    52,
    189,
    243,
    99,
    52,
    3,
    1,
    77,
    112,
    78,
    246,
    80,
    78,
    151,
    125,
    43,
    139,
    234,
    150,
    240,
    221,
    244,
    82,
    74,
    110,
    30,
    52,
    5,
    136,
    16,
    180,
    88,
    97,
    141,
    158,
    64,
    96,
    208,
    237,
    3,
    92,
    71,
    182,
    101,
    231,
    36,
    231,
    145,
    72,
    88,
    192,
    223,
    74,
    209,
    87,
    10,
    121,
    110,
    90,
    1,
    251,
    81,
    222,
    8,
    140,
    3,
    114,
    35,
    34,
    105,
    132,
    17,
    120,
    75,
    59,
    158,
    116,
    25,
    189,
    204,
    146,
    110,
    242,
    149,
    201,
    107,
    105,
    169,
    202,
    163,
    241,
    229,
    76,
    147,
    228,
    112,
    56,
    44,
    25,
    53,
    46,
    141,
    221,
    37,
    19,
    137,
    75,
    92,
    71,
    165,
    44,
    104,
    84,
    124,
    50,
    150,
    132,
    83,
    0,
    219,
    9,
    103,
    41,
    72,
    88,
    4,
    180,
    44,
    9,
    41,
    102,
    201,
    57,
    211,
    100,
    250,
    243,
    248,
    13,
    215,
    32,
    235,
    247,
    84,
    4,
    0,
    0
  ], '''
<!doctype html>
<html>
<head>
	<title>Example Domain</title>

	<meta charset="utf-8" />
	<meta http-equiv="Content-type" content="text/html; charset=utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<style type="text/css">
	body {
		background-color: #f0f0f2;
		margin: 0;
		padding: 0;
		font-family: "Open Sans", "Helvetica Neue", Helvetica, Arial, sans-serif;

	}
	div {
		width: 600px;
		margin: 5em auto;
		padding: 3em;
		background-color: #fff;
		border-radius: 1em;
	}
	a:link, a:visited {
		color: #38488f;
		text-decoration: none;
	}
	@media (max-width: 600px) {
		body {
			background-color: #fff;
		}
		div {
			width: auto;
			margin: 0 auto;
			border-radius: 0;
			padding: 1em;
		}
	}
	</style>
</head>

<body>
<div>
	<h1>Example Domain</h1>
	<p>This domain is established to be used for illustrative examples in documents. You do not need to
		coordinate or ask for permission to use this domain in examples, and it is not available for
		registration.</p>
	<p><a href="http://www.iana.org/domains/special">More information...</a></p>
</div>
</body>
</html>
''');
}

void main() {
  testZLibInflate_regress10026();
}
