/***************************************************************************
 *   Copyright © 2005-2011 by Gabriele Svelto                              *
 *   gabriele.svelto@gmail.com                                             *
 *                                                                         *
 *   This file is part of Jelatine.                                        *
 *                                                                         *
 *   Jelatine is free software: you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Jelatine is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with Jelatine.  If not, see <http://www.gnu.org/licenses/>.     *
 ***************************************************************************/

/** \file opcodes.h
 * Java Bytecode opcodes definitions */

/** \def JELATINE_OPCODES_H
 * opcodes.h inclusion macro */

#ifndef JELATINE_OPCODES_H
#   define JELATINE_OPCODES_H (1)

#include "wrappers.h"

/** Defines the array types used by the NEWARRAY opcode */

enum array_type_t {
    T_BOOLEAN = 4, ///< Array of booleans
    T_CHAR = 5, ///< Array of chars
#if JEL_FP_SUPPORT
    T_FLOAT = 6, ///< Array of floats
    T_DOUBLE = 7, ///< Array of doubles
#endif // JEL_FP_SUPPORT
    T_BYTE = 8, ///< Array of bytes
    T_SHORT = 9, ///< Array of shorts
    T_INT = 10, ///< Array of integers
    T_LONG = 11 ///< Array of longs
};

/** Typedef for enum array_type_t */
typedef enum array_type_t array_type_t;

/** Internal opcodes definition */

