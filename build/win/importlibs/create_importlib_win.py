#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""Creates an import library from an import description file."""
import ast
import logging
import optparse
import os
import os.path
import shutil
import subprocess
import sys
import tempfile

_USAGE = """\
Usage: %prog [options] [imports-file]

Creates an import library from imports-file.

Note: this script uses the microsoft assembler (ml.exe) and the library tool
    (lib.exe), both of which must be in path.
"""

_ASM_STUB_HEADER = """\
; This file is autogenerated by create_importlib_win.py, do not edit.
.386
.MODEL FLAT, C
.CODE

; Stubs to provide mangled names to lib.exe for the
; correct generation of import libs.
"""

_DEF_STUB_HEADER = """\
; This file is autogenerated by create_importlib_win.py, do not edit.

; Export declarations for generating import libs.
"""

_LOGGER = logging.getLogger()


class _Error(Exception):
    pass


class _ImportLibraryGenerator(object):

    def __init__(self, temp_dir):
        self._temp_dir = temp_dir

    def _Shell(self, cmd, **kw):
        ret = subprocess.call(cmd, **kw)
        _LOGGER.info('Running "%s" returned %d.', cmd, ret)
        if ret != 0:
            raise _Error('Command "%s" returned %d.' % (cmd, ret))

    def _ReadImportsFile(self, imports_file):
        # Slurp the imports file.
        return ast.literal_eval(open(imports_file).read())

    def _WriteStubsFile(self, import_names, output_file):
        output_file.write(_ASM_STUB_HEADER)

        for name in import_names:
            output_file.write('%s PROC\n' % name)
            output_file.write('%s ENDP\n' % name)

        output_file.write('END\n')

    def _WriteDefFile(self, dll_name, import_names, output_file):
        output_file.write(_DEF_STUB_HEADER)
        output_file.write('NAME %s\n' % dll_name)
        output_file.write('EXPORTS\n')
        for name in import_names:
            name = name.split('@')[0]
            output_file.write('  %s\n' % name)

    def _CreateObj(self, dll_name, imports):
        """Writes an assembly file containing empty declarations.

    For each imported function of the form:

    AddClipboardFormatListener@4 PROC
    AddClipboardFormatListener@4 ENDP

    The resulting object file is then supplied to lib.exe with a .def file
    declaring the corresponding non-adorned exports as they appear on the
    exporting DLL, e.g.

    EXPORTS
      AddClipboardFormatListener

    In combination, the .def file and the .obj file cause lib.exe to generate
    an x86 import lib with public symbols named like
    "__imp__AddClipboardFormatListener@4", binding to exports named like
    "AddClipboardFormatListener".

    All of this is perpetrated in a temporary directory, as the intermediate
    artifacts are quick and easy to produce, and of no interest to anyone
    after the fact."""

        # Create an .asm file to provide stdcall-like stub names to lib.exe.
        asm_name = dll_name + '.asm'
        _LOGGER.info('Writing asm file "%s".', asm_name)
        with open(os.path.join(self._temp_dir, asm_name), 'wb') as stubs_file:
            self._WriteStubsFile(imports, stubs_file)

        # Invoke on the assembler to compile it to .obj.
        obj_name = dll_name + '.obj'
        cmdline = ['ml.exe', '/nologo', '/c', asm_name, '/Fo', obj_name]
        self._Shell(cmdline, cwd=self._temp_dir, stdout=open(os.devnull))

        return obj_name

    def _CreateImportLib(self, dll_name, imports, architecture, output_file):
        """Creates an import lib binding imports to dll_name for architecture.

    On success, writes the import library to output file.
    """
        obj_file = None

        # For x86 architecture we have to provide an object file for correct
        # name mangling between the import stubs and the exported functions.
        if architecture == 'x86':
            obj_file = self._CreateObj(dll_name, imports)

        # Create the corresponding .def file. This file has the non stdcall-adorned
        # names, as exported by the destination DLL.
        def_name = dll_name + '.def'
        _LOGGER.info('Writing def file "%s".', def_name)
        with open(os.path.join(self._temp_dir, def_name), 'wb') as def_file:
            self._WriteDefFile(dll_name, imports, def_file)

        # Invoke on lib.exe to create the import library.
        # We generate everything into the temporary directory, as the .exp export
        # files will be generated at the same path as the import library, and we
        # don't want those files potentially gunking the works.
        dll_base_name, ext = os.path.splitext(dll_name)
        lib_name = dll_base_name + '.lib'
        cmdline = [
            'lib.exe',
            '/machine:%s' % architecture,
            '/def:%s' % def_name,
            '/out:%s' % lib_name
        ]
        if obj_file:
            cmdline.append(obj_file)

        self._Shell(cmdline, cwd=self._temp_dir, stdout=open(os.devnull))

        # Copy the .lib file to the output directory.
        shutil.copyfile(os.path.join(self._temp_dir, lib_name), output_file)
        _LOGGER.info('Created "%s".', output_file)

    def CreateImportLib(self, imports_file, output_file):
        # Read the imports file.
        imports = self._ReadImportsFile(imports_file)

        # Creates the requested import library in the output directory.
        self._CreateImportLib(imports['dll_name'], imports['imports'],
                              imports.get('architecture', 'x86'), output_file)


def main():
    parser = optparse.OptionParser(usage=_USAGE)
    parser.add_option(
        '-o', '--output-file', help='Specifies the output file path.')
    parser.add_option(
        '-k',
        '--keep-temp-dir',
        action='store_true',
        help='Keep the temporary directory.')
    parser.add_option(
        '-v', '--verbose', action='store_true', help='Verbose logging.')

    options, args = parser.parse_args()

    if len(args) != 1:
        parser.error('You must provide an imports file.')

    if not options.output_file:
        parser.error('You must provide an output file.')

    options.output_file = os.path.abspath(options.output_file)

    if options.verbose:
        logging.basicConfig(level=logging.INFO)
    else:
        logging.basicConfig(level=logging.WARN)

    temp_dir = tempfile.mkdtemp()
    _LOGGER.info('Created temporary directory "%s."', temp_dir)
    try:
        # Create a generator and create the import lib.
        generator = _ImportLibraryGenerator(temp_dir)

        ret = generator.CreateImportLib(args[0], options.output_file)
    except Exception, e:
        _LOGGER.exception('Failed to create import lib.')
        ret = 1
    finally:
        if not options.keep_temp_dir:
            shutil.rmtree(temp_dir)
            _LOGGER.info('Deleted temporary directory "%s."', temp_dir)

    return ret


if __name__ == '__main__':
    sys.exit(main())
