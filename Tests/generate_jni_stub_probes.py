import re
import sys
from pathlib import Path


tests_dir = Path(__file__).resolve().parent
header_path = tests_dir.parent / "JVMX2" / "include" / "jni_external.h"
implementation_path = tests_dir.parent / "JVMX2" / "jni_internal.cpp"
test_source_path = tests_dir / "TestJNIEntrypoints.cpp"
output_path = Path(sys.argv[1]) if len(sys.argv) > 1 else tests_dir / "TestJNIStubProbes.inc"

header = header_path.read_text(encoding="utf-8-sig")
implementation = implementation_path.read_text(encoding="utf-8-sig")
test_source = test_source_path.read_text(encoding="utf-8-sig")

table_start = header.index("  struct JNIEnv\n")
table_end = header.index("\n  };", table_start)
table = header[table_start:table_end]
entries = {
    match.group(1): match.group(2)
    for match in re.finditer(r"\*\s*(\w+)\s*\)\s*\((.*?)\)\s*;", table, re.S)
}
entries = {name: args for name, args in entries.items() if not name.startswith("pReserved")}

stub_names = set(re.findall(
    r"JNIEnvInternal::(\w+)\s*\([^)]*\)\s*\{\s*throw NotImplementedException",
    implementation,
    re.S,
))
stub_names.intersection_update(entries)
tested_names = re.findall(r'RunJniTest\(env,\s*"(\w+)"', test_source)

duplicates = sorted(name for name in set(tested_names) if tested_names.count(name) != 1)
overlap = sorted(set(tested_names).intersection(stub_names))
missing = sorted(set(entries).difference(stub_names).difference(tested_names))
unknown = sorted(set(tested_names).difference(entries))
if duplicates or overlap or missing or unknown:
    raise SystemExit(
        "JNI entry test coverage mismatch: "
        f"duplicates={duplicates}, overlap={overlap}, missing={missing}, unknown={unknown}"
    )


def default_argument(argument):
    argument = argument.strip()
    if argument == "...":
        return None

    match = re.search(r"([A-Za-z_]\w*)$", argument)
    if match is None:
        raise ValueError(f"Could not parse JNI argument: {argument}")

    argument_type = argument[:match.start()].strip()
    if "*" in argument_type or argument_type == "va_list":
        return "nullptr"
    return "0"


lines = []
for name in sorted(stub_names):
    arguments = [argument.strip() for argument in entries[name].split(",")]
    arguments = arguments[1:]
    values = [value for value in (default_argument(argument) for argument in arguments)
              if value is not None]
    call = f"env->{name}(env"
    if values:
        call += ", " + ", ".join(values)
    call += ")"
    lines.extend([
        f"    if (nullptr == env->{name}) {{",
        f'      RunJniTest(env, "{name}", totalTests, failedTests, [&]() {{ return false; }});',
        "    } else {",
        f'    RunJniTest(env, "{name}", totalTests, failedTests, [&]() {{',
        f"      {call};",
        "      return true;",
        "    });",
        "    }",
        "",
    ])

output_path.write_text("\n".join(lines), encoding="utf-8")
print(f"Generated probes for {len(stub_names)} unimplemented JNI entries.")
