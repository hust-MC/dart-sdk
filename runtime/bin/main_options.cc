// Copyright (c) 2017, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "bin/main_options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin/abi_version.h"
#include "bin/dartdev_utils.h"
#include "bin/error_exit.h"
#include "bin/options.h"
#include "bin/platform.h"
#include "bin/utils.h"
#include "platform/syslog.h"
#if !defined(DART_IO_SECURE_SOCKET_DISABLED)
#include "bin/security_context.h"
#endif  // !defined(DART_IO_SECURE_SOCKET_DISABLED)
#include "bin/socket.h"
#include "include/dart_api.h"
#include "platform/assert.h"
#include "platform/globals.h"
#include "platform/hashmap.h"

namespace dart {
namespace bin {

// These strings must match the enum SnapshotKind in main_options.h.
static const char* kSnapshotKindNames[] = {
    "none",
    "kernel",
    "app-jit",
    NULL,
};

static const char* kEnableExperiment1 = "--enable-experiment";
static const char* kEnableExperiment2 = "--enable_experiment";

SnapshotKind Options::gen_snapshot_kind_ = kNone;
bool Options::enable_vm_service_ = false;
MallocGrowableArray<const char*> Options::enabled_experiments_ =
    MallocGrowableArray<const char*>(4);
bool Options::disable_dart_dev_ = true;

#define OPTION_FIELD(variable) Options::variable##_

#define STRING_OPTION_DEFINITION(name, variable)                               \
  const char* OPTION_FIELD(variable) = NULL;                                   \
  DEFINE_STRING_OPTION(name, OPTION_FIELD(variable))
STRING_OPTIONS_LIST(STRING_OPTION_DEFINITION)
#undef STRING_OPTION_DEFINITION

#define BOOL_OPTION_DEFINITION(name, variable)                                 \
  bool OPTION_FIELD(variable) = false;                                         \
  DEFINE_BOOL_OPTION(name, OPTION_FIELD(variable))
BOOL_OPTIONS_LIST(BOOL_OPTION_DEFINITION)
#if defined(DEBUG)
DEBUG_BOOL_OPTIONS_LIST(BOOL_OPTION_DEFINITION)
#endif
#undef BOOL_OPTION_DEFINITION

#define SHORT_BOOL_OPTION_DEFINITION(short_name, long_name, variable)          \
  bool OPTION_FIELD(variable) = false;                                         \
  DEFINE_BOOL_OPTION_SHORT(short_name, long_name, OPTION_FIELD(variable))
SHORT_BOOL_OPTIONS_LIST(SHORT_BOOL_OPTION_DEFINITION)
#undef SHORT_BOOL_OPTION_DEFINITION

#define ENUM_OPTION_DEFINITION(name, type, variable)                           \
  DEFINE_ENUM_OPTION(name, type, OPTION_FIELD(variable))
ENUM_OPTIONS_LIST(ENUM_OPTION_DEFINITION)
#undef ENUM_OPTION_DEFINITION

#define CB_OPTION_DEFINITION(callback)                                         \
  static bool callback##Helper(const char* arg, CommandLineOptions* o) {       \
    return Options::callback(arg, o);                                          \
  }                                                                            \
  DEFINE_CB_OPTION(callback##Helper)
CB_OPTIONS_LIST(CB_OPTION_DEFINITION)
#undef CB_OPTION_DEFINITION

#if !defined(DART_PRECOMPILED_RUNTIME)
DFE* Options::dfe_ = NULL;

DEFINE_STRING_OPTION_CB(dfe, { Options::dfe()->set_frontend_filename(value); });
#endif  // !defined(DART_PRECOMPILED_RUNTIME)

static void hot_reload_test_mode_callback(CommandLineOptions* vm_options) {
  // Identity reload.
  vm_options->AddArgument("--identity_reload");
  // Start reloading quickly.
  vm_options->AddArgument("--reload_every=4");
  // Reload from optimized and unoptimized code.
  vm_options->AddArgument("--reload_every_optimized=false");
  // Reload less frequently as time goes on.
  vm_options->AddArgument("--reload_every_back_off");
  // Ensure that every isolate has reloaded once before exiting.
  vm_options->AddArgument("--check_reloaded");
#if !defined(DART_PRECOMPILED_RUNTIME)
  Options::dfe()->set_use_incremental_compiler(true);
#endif  // !defined(DART_PRECOMPILED_RUNTIME)
}

DEFINE_BOOL_OPTION_CB(hot_reload_test_mode, hot_reload_test_mode_callback);

static void hot_reload_rollback_test_mode_callback(
    CommandLineOptions* vm_options) {
  // Identity reload.
  vm_options->AddArgument("--identity_reload");
  // Start reloading quickly.
  vm_options->AddArgument("--reload_every=4");
  // Reload from optimized and unoptimized code.
  vm_options->AddArgument("--reload_every_optimized=false");
  // Reload less frequently as time goes on.
  vm_options->AddArgument("--reload_every_back_off");
  // Ensure that every isolate has reloaded once before exiting.
  vm_options->AddArgument("--check_reloaded");
  // Force all reloads to fail and execute the rollback code.
  vm_options->AddArgument("--reload_force_rollback");
#if !defined(DART_PRECOMPILED_RUNTIME)
  Options::dfe()->set_use_incremental_compiler(true);
#endif  // !defined(DART_PRECOMPILED_RUNTIME)
}

DEFINE_BOOL_OPTION_CB(hot_reload_rollback_test_mode,
                      hot_reload_rollback_test_mode_callback);

void Options::PrintVersion() {
  Syslog::PrintErr("Dart SDK version: %s\n", Dart_VersionString());
}

// clang-format off
void Options::PrintUsage() {
  Syslog::PrintErr(
      "Usage: dart [<vm-flags>] <dart-script-file> [<script-arguments>]\n"
      "\n"
      "Executes the Dart script <dart-script-file> with "
      "the given list of <script-arguments>.\n"
      "\n");
  if (!Options::verbose_option()) {
    Syslog::PrintErr(
"Common VM flags:\n"
"--enable-asserts\n"
"  Enable assert statements.\n"
"--help or -h\n"
"  Display this message (add -v or --verbose for information about\n"
"  all VM options).\n"
"--packages=<path>\n"
"  Where to find a package spec file.\n"
"--observe[=<port>[/<bind-address>]]\n"
"  The observe flag is a convenience flag used to run a program with a\n"
"  set of options which are often useful for debugging under Observatory.\n"
"  These options are currently:\n"
"      --enable-vm-service[=<port>[/<bind-address>]]\n"
"      --pause-isolates-on-exit\n"
"      --pause-isolates-on-unhandled-exceptions\n"
"      --warn-on-pause-with-no-debugger\n"
"  This set is subject to change.\n"
"  Please see these options (--help --verbose) for further documentation.\n"
"--write-service-info=<file_uri>\n"
"  Outputs information necessary to connect to the VM service to the\n"
"  specified file in JSON format. Useful for clients which are unable to\n"
"  listen to stdout for the Observatory listening message.\n"
"--snapshot-kind=<snapshot_kind>\n"
"--snapshot=<file_name>\n"
"  These snapshot options are used to generate a snapshot of the loaded\n"
"  Dart script:\n"
"    <snapshot-kind> controls the kind of snapshot, it could be\n"
"                    kernel(default) or app-jit\n"
"    <file_name> specifies the file into which the snapshot is written\n"
"--version\n"
"  Print the SDK version.\n");
  } else {
    Syslog::PrintErr(
"Supported options:\n"
"--enable-asserts\n"
"  Enable assert statements.\n"
"--help or -h\n"
"  Display this message (add -v or --verbose for information about\n"
"  all VM options).\n"
"--packages=<path>\n"
"  Where to find a package spec file.\n"
"--observe[=<port>[/<bind-address>]]\n"
"  The observe flag is a convenience flag used to run a program with a\n"
"  set of options which are often useful for debugging under Observatory.\n"
"  These options are currently:\n"
"      --enable-vm-service[=<port>[/<bind-address>]]\n"
"      --pause-isolates-on-exit\n"
"      --pause-isolates-on-unhandled-exceptions\n"
"      --warn-on-pause-with-no-debugger\n"
"  This set is subject to change.\n"
"  Please see these options for further documentation.\n"
"--write-service-info=<file_uri>\n"
"  Outputs information necessary to connect to the VM service to the\n"
"  specified file in JSON format. Useful for clients which are unable to\n"
"  listen to stdout for the Observatory listening message.\n"
"--snapshot-kind=<snapshot_kind>\n"
"--snapshot=<file_name>\n"
"  These snapshot options are used to generate a snapshot of the loaded\n"
"  Dart script:\n"
"    <snapshot-kind> controls the kind of snapshot, it could be\n"
"                    kernel(default) or app-jit\n"
"    <file_name> specifies the file into which the snapshot is written\n"
"--version\n"
"  Print the VM version.\n"
"\n"
"--trace-loading\n"
"  enables tracing of library and script loading\n"
"\n"
"--enable-vm-service[=<port>[/<bind-address>]]\n"
"  Enables the VM service and listens on specified port for connections\n"
"  (default port number is 8181, default bind address is localhost).\n"
"\n"
"--disable-service-auth-codes\n"
"  Disables the requirement for an authentication code to communicate with\n"
"  the VM service. Authentication codes help protect against CSRF attacks,\n"
"  so it is not recommended to disable them unless behind a firewall on a\n"
"  secure device.\n"
"\n"
"--enable-service-port-fallback\n"
"  When the VM service is told to bind to a particular port, fallback to 0 if\n"
"  it fails to bind instead of failing to start.\n"
"\n"
"--root-certs-file=<path>\n"
"  The path to a file containing the trusted root certificates to use for\n"
"  secure socket connections.\n"
"--root-certs-cache=<path>\n"
"  The path to a cache directory containing the trusted root certificates to\n"
"  use for secure socket connections.\n"
#if defined(HOST_OS_LINUX) || \
    defined(HOST_OS_ANDROID) || \
    defined(HOST_OS_FUCHSIA)
"--namespace=<path>\n"
"  The path to a directory that dart:io calls will treat as the root of the\n"
"  filesystem.\n"
#endif  // defined(HOST_OS_LINUX) || defined(HOST_OS_ANDROID)
"\n"
"The following options are only used for VM development and may\n"
"be changed in any future version:\n");
    const char* print_flags = "--print_flags";
    char* error = Dart_SetVMFlags(1, &print_flags);
    ASSERT(error == NULL);
  }
}
// clang-format on

dart::SimpleHashMap* Options::environment_ = NULL;
bool Options::ProcessEnvironmentOption(const char* arg,
                                       CommandLineOptions* vm_options) {
  return OptionProcessor::ProcessEnvironmentOption(arg, vm_options,
                                                   &Options::environment_);
}

void Options::DestroyEnvironment() {
  if (environment_ != NULL) {
    for (SimpleHashMap::Entry* p = environment_->Start(); p != NULL;
         p = environment_->Next(p)) {
      free(p->key);
      free(p->value);
    }
    delete environment_;
    environment_ = NULL;
  }
}

bool Options::ExtractPortAndAddress(const char* option_value,
                                    int* out_port,
                                    const char** out_ip,
                                    int default_port,
                                    const char* default_ip) {
  // [option_value] has to be one of the following formats:
  //   - ""
  //   - ":8181"
  //   - "=8181"
  //   - ":8181/192.168.0.1"
  //   - "=8181/192.168.0.1"
  //   - "=8181/::1"

  if (*option_value == '\0') {
    *out_ip = default_ip;
    *out_port = default_port;
    return true;
  }

  if ((*option_value != '=') && (*option_value != ':')) {
    return false;
  }

  int port = atoi(option_value + 1);
  const char* slash = strstr(option_value, "/");
  if (slash == NULL) {
    *out_ip = default_ip;
    *out_port = port;
    return true;
  }

  *out_ip = slash + 1;
  *out_port = port;
  return true;
}

static const char* DEFAULT_VM_SERVICE_SERVER_IP = "localhost";
static const int DEFAULT_VM_SERVICE_SERVER_PORT = 8181;
static const int INVALID_VM_SERVICE_SERVER_PORT = -1;

const char* Options::vm_service_server_ip_ = DEFAULT_VM_SERVICE_SERVER_IP;
int Options::vm_service_server_port_ = INVALID_VM_SERVICE_SERVER_PORT;
bool Options::ProcessEnableVmServiceOption(const char* arg,
                                           CommandLineOptions* vm_options) {
  const char* value =
      OptionProcessor::ProcessOption(arg, "--enable-vm-service");
  if (value == NULL) {
    return false;
  }
  if (!ExtractPortAndAddress(
          value, &vm_service_server_port_, &vm_service_server_ip_,
          DEFAULT_VM_SERVICE_SERVER_PORT, DEFAULT_VM_SERVICE_SERVER_IP)) {
    Syslog::PrintErr(
        "unrecognized --enable-vm-service option syntax. "
        "Use --enable-vm-service[=<port number>[/<bind address>]]\n");
    return false;
  }
#if !defined(DART_PRECOMPILED_RUNTIME)
  dfe()->set_use_incremental_compiler(true);
#endif  // !defined(DART_PRECOMPILED_RUNTIME)
  enable_vm_service_ = true;
  return true;
}

bool Options::ProcessObserveOption(const char* arg,
                                   CommandLineOptions* vm_options) {
  const char* value = OptionProcessor::ProcessOption(arg, "--observe");
  if (value == NULL) {
    return false;
  }
  if (!ExtractPortAndAddress(
          value, &vm_service_server_port_, &vm_service_server_ip_,
          DEFAULT_VM_SERVICE_SERVER_PORT, DEFAULT_VM_SERVICE_SERVER_IP)) {
    Syslog::PrintErr(
        "unrecognized --observe option syntax. "
        "Use --observe[=<port number>[/<bind address>]]\n");
    return false;
  }

  // These options should also be documented in the help message.
  vm_options->AddArgument("--pause-isolates-on-exit");
  vm_options->AddArgument("--pause-isolates-on-unhandled-exceptions");
  vm_options->AddArgument("--profiler");
  vm_options->AddArgument("--warn-on-pause-with-no-debugger");
#if !defined(DART_PRECOMPILED_RUNTIME)
  dfe()->set_use_incremental_compiler(true);
#endif  // !defined(DART_PRECOMPILED_RUNTIME)
  enable_vm_service_ = true;
  return true;
}

int Options::target_abi_version_ = Options::kAbiVersionUnset;
bool Options::ProcessAbiVersionOption(const char* arg,
                                      CommandLineOptions* vm_options) {
  const char* value = OptionProcessor::ProcessOption(arg, "--use_abi_version=");
  if (value == NULL) {
    return false;
  }
  int ver = 0;
  for (int i = 0; value[i] != '\0'; ++i) {
    if (value[i] >= '0' && value[i] <= '9') {
      ver = (ver * 10) + value[i] - '0';
    } else {
      Syslog::PrintErr("--use_abi_version must be an int\n");
      return false;
    }
  }
  if (ver < AbiVersion::GetOldestSupported() ||
      ver > AbiVersion::GetCurrent()) {
    Syslog::PrintErr("--use_abi_version must be between %d and %d inclusive\n",
                     AbiVersion::GetOldestSupported(),
                     AbiVersion::GetCurrent());
    return false;
  }
  target_abi_version_ = ver;
  return true;
}

bool Options::ProcessEnableExperimentOption(const char* arg,
                                            CommandLineOptions* vm_options) {
  const char* value =
      OptionProcessor::ProcessOption(arg, "--enable_experiment=");
  if (value == nullptr) {
    value = OptionProcessor::ProcessOption(arg, "--enable-experiment=");
  }
  if (value == nullptr) {
    return false;
  }
  vm_options->AddArgument(arg);
  Utils::CStringUniquePtr tmp = Utils::CreateCStringUniquePtr(
      Utils::StrDup(value));
  char* save_ptr;  // Needed for strtok_r.
  char* token = strtok_r(const_cast<char*>(tmp.get()), ",", &save_ptr);
  while (token != NULL) {
    enabled_experiments_.Add(token);
    token = strtok_r(NULL, ",", &save_ptr);
  }
  return true;
}

bool Options::ProcessEnableDartDevOption(const char* arg,
                                         CommandLineOptions* vm_options) {
  const char* value = OptionProcessor::ProcessOption(arg, "--enable-dart-dev");
  if (value == nullptr) {
    value = OptionProcessor::ProcessOption(arg, "--enable_dart_dev");
  }
  if (value == nullptr) {
    // Ensure --disable-dart-dev doesn't result in an unknown flag error.
    value = OptionProcessor::ProcessOption(arg, "--disable-dart-dev");
    if (value != nullptr) {
      return true;
    }
    value = OptionProcessor::ProcessOption(arg, "--disable_dart_dev");
    if (value != nullptr) {
      return true;
    }
    return false;
  }
  disable_dart_dev_ = false;
  return true;
}

static void ResolveDartDevSnapshotPath(const char* script,
                                       char** snapshot_path) {
  if (!DartDevUtils::TryResolveDartDevSnapshotPath(snapshot_path)) {
    Syslog::PrintErr(
        "Could not find DartDev snapshot and '%s' is not a valid script.\n",
        script);
    Platform::Exit(kErrorExitCode);
  }
}

int Options::ParseArguments(int argc,
                            char** argv,
                            bool vm_run_app_snapshot,
                            CommandLineOptions* vm_options,
                            char** script_name,
                            CommandLineOptions* dart_options,
                            bool* print_flags_seen,
                            bool* verbose_debug_seen) {
  const char* kPrefix = "--";
  const intptr_t kPrefixLen = strlen(kPrefix);

  // Store the executable name.
  Platform::SetExecutableName(argv[0]);

  // Start the rest after the executable name.
  int i = 1;

  CommandLineOptions temp_vm_options(vm_options->max_count());

  bool enable_dartdev_analytics = false;
  bool disable_dartdev_analytics = false;

  // Parse out the vm options.
  while (i < argc) {
    bool skipVmOption = false;
    if (OptionProcessor::TryProcess(argv[i], &temp_vm_options)) {
      i++;
    } else {
      // Check if this flag is a potentially valid VM flag.
      if (!OptionProcessor::IsValidFlag(argv[i], kPrefix, kPrefixLen)) {
        break;
      }
      // The following two flags are processed by both the embedder and
      // the VM.
      const char* kPrintFlags1 = "--print-flags";
      const char* kPrintFlags2 = "--print_flags";
      const char* kVerboseDebug1 = "--verbose_debug";
      const char* kVerboseDebug2 = "--verbose-debug";

      // The following two flags are processed as DartDev flags and are not to
      // be treated as if they are VM flags.
      const char* kEnableDartDevAnalytics1 = "--enable-analytics";
      const char* kEnableDartDevAnalytics2 = "--enable_analytics";
      const char* kDisableDartDevAnalytics1 = "--disable-analytics";
      const char* kDisableDartDevAnalytics2 = "--disable_analytics";

      if ((strncmp(argv[i], kPrintFlags1, strlen(kPrintFlags1)) == 0) ||
          (strncmp(argv[i], kPrintFlags2, strlen(kPrintFlags2)) == 0)) {
        *print_flags_seen = true;
      } else if ((strncmp(argv[i], kVerboseDebug1, strlen(kVerboseDebug1)) ==
                  0) ||
                 (strncmp(argv[i], kVerboseDebug2, strlen(kVerboseDebug2)) ==
                  0)) {
        *verbose_debug_seen = true;
      } else if ((strncmp(argv[i], kEnableDartDevAnalytics1,
                          strlen(kEnableDartDevAnalytics1)) == 0) ||
                 (strncmp(argv[i], kEnableDartDevAnalytics2,
                          strlen(kEnableDartDevAnalytics2)) == 0)) {
        enable_dartdev_analytics = true;
        skipVmOption = true;
      } else if ((strncmp(argv[i], kDisableDartDevAnalytics1,
                          strlen(kDisableDartDevAnalytics1)) == 0) ||
                 (strncmp(argv[i], kDisableDartDevAnalytics2,
                          strlen(kDisableDartDevAnalytics2)) == 0)) {
        disable_dartdev_analytics = true;
        skipVmOption = true;
      }
      if (!skipVmOption) {
        temp_vm_options.AddArgument(argv[i]);
      }
      i++;
    }
  }

  if (!Options::disable_dart_dev()) {
    // Don't start the VM service for the DartDev process. Without doing a
    // second pass over the argument list to explicitly check for
    // --disable-dart-dev, this is the earliest we can assume we know whether
    // or not we're running with DartDev enabled.
    vm_service_server_port_ = INVALID_VM_SERVICE_SERVER_PORT;
    vm_service_server_ip_ = DEFAULT_VM_SERVICE_SERVER_IP;
  }

#if !defined(DART_PRECOMPILED_RUNTIME)
  Options::dfe()->set_use_dfe();
#endif  // !defined(DART_PRECOMPILED_RUNTIME)
  if (Options::deterministic()) {
    // Both an embedder and VM flag.
    temp_vm_options.AddArgument("--deterministic");
  }

  Socket::set_short_socket_read(Options::short_socket_read());
  Socket::set_short_socket_write(Options::short_socket_write());
#if !defined(DART_IO_SECURE_SOCKET_DISABLED)
  SSLCertContext::set_root_certs_file(Options::root_certs_file());
  SSLCertContext::set_root_certs_cache(Options::root_certs_cache());
#endif  // !defined(DART_IO_SECURE_SOCKET_DISABLED)

  // The arguments to the VM are at positions 1 through i-1 in argv.
  Platform::SetExecutableArguments(i, argv);

  bool implicitly_use_dart_dev = false;
  bool run_script = false;
  int script_or_cmd_index = -1;
  // Get the script name.
  if (i < argc) {
    // If the script name is a valid file or a URL, we'll run the script
    // directly. Otherwise, this might be a DartDev command and we need to try
    // to find the DartDev snapshot so we can forward the command and its
    // arguments.
    bool is_potential_file_path = !DartDevUtils::ShouldParseCommand(argv[i]);
    script_or_cmd_index = i;
    if (Options::disable_dart_dev() ||
        (is_potential_file_path && !enable_vm_service_)) {
      *script_name = Utils::StrDup(argv[i]);
      run_script = true;
      i++;
    } else {
      ResolveDartDevSnapshotPath(argv[i], script_name);
    }
    // Handle the special case where the user is running a Dart program without
    // using a DartDev command and wants to use the VM service. Here we'll run
    // the program using DartDev as it's used to spawn a DDS instance
    if (!Options::disable_dart_dev() && is_potential_file_path &&
        enable_vm_service_) {
      implicitly_use_dart_dev = true;
      dart_options->AddArgument("run");
    }
  } else if (!Options::disable_dart_dev() &&
             ((Options::help_option() && !Options::verbose_option()) ||
              (argc == 1)) &&
             DartDevUtils::TryResolveDartDevSnapshotPath(script_name)) {
    // Let DartDev handle the default help message.
    dart_options->AddArgument("help");
    return 0;
  } else if (!Options::disable_dart_dev() &&
             (enable_dartdev_analytics || disable_dartdev_analytics)) {
    // The analytics flags are a special case as we don't have a target script
    // or DartDev command but we still want to launch DartDev.
    ResolveDartDevSnapshotPath(argv[i], script_name);

    if (enable_dartdev_analytics) {
      dart_options->AddArgument("--enable-analytics");
    }
    if (disable_dartdev_analytics) {
      dart_options->AddArgument("--disable-analytics");
    }
    return 0;
  } else {
    return -1;
  }

  const char** vm_argv = temp_vm_options.arguments();
  int vm_argc = temp_vm_options.count();

  if (Options::disable_dart_dev() || run_script) {
    // Only populate the VM options if we're not running with DartDev.
    vm_options->AddArguments(vm_argv, vm_argc);
  } else if (implicitly_use_dart_dev) {
    // If we're using DartDev implicitly (e.g., dart --observe foo.dart), we
    // want to forward all the VM arguments to the spawned process to ensure
    // the program behaves as the user expects even though we're running
    // through DartDev without their knowledge.
    dart_options->AddArguments(const_cast<const char**>(argv + 1),
                               script_or_cmd_index - 1);
  } else if (i > 1) {
    // If we're running with DartDev, we're going to ignore the VM options for
    // this VM instance and print a warning.

    int num_experiment_flags = 0;
    if (!enabled_experiments_.is_empty()) {
      for (intptr_t j = 1; j < script_or_cmd_index; ++j) {
        if ((strstr(argv[j], kEnableExperiment1) != nullptr) ||
            (strstr(argv[j], kEnableExperiment2) != nullptr)) {
          ++num_experiment_flags;
        }
      }
    }
    // +2 since --enable-dart-dev needs to be passed to enable DartDev.
    if (num_experiment_flags + 2 != script_or_cmd_index) {
      Syslog::PrintErr(
          "Warning: The following flags were passed as VM options and are "
          "being "
          "ignored:\n\n");
      for (int j = 1; j < script_or_cmd_index; ++j) {
        if ((strstr(argv[j], kEnableExperiment1) == nullptr) &&
            (strstr(argv[j], kEnableExperiment2) == nullptr)) {
          Syslog::PrintErr("  %s\n", argv[j]);
        }
      }
      Syslog::PrintErr(
          "\nThese flags should be passed after the dart command (e.g., 'dart "
          "run --enable-asserts foo.dart' instead of 'dart --enable-asserts "
          "run "
          "foo.dart').\n\n");
    }
  }

  if (!enabled_experiments_.is_empty() &&
      !(Options::disable_dart_dev() || run_script)) {
    intptr_t num_experiments = enabled_experiments_.length();
    int option_size = strlen(kEnableExperiment1) + 1;
    for (intptr_t i = 0; i < num_experiments; ++i) {
      const char* flag = enabled_experiments_.At(i);
      option_size += strlen(flag);
      if (i + 1 != num_experiments) {
        // Account for comma if there's more experiments to add.
        ++option_size;
      }
    }
    // Make room for null terminator
    ++option_size;

    char* enabled_experiments_arg = new char[option_size];
    int offset = snprintf(enabled_experiments_arg, option_size,
                          "%s=", kEnableExperiment1);
    for (intptr_t i = 0; i < num_experiments; ++i) {
      const char* flag = enabled_experiments_.At(i);
      const char* kFormat = (i + 1 != num_experiments) ? "%s," : "%s";
      offset += snprintf(enabled_experiments_arg + offset, option_size - offset,
                         kFormat, flag);
      ASSERT(offset < option_size);
    }
    dart_options->AddArgument(enabled_experiments_arg);
  }

  // Parse out options to be passed to dart main.
  while (i < argc) {
    dart_options->AddArgument(argv[i]);
    i++;
  }

  // Verify consistency of arguments.

  // snapshot_depfile is an alias for depfile. Passing them both is an error.
  if ((snapshot_deps_filename_ != NULL) && (depfile_ != NULL)) {
    Syslog::PrintErr("Specify only one of --depfile and --snapshot_depfile\n");
    return -1;
  }
  if (snapshot_deps_filename_ != NULL) {
    depfile_ = snapshot_deps_filename_;
    snapshot_deps_filename_ = NULL;
  }

  if ((packages_file_ != NULL) && (strlen(packages_file_) == 0)) {
    Syslog::PrintErr("Empty package file name specified.\n");
    return -1;
  }
  if ((gen_snapshot_kind_ != kNone) && (snapshot_filename_ == NULL)) {
    Syslog::PrintErr(
        "Generating a snapshot requires a filename (--snapshot).\n");
    return -1;
  }
  if ((gen_snapshot_kind_ == kNone) && (depfile_ != NULL) &&
      (snapshot_filename_ == NULL) && (depfile_output_filename_ == NULL)) {
    Syslog::PrintErr(
        "Generating a depfile requires an output filename"
        " (--depfile-output-filename or --snapshot).\n");
    return -1;
  }
  if ((gen_snapshot_kind_ != kNone) && vm_run_app_snapshot) {
    Syslog::PrintErr(
        "Specifying an option to generate a snapshot and"
        " run using a snapshot is invalid.\n");
    return -1;
  }

  // If --snapshot is given without --snapshot-kind, default to script snapshot.
  if ((snapshot_filename_ != NULL) && (gen_snapshot_kind_ == kNone)) {
    gen_snapshot_kind_ = kKernel;
  }

  return 0;
}

}  // namespace bin
}  // namespace dart