enum internal_opcode_t {
    NOP = 0, ///< No operation
    ACONST_NULL = 1, ///< Push null
    ICONST_M1 = 2, ///< Push integer constant -1
    ICONST_0 = 3, ///< Push integer constant 0
    ICONST_1 = 4, ///< Push integer constant 1
    ICONST_2 = 5, ///< Push integer constant 2
    ICONST_3 = 6, ///< Push integer constant 3
    ICONST_4 = 7, ///< Push integer constant 4
    ICONST_5 = 8, ///< Push integer constant 5
    LCONST_0 = 9, ///< Push long constant 0
    LCONST_1 = 10, ///< Push long constant 1
    FCONST_0 = 11, ///< Push float constant 0.0
    FCONST_1 = 12, ///< Push float constant 1.0
    FCONST_2 = 13, ///< Push float constant 2.0
    DCONST_0 = 14, ///< Push double constant 0.0
    DCONST_1 = 15, ///< Push double constant 1.0
    BIPUSH = 16, ///< Push a byte sign-extended to an integer
    SIPUSH = 17, ///< Push a short sign-extended to an integer
    LDC = 18, ///< Push item from runtime constant pool using a 1-byte index
    LDC_W = 19, ///< Push item from runtime constant pool using a 2-byte index
    LDC2_W = 20, ///< Push double-word item from runtime constant pool
    ILOAD = 21, ///< Load int from local variable
    LLOAD = 22, ///< Load long from local variable
    FLOAD = 23, ///< Load float from local variable
    DLOAD = 24, ///< Load double from local variable
    ALOAD = 25, ///< Load reference from local variable
    ILOAD_0 = 26, ///< Load int from the first local variable
    ILOAD_1 = 27, ///< Load int from the second local variable
    ILOAD_2 = 28, ///< Load int from the third local variable
    ILOAD_3 = 29, ///< Load int from the fourth local variable
    LLOAD_0 = 30, ///< Load long from the first local variable
    LLOAD_1 = 31, ///< Load long from the second local variable
    LLOAD_2 = 32, ///< Load long from the third local variable
    LLOAD_3 = 33, ///< Load long from the fourth local variable
    FLOAD_0 = 34, ///< Load float from the first local variable
    FLOAD_1 = 35, ///< Load float from the second local variable
    FLOAD_2 = 36, ///< Load float from the third local variable
    FLOAD_3 = 37, ///< Load float from the fourth local variable
    DLOAD_0 = 38, ///< Load double from the first local variable
    DLOAD_1 = 39, ///< Load double from the second local variable
    DLOAD_2 = 40, ///< Load double from the third local variable
    DLOAD_3 = 41, ///< Load double from the fourth local variable
    ALOAD_0 = 42, ///< Load reference from the first local variable
    ALOAD_1 = 43, ///< Load reference from the second local variable
    ALOAD_2 = 44, ///< Load reference from the third local variable
    ALOAD_3 = 45, ///< Load reference from the fourth local variable
    IALOAD = 46, ///< Load int from array
    LALOAD = 47, ///< Load long from array
    FALOAD = 48, ///< Load float from array
    DALOAD = 49, ///< Load double from array
    AALOAD = 50, ///< Load reference from array
    BALOAD = 51, ///< Load byte or boolean from array
    CALOAD = 52, ///< Load char from array
    SALOAD = 53, ///< Load short from array
    ISTORE = 54, ///< Store int into local variable
    LSTORE = 55, ///< Store long into local variable
    FSTORE = 56, ///< Store float into local variable
    DSTORE = 57, ///< Store double into local variable
    ASTORE = 58, ///< Store reference into local variable
    ISTORE_0 = 59, ///< Store int into the first local variable
    ISTORE_1 = 60, ///< Store int into the second local variable
    ISTORE_2 = 61, ///< Store int into the third local variable
    ISTORE_3 = 62, ///< Store int into the fourth local variable
    LSTORE_0 = 63, ///< Store long into the first local variable
    LSTORE_1 = 64, ///< Store long into the second local variable
    LSTORE_2 = 65, ///< Store long into the third local variable
    LSTORE_3 = 66, ///< Store long into the fourth local variable
    FSTORE_0 = 67, ///< Store float into the first local variable
    FSTORE_1 = 68, ///< Store float into the second local variable
    FSTORE_2 = 69, ///< Store float into the third local variable
    FSTORE_3 = 70, ///< Store float into the fourth local variable
    DSTORE_0 = 71, ///< Store double into the first local variable
    DSTORE_1 = 72, ///< Store double into the second local variable
    DSTORE_2 = 73, ///< Store double into the third local variable
    DSTORE_3 = 74, ///< Store double into the fourth local variable
    ASTORE_0 = 75, ///< Store reference into the first local variable
    ASTORE_1 = 76, ///< Store reference into the second local variable
    ASTORE_2 = 77, ///< Store reference into the third local variable
    ASTORE_3 = 78, ///< Store reference into the fourth local variable
    IASTORE = 79, ///< Store into int array
    LASTORE = 80, ///< Store into long array
    FASTORE = 81, ///< Store into float array
    DASTORE = 82, ///< Store into double array
    AASTORE = 83, ///< Store into reference array
    BASTORE = 84, ///< Store into byte array
    CASTORE = 85, ///< Store into char array
    SASTORE = 86, ///< Store into short array
    POP = 87, ///< Pop the top operand stack value
    POP2 = 88, ///< Pop the one or two top operand stack values
    DUP = 89, ///< Duplicate the top operand stack value
    DUP_X1 = 90, ///< Duplicate the top operand stack value and insert two values down
    DUP_X2 = 91, ///< Duplicate the top operand stack value and insert two or three values down
    DUP2 = 92, ///< Duplicate the top operand stack value and insert two or three values down*/
    DUP2_X1 = 93, ///< Duplicate the top one or two operand stack values and insert two or three values down*/
    DUP2_X2 = 94, ///< Duplicate the top one or two operand stack values and insert two, three, or four values down
    SWAP = 95, ///< Swap the top two operand stack values
    IADD = 96, ///< Add int
    LADD = 97, ///< Add long
    FADD = 98, ///< Add float
    DADD = 99, ///< Add double
    ISUB = 100, ///< Substract int
    LSUB = 101, ///< Substract long
    FSUB = 102, ///< Substract float
    DSUB = 103, ///< Sustract double
    IMUL = 104, ///< Multiply int
    LMUL = 105, ///< Multiply long
    FMUL = 106, ///< Multiply float
    DMUL = 107, ///< Multiply double
    IDIV = 108, ///< Divide int
    LDIV = 109, ///< Divide long
    FDIV = 110, ///< Divide float
    DDIV = 111, ///< Divide double
    IREM = 112, ///< Remainder int
    LREM = 113, ///< Remainder long
    FREM = 114, ///< Remainder float
    DREM = 115, ///< Remainder double
    INEG = 116, ///< Negate int
    LNEG = 117, ///< Negate long
    FNEG = 118, ///< Negate float
    DNEG = 119, ///< Negate double
    ISHL = 120, ///< Shift left int
    LSHL = 121, ///< Shift left long
    ISHR = 122, ///< Arithmetic shift right int
    LSHR = 123, ///< Arithmetic shift right long
    IUSHR = 124, ///< Logical shift right int
    LUSHR = 125, ///< Logical shift right long
    IAND = 126, ///< Boolean AND int
    LAND = 127, ///< Boolean AND long
    IOR = 128, ///< Boolean OR int
    LOR = 129, ///< Boolean OR long
    IXOR = 130, ///< Boolean XOR int
    LXOR = 131, ///< Boolean XOR long
    IINC = 132, ///< Increment local variable by constant
    I2L = 133, ///< Convert int to long
    I2F = 134, ///< Convert int to float
    I2D = 135, ///< Convert int to double
    L2I = 136, ///< Convert long to int
    L2F = 137, ///< Convert long to float
    L2D = 138, ///< Convert long to double
    F2I = 139, ///< Convert float to int
    F2L = 140, ///< Convert float to long
    F2D = 141, ///< Convert float to double
    D2I = 142, ///< Convert double to int
    D2L = 143, ///< Convert double to long
    D2F = 144, ///< Convert double to float
    I2B = 145, ///< Convert int to byte
    I2C = 146, ///< Convert int to char
    I2S = 147, ///< Convert int to short
    LCMP = 148, ///< Long compare
    FCMPL = 149, ///< Float compare NaN negative
    FCMPG = 150, ///< Float compare NaN positive
    DCMPL = 151, ///< Double compare NaN negative
    DCMPG = 152, ///< Double compare NaN positive
    IFEQ = 153, ///< Branch if equal to zero
    IFNE = 154, ///< Branch if not equal to zero
    IFLT = 155, ///< Branch if lower than zero
    IFGE = 156, ///< Branch if greater or equal to zero
    IFGT = 157, ///< Branch if greater than zero
    IFLE = 158, ///< Branch if lower or equal to zero
    IF_ICMPEQ = 159, ///< Branch if equal
    IF_ICMPNE = 160, ///< Branch if not equal
    IF_ICMPLT = 161, ///< Branch if lower than
    IF_ICMPGE = 162, ///< Branch if greater or equal
    IF_ICMPGT = 163, ///< Branch if greater than
    IF_ICMPLE = 164, ///< Branch if lower or equal
    IF_ACMPEQ = 165, ///< Branch if reference equal
    IF_ACMPNE = 166, ///< Branch if reference not equal
    GOTO = 167, ///< Branch always
    LDC_REF = 168, ///< Load a string or class constant reference
    LDC_W_REF = 169, ///< Load a string or class constant reference
    TABLESWITCH = 170, ///< Access jump table by index and jump
    LOOKUPSWITCH = 171, ///< Access jump table by key match and jump
    IRETURN = 172, ///< Return int from method
    LRETURN = 173, ///< Return long from method
    FRETURN = 174, ///< Return float from method
    DRETURN = 175, ///< Return double from method
    ARETURN = 176, ///< Return reference from method
    RETURN = 177, ///< Return void from method
    GETSTATIC_PRELINK = 178, ///< Prelink get static field from class
    PUTSTATIC_PRELINK = 179, ///< Prelink set static field in class
    GETFIELD_PRELINK = 180, ///< Prelink fetch field from object
    PUTFIELD_PRELINK = 181, ///< Prelink set field in object
    INVOKEVIRTUAL = 182, ///< Invoke virtual method
    INVOKESPECIAL = 183, ///< Invoke special instance method
    INVOKESTATIC = 184, ///< Invoke a (class) static method
    INVOKEINTERFACE = 185, ///< Invoke interface method
    INVOKESUPER = 186, ///< Implements the super() call semantics of INVOKESPECIAL
    NEW = 187, ///< Create new object
    NEWARRAY = 188, ///< Create new array
    ANEWARRAY = 189, ///< Create new array of reference
    ARRAYLENGTH = 190, ///< Get length of array
    ATHROW = 191, ///< Throw exception or error
    CHECKCAST = 192, ///< Check whethever object is of given type
    INSTANCEOF = 193, ///< Determine if object is of given type
    MONITORENTER = 194, ///< Enter monitor for object
    MONITOREXIT = 195, ///< Monitor exit
    WIDE = 196, ///< Extend local variable index by additional bytes
    MULTIANEWARRAY = 197, ///< Create new multidimensional array
    IFNULL = 198, ///< Branch if reference is null
    IFNONNULL = 199, ///< Branch if reference is not null
    GOTO_W = 200, ///< Branch always wide index
    GETSTATIC_BYTE = 201, ///< Gets a byte-typed static field
    GETSTATIC_BOOL = 201, ///< Gets a bool-typed static field (same as GETSTATIC_BYTE)
    GETSTATIC_CHAR = 202, ///< Gets a char-typed static field
    GETSTATIC_SHORT = 203, ///< Gets a short-typed static field
    GETSTATIC_INT = 204, ///< Gets an int-typed static field
    GETSTATIC_FLOAT = 205, ///< Gets a float-typed static field
    GETSTATIC_LONG = 206, ///< Gets a long-typed static field
    GETSTATIC_DOUBLE = 207, ///< Gets a double-typed static field
    GETSTATIC_REFERENCE = 208, ///< Gets a reference-typed static field
    PUTSTATIC_BYTE = 209, ///< Sets a byte-typed static field
    PUTSTATIC_BOOL = 210, ///< Sets a bool-typed static field
    PUTSTATIC_CHAR = 211, ///< Sets a char-typed static field
    PUTSTATIC_SHORT = 211, ///< Sets a short-typed static field (same as PUTSTATIC_CHAR)
    PUTSTATIC_INT = 212, ///< Sets an int-typed static field
    PUTSTATIC_FLOAT = 213, ///< Sets a float-typed static field
    PUTSTATIC_LONG = 214, ///< Sets a long-typed static field
    PUTSTATIC_DOUBLE = 215, ///< Sets a double-typed static field
    PUTSTATIC_REFERENCE = 216, ///< Sets a reference-typed static field
    GETFIELD_BYTE = 217, ///< Gets a byte-typed field
    GETFIELD_BOOL = 218, ///< Gets a bool-typed field
    GETFIELD_CHAR = 219, ///< Gets a char-typed field
    GETFIELD_SHORT = 220, ///< Gets a short-typed field
    GETFIELD_INT = 221, ///< Gets an int-typed field
    GETFIELD_FLOAT = 222, ///< Gets a float-typed field
    GETFIELD_LONG = 223, ///< Gets a long-typed field
    GETFIELD_DOUBLE = 224, ///< Gets a double-typed field
    GETFIELD_REFERENCE = 225, ///< Gets a reference-typed field
    PUTFIELD_BYTE = 226, ///< Sets a byte-typed field
    PUTFIELD_BOOL = 227, ///< Sets a bool-typed field
    PUTFIELD_CHAR = 228, ///< Sets a char-typed field
    PUTFIELD_SHORT = 228, ///< Sets a short-typed field (same as PUTFIELD_CHAR)
    PUTFIELD_INT = 229, ///< Sets an int-typed field
    PUTFIELD_FLOAT = 230, ///< Sets a float-typed field
    PUTFIELD_LONG = 231, ///< Sets a long-typed field
    PUTFIELD_DOUBLE = 232, ///< Sets a double-typed field
    PUTFIELD_REFERENCE = 233, ///< Sets a reference-typedef field
    INVOKEVIRTUAL_PRELINK = 234, ///< Link a virtual method
    INVOKESPECIAL_PRELINK = 235, ///< Link a special method
    INVOKESTATIC_PRELINK = 236, ///< Link a static method
    INVOKEINTERFACE_PRELINK = 237, ///< Link an interface method
    NEW_PRELINK = 238, ///< Link a class to be created
    NEWARRAY_PRELINK = 239, ///< Links an array class to be created
    ANEWARRAY_PRELINK = 240, ///< Links an array class to be created
    CHECKCAST_PRELINK = 241, ///< Links classes requested by a checkcast
    INSTANCEOF_PRELINK = 242, ///< Links the class requested by an instanceof
    MULTIANEWARRAY_PRELINK = 243, ///< Links classes requested by a MULTIANEWARRAY
    MONITORENTER_SPECIAL = 244, ///< Modified MONITORENTER for synchronized virtual methods
    MONITORENTER_SPECIAL_STATIC = 245, ///< Modified MONITORENTER for synchronized static methods
    IRETURN_MONITOREXIT = 246, ///< Returns an int and releases the current monitor
    LRETURN_MONITOREXIT = 247, ///< Returns a long and releases the current monitor
    FRETURN_MONITOREXIT = 248, ///< Returns a float and releases the current monitor
    DRETURN_MONITOREXIT = 249, ///< Returns a double and releases the current monitor
    ARETURN_MONITOREXIT = 250, ///< Returns a reference and releases the current monitor
    RETURN_MONITOREXIT = 251, ///< Returns and releases the current monitor
    NEW_FINALIZER = 252, ///< Same as new but for finalizable objects
    LDC_PRELINK = 253, ///< Links a load constant opcode
    LDC_W_PRELINK = 254, ///< Links a load constant wide opcode
    // The following opcodes are called using the WIDE prefix
    METHOD_LOAD = 201, ///< Loads a method code
    METHOD_ABSTRACT = 202, ///< Dummy opcode for triggering abstract method errors
    INVOKE_NATIVE = 203, ///< Invoke a native method
    HALT = 204 ///< Halt the exection of the interpreter
};

