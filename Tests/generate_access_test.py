# -*- coding: utf-8 -*-
"""
Generates a binary .class file for testing runtime access control (JVMS §5.4.4).
This test attempts to access restricted members, demonstrating where runtime
access checks (and IllegalAccessError) should be enforced.
"""

import struct


def create_method(access_flags, name_index, descriptor_index, code):
    code_attribute = struct.pack('>HHI', 1, 1, len(code))
    code_attribute += code
    code_attribute += struct.pack('>HH', 0, 0)

    method = struct.pack('>HHHH', access_flags, name_index, descriptor_index, 1)
    method += struct.pack('>HI', 11, len(code_attribute))
    method += code_attribute
    return method


def write_class_file(filename="AccessTest.class"):
    bytecode = b'\xca\xfe\xba\xbe' # Magic
    bytecode += struct.pack('>HH', 0, 51) # Minor=0, Major=51 (Java 7)
    
    constant_pool = [
        b'',
        b'\x07\x00\x02', # #1 Class #2 (java/lang/Object)
        b'\x01\x00\x10java/lang/Object', # #2 Utf8
        b'\x07\x00\x04', # #3 Class #4 (AccessTest)
        b'\x01\x00\nAccessTest', # #4 Utf8
        b'\x01\x00\x06<init>', # #5 Utf8
        b'\x01\x00\x03()V', # #6 Utf8
        b'\x0c\x00\x05\x00\x06', # #7 NameAndType <init>:()V
        b'\x0a\x00\x01\x00\x07', # #8 Methodref java/lang/Object."<init>":()V
        b'\x01\x00\x04main', # #9 Utf8
        b'\x01\x00\x16([Ljava/lang/String;)V', # #10 Utf8
        b'\x01\x00\x04Code', # #11 Utf8
    ]
    
    bytecode += struct.pack('>H', len(constant_pool))
    for entry in constant_pool[1:]:
        bytecode += entry
        
    # Access flags: ACC_PUBLIC | ACC_SUPER (0x0021)
    bytecode += struct.pack('>H', 0x0021)
    # This class: #3
    bytecode += struct.pack('>H', 3)
    # Super class: #1
    bytecode += struct.pack('>H', 1)
    # Interfaces count: 0
    bytecode += struct.pack('>H', 0)
    # Fields count: 0
    bytecode += struct.pack('>H', 0)
    
    methods = [
        create_method(0x0001, 5, 6, b'\x2a\xb7\x00\x08\xb1'),
        create_method(0x0009, 9, 10, b'\xb1'),
    ]
    
    bytecode += struct.pack('>H', len(methods))
    for m in methods:
        bytecode += m
        
    # Attributes count: 0
    bytecode += struct.pack('>H', 0)
    
    with open(filename, "wb") as f:
        f.write(bytecode)
        
    print(f"Successfully generated {filename} (Access control test specification)")

if __name__ == '__main__':
    write_class_file()
