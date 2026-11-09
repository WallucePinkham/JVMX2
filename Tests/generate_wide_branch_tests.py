import os
import struct
import subprocess
import tempfile


TEST_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
JAVAC = r"C:\Program Files\Java\jdk1.7.0_80\bin\javac.exe"


def compile_goto_w_test():
    source_lines = [
        "public class TestWideBranches {",
        "  public static int testGotoWide(boolean executeBlock) {",
        "    int value = 0;",
        "    if (executeBlock) {",
    ]
    source_lines.extend(["      value++;"] * 12000)
    source_lines.extend([
        "    }",
        "    return value;",
        "  }",
    ])
    source_lines.extend([
        "  public static int testGotoWideBackward() {",
        "    int value = 0;",
        "    do {",
    ])
    source_lines.extend(["      value++;"] * 12000)
    source_lines.extend([
        "    } while (value < 24000);",
        "    return value;",
        "  }",
        "}",
    ])

    with tempfile.TemporaryDirectory() as temporary_directory:
        source_path = os.path.join(temporary_directory, "TestWideBranches.java")
        with open(source_path, "w") as source_file:
            source_file.write("\n".join(source_lines))
            source_file.write("\n")

        subprocess.check_call([
            JAVAC,
            "-d",
            TEST_DIRECTORY,
            source_path,
        ])


def write_jsr_w_test():
    # JDK 7 no longer emits jsr_w, so construct a minimal Java 5 class containing it.
    constant_pool = b"".join([
        b"\x01" + struct.pack(">H", len(b"TestWideSubroutine")) + b"TestWideSubroutine",
        b"\x07" + struct.pack(">H", 1),
        b"\x01" + struct.pack(">H", len(b"java/lang/Object")) + b"java/lang/Object",
        b"\x07" + struct.pack(">H", 3),
        b"\x01" + struct.pack(">H", len(b"testWideJsr")) + b"testWideJsr",
        b"\x01" + struct.pack(">H", len(b"()I")) + b"()I",
        b"\x01" + struct.pack(">H", len(b"Code")) + b"Code",
    ])
    code = b"\xc9\x00\x00\x00\x08\x10\x2a\xac\x4b\xa9\x00"
    code_attribute = struct.pack(">HHI", 1, 1, len(code))
    code_attribute += code
    code_attribute += struct.pack(">HH", 0, 0)

    class_file = struct.pack(">IHHH", 0xCAFEBABE, 0, 49, 8)
    class_file += constant_pool
    class_file += struct.pack(">HHHHHH", 0x0021, 2, 4, 0, 0, 1)
    class_file += struct.pack(">HHHH", 0x0009, 5, 6, 1)
    class_file += struct.pack(">HI", 7, len(code_attribute))
    class_file += code_attribute
    class_file += struct.pack(">H", 0)

    class_path = os.path.join(TEST_DIRECTORY, "TestWideSubroutine.class")
    with open(class_path, "wb") as class_file_output:
        class_file_output.write(class_file)


compile_goto_w_test()
write_jsr_w_test()