/** Typedef for enum internal_opcode_t */
typedef enum internal_opcode_t internal_opcode_t;

/** Java opcodes definition */

enum java_opcode_t {
    JAVA_NOP = 0, ///< No operation
    JAVA_ACONST_NULL = 1, ///< Push null
    JAVA_ICONST_M1 = 2, ///< Push integer constant -1
    JAVA_ICONST_0 = 3, ///< Push integer constant 0
    JAVA_ICONST_1 = 4, ///< Push integer constant 1
    JAVA_ICONST_2 = 5, ///< Push integer constant 2
    JAVA_ICONST_3 = 6, ///< Push integer constant 3
    JAVA_ICONST_4 = 7, ///< Push integer constant 4
    JAVA_ICONST_5 = 8, ///< Push integer constant 5
    JAVA_LCONST_0 = 9, ///< Push long constant 0
    JAVA_LCONST_1 = 10, ///< Push long constant 1
    JAVA_FCONST_0 = 11, ///< Push float constant 0.0
    JAVA_FCONST_1 = 12, ///< Push float constant 1.0
    JAVA_FCONST_2 = 13, ///< Push float constant 2.0
    JAVA_DCONST_0 = 14, ///< Push double constant 0.0
    JAVA_DCONST_1 = 15, ///< Push double constant 1.0
    JAVA_BIPUSH = 16, ///< Push a byte sign-extended to an integer
    JAVA_SIPUSH = 17, ///< Push a short sign-extended to an integer
    JAVA_LDC = 18, ///< Push item from runtime constant pool using a 1-byte index
    JAVA_LDC_W = 19, ///< Push item from runtime constant pool using a 2-byte index
    JAVA_LDC2_W = 20, ///< Push double-word item from runtime constant pool
    JAVA_ILOAD = 21, ///< Load int from local variable
    JAVA_LLOAD = 22, ///< Load long from local variable
    JAVA_FLOAD = 23, ///< Load float from local variable
    JAVA_DLOAD = 24, ///< Load double from local variable
    JAVA_ALOAD = 25, ///< Load reference from local variable
    JAVA_ILOAD_0 = 26, ///< Load int from the first local variable
    JAVA_ILOAD_1 = 27, ///< Load int from the second local variable
    JAVA_ILOAD_2 = 28, ///< Load int from the third local variable
    JAVA_ILOAD_3 = 29, ///< Load int from the fourth local variable
    JAVA_LLOAD_0 = 30, ///< Load long from the first local variable
    JAVA_LLOAD_1 = 31, ///< Load long from the second local variable
    JAVA_LLOAD_2 = 32, ///< Load long from the third local variable
    JAVA_LLOAD_3 = 33, ///< Load long from the fourth local variable
    JAVA_FLOAD_0 = 34, ///< Load float from the first local variable
    JAVA_FLOAD_1 = 35, ///< Load float from the second local variable
    JAVA_FLOAD_2 = 36, ///< Load float from the third local variable
    JAVA_FLOAD_3 = 37, ///< Load float from the fourth local variable
    JAVA_DLOAD_0 = 38, ///< Load double from the first local variable
    JAVA_DLOAD_1 = 39, ///< Load double from the second local variable
    JAVA_DLOAD_2 = 40, ///< Load double from the third local variable
    JAVA_DLOAD_3 = 41, ///< Load double from the fourth local variable
    JAVA_ALOAD_0 = 42, ///< Load reference from the first local variable
    JAVA_ALOAD_1 = 43, ///< Load reference from the second local variable
    JAVA_ALOAD_2 = 44, ///< Load reference from the third local variable
    JAVA_ALOAD_3 = 45, ///< Load reference from the fourth local variable
    JAVA_IALOAD = 46, ///< Load int from array
    JAVA_LALOAD = 47, ///< Load long from array
    JAVA_FALOAD = 48, ///< Load float from array
    JAVA_DALOAD = 49, ///< Load double from array
    JAVA_AALOAD = 50, ///< Load reference from array
    JAVA_BALOAD = 51, ///< Load byte or boolean from array
    JAVA_CALOAD = 52, ///< Load char from array
    JAVA_SALOAD = 53, ///< Load short from array
    JAVA_ISTORE = 54, ///< Store int into local variable
    JAVA_LSTORE = 55, ///< Store long into local variable
    JAVA_FSTORE = 56, ///< Store float into local variable
    JAVA_DSTORE = 57, ///< Store double into local variable
    JAVA_ASTORE = 58, ///< Store reference into local variable
    JAVA_ISTORE_0 = 59, ///< Store int into the first local variable
    JAVA_ISTORE_1 = 60, ///< Store int into the second local variable
    JAVA_ISTORE_2 = 61, ///< Store int into the third local variable
    JAVA_ISTORE_3 = 62, ///< Store int into the fourth local variable
    JAVA_LSTORE_0 = 63, ///< Store long into the first local variable
    JAVA_LSTORE_1 = 64, ///< Store long into the second local variable
    JAVA_LSTORE_2 = 65, ///< Store long into the third local variable
    JAVA_LSTORE_3 = 66, ///< Store long into the fourth local variable
    JAVA_FSTORE_0 = 67, ///< Store float into the first local variable
    JAVA_FSTORE_1 = 68, ///< Store float into the second local variable
    JAVA_FSTORE_2 = 69, ///< Store float into the third local variable
    JAVA_FSTORE_3 = 70, ///< Store float into the fourth local variable
    JAVA_DSTORE_0 = 71, ///< Store double into the first local variable
    JAVA_DSTORE_1 = 72, ///< Store double into the second local variable
    JAVA_DSTORE_2 = 73, ///< Store double into the third local variable
    JAVA_DSTORE_3 = 74, ///< Store double into the fourth local variable
    JAVA_ASTORE_0 = 75, ///< Store reference into the first local variable
    JAVA_ASTORE_1 = 76, ///< Store reference into the second local variable
    JAVA_ASTORE_2 = 77, ///< Store reference into the third local variable
    JAVA_ASTORE_3 = 78, ///< Store reference into the fourth local variable
    JAVA_IASTORE = 79, ///< Store into int array
    JAVA_LASTORE = 80, ///< Store into long array
    JAVA_FASTORE = 81, ///< Store into float array
    JAVA_DASTORE = 82, ///< Store into double array
    JAVA_AASTORE = 83, ///< Store into reference array
    JAVA_BASTORE = 84, ///< Store into byte array
    JAVA_CASTORE = 85, ///< Store into char array
    JAVA_SASTORE = 86, ///< Store into short array
    JAVA_POP = 87, ///< Pop the top operand stack value
    JAVA_POP2 = 88, ///< Pop the one or two top operand stack values
    JAVA_DUP = 89, ///< Duplicate the top operand stack value
    JAVA_DUP_X1 = 90, ///< Duplicate the top operand stack value and insert two values down
    JAVA_DUP_X2 = 91, ///< Duplicate the top operand stack value and insert two or three values down
    JAVA_DUP2 = 92, ///< Duplicate the top operand stack value and insert two or three values down*/
    JAVA_DUP2_X1 = 93, ///< Duplicate the top one or two operand stack values and insert two or three values down*/
    JAVA_DUP2_X2 = 94, ///< Duplicate the top one or two operand stack values and insert two, three, or four values down
    JAVA_SWAP = 95, ///< Swap the top two operand stack values
    JAVA_IADD = 96, ///< Add int
    JAVA_LADD = 97, ///< Add long
    JAVA_FADD = 98, ///< Add float
    JAVA_DADD = 99, ///< Add double
    JAVA_ISUB = 100, ///< Substract int
    JAVA_LSUB = 101, ///< Substract long
    JAVA_FSUB = 102, ///< Substract float
    JAVA_DSUB = 103, ///< Sustract double
    JAVA_IMUL = 104, ///< Multiply int
    JAVA_LMUL = 105, ///< Multiply long
    JAVA_FMUL = 106, ///< Multiply float
    JAVA_DMUL = 107, ///< Multiply double
    JAVA_IDIV = 108, ///< Divide int
    JAVA_LDIV = 109, ///< Divide long
    JAVA_FDIV = 110, ///< Divide float
    JAVA_DDIV = 111, ///< Divide double
    JAVA_IREM = 112, ///< Remainder int
    JAVA_LREM = 113, ///< Remainder long
    JAVA_FREM = 114, ///< Remainder float
    JAVA_DREM = 115, ///< Remainder double
    JAVA_INEG = 116, ///< Negate int
    JAVA_LNEG = 117, ///< Negate long
    JAVA_FNEG = 118, ///< Negate float
    JAVA_DNEG = 119, ///< Negate double
    JAVA_ISHL = 120, ///< Shift left int
    JAVA_LSHL = 121, ///< Shift left long
    JAVA_ISHR = 122, ///< Arithmetic shift right int
    JAVA_LSHR = 123, ///< Arithmetic shift right long
    JAVA_IUSHR = 124, ///< Logical shift right int
    JAVA_LUSHR = 125, ///< Logical shift right long
    JAVA_IAND = 126, ///< Boolean AND int
    JAVA_LAND = 127, ///< Boolean AND long
    JAVA_IOR = 128, ///< Boolean OR int
    JAVA_LOR = 129, ///< Boolean OR long
    JAVA_IXOR = 130, ///< Boolean XOR int
    JAVA_LXOR = 131, ///< Boolean XOR long
    JAVA_IINC = 132, ///< Increment local variable by constant
    JAVA_I2L = 133, ///< Convert int to long
    JAVA_I2F = 134, ///< Convert int to float
    JAVA_I2D = 135, ///< Convert int to double
    JAVA_L2I = 136, ///< Convert long to int
    JAVA_L2F = 137, ///< Convert long to float
    JAVA_L2D = 138, ///< Convert long to double
    JAVA_F2I = 139, ///< Convert float to int
    JAVA_F2L = 140, ///< Convert float to long
    JAVA_F2D = 141, ///< Convert float to double
    JAVA_D2I = 142, ///< Convert double to int
    JAVA_D2L = 143, ///< Convert double to long
    JAVA_D2F = 144, ///< Convert double to float
    JAVA_I2B = 145, ///< Convert int to byte
    JAVA_I2C = 146, ///< Convert int to char
    JAVA_I2S = 147, ///< Convert int to short
    JAVA_LCMP = 148, ///< Long compare
    JAVA_FCMPL = 149, ///< Float compare NaN negative
    JAVA_FCMPG = 150, ///< Float compare NaN positive
    JAVA_DCMPL = 151, ///< Double compare NaN negative
    JAVA_DCMPG = 152, ///< Double compare NaN positive
    JAVA_IFEQ = 153, ///< Branch if equal to zero
    JAVA_IFNE = 154, ///< Branch if not equal to zero
    JAVA_IFLT = 155, ///< Branch if lower than zero
    JAVA_IFGE = 156, ///< Branch if greater or equal to zero
    JAVA_IFGT = 157, ///< Branch if greater than zero
    JAVA_IFLE = 158, ///< Branch if lower or equal to zero
    JAVA_IF_ICMPEQ = 159, ///< Branch if equal
    JAVA_IF_ICMPNE = 160, ///< Branch if not equal
    JAVA_IF_ICMPLT = 161, ///< Branch if lower than
    JAVA_IF_ICMPGE = 162, ///< Branch if greater or equal
    JAVA_IF_ICMPGT = 163, ///< Branch if greater than
    JAVA_IF_ICMPLE = 164, ///< Branch if lower or equal
    JAVA_IF_ACMPEQ = 165, ///< Branch if reference equal
    JAVA_IF_ACMPNE = 166, ///< Branch if reference not equal
    JAVA_GOTO = 167, ///< Branch always
    JAVA_JSR = 168, ///< Jump to subroutine
    JAVA_RET = 169, ///< Return from subroutine
    JAVA_TABLESWITCH = 170, ///< Access jump table by index and jump
    JAVA_LOOKUPSWITCH = 171, ///< Access jump table by key match and jump
    JAVA_IRETURN = 172, ///< Return int from method
    JAVA_LRETURN = 173, ///< Return long from method
    JAVA_FRETURN = 174, ///< Return float from method
    JAVA_DRETURN = 175, ///< Return double from method
    JAVA_ARETURN = 176, ///< Return reference from method
    JAVA_RETURN = 177, ///< Return void from method
    JAVA_GETSTATIC = 178, ///< Get static field from class
    JAVA_PUTSTATIC = 179, ///< Set static field in class
    JAVA_GETFIELD = 180, ///< Fetch field from object
    JAVA_PUTFIELD = 181, ///< Set field in object
    JAVA_INVOKEVIRTUAL = 182, ///< Invoke virtual method
    JAVA_INVOKESPECIAL = 183, ///< Invoke special instance method
    JAVA_INVOKESTATIC = 184, ///< Invoke a (class) static method
    JAVA_INVOKEINTERFACE = 185, ///< Invoke interface method
    JAVA_NEW = 187, ///< Create new object
    JAVA_NEWARRAY = 188, ///< Create new array
    JAVA_ANEWARRAY = 189, ///< Create new array of reference
    JAVA_ARRAYLENGTH = 190, ///< Get length of array
    JAVA_ATHROW = 191, ///< Throw exception or error
    JAVA_CHECKCAST = 192, ///< Check whethever object is of given type
    JAVA_INSTANCEOF = 193, ///< Determine if object is of given type
    JAVA_MONITORENTER = 194, ///< Enter monitor for object
    JAVA_MONITOREXIT = 195, ///< Monitor exit
    JAVA_WIDE = 196, ///< Extend local variable index by additional bytes
    JAVA_MULTIANEWARRAY = 197, ///< Create new multidimensional array
    JAVA_IFNULL = 198, ///< Branch if reference is null
    JAVA_IFNONNULL = 199, ///< Branch if reference is not null
    JAVA_GOTO_W = 200, ///< Branch always wide index
    JAVA_JSR_W = 201, ///< Branch to subrouting wide index
};

/** Typedef for enum java_opcode_t */
typedef enum java_opcode_t java_opcode_t;

#endif // !JELATINE_OPCODES_H
