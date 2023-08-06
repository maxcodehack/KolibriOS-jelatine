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

/** \file bytecode.c
 * Bytecode manipulation functions implementation */

#include "wrappers.h"

#include "bytecode.h"
#include "class.h"
#include "constantpool.h"
#include "loader.h"
#include "memory.h"
#include "method.h"
#include "opcodes.h"
#include "util.h"

/**  Translates regular Java bytecode into the internal version used by
 * the VM, bytecode verification could be done here too
 *
 * Note that during bytecode translation constants larger than a single byte are
 * turned to the machines native endian format (for unaligned constants only if
 * the machine supports unaligned accesses). Static constraints in the bytecode
 * are enforced here and partial verification happens too.
 *
 * \param cl A pointer to the class owning the method
 * \param method A pointer to the method to be translated
 * \param code The method code
 * \param handlers The method exception handlers */

void translate_bytecode(class_t *cl, method_t *method, uint8_t *code,
                        exception_handler_t *handlers)
{
    const_pool_t *cp = cl->const_pool;
    uint8_t opcode;
    int32_t i = 0;
    int32_t j = 0;
    int32_t code_length = method_get_code_length(method);
    bool synchronized = method_is_synchronized(method);

    /* Add a special monitor enter opcode at the beginning of the code if the
     * method is synchronized */
    if (synchronized) {
        if (method_is_static(method)) {
            code[0] = MONITORENTER_SPECIAL_STATIC;
        } else {
            code[0] = MONITORENTER_SPECIAL;
        }

        i++;
    }

    while  (i < code_length) {
        opcode = code[i];

        switch (opcode) {
            // Leave the following opcodes unchanged
            case JAVA_NOP: // NOP
            case JAVA_ACONST_NULL: // ACONST_NULL
            case JAVA_ICONST_M1: // ICONST_M1
            case JAVA_ICONST_0: // ICONST_0
            case JAVA_ICONST_1: // ICONST_1
            case JAVA_ICONST_2: // ICONST_2
            case JAVA_ICONST_3: // ICONST_3
            case JAVA_ICONST_4: // ICONST_4
            case JAVA_ICONST_5: // ICONST_5
            case JAVA_LCONST_0: // LCONST_0
            case JAVA_LCONST_1: // LCONST_1
#if JEL_FP_SUPPORT
            case JAVA_FCONST_0: // FCONST_0
            case JAVA_FCONST_1: // FCONST_1
            case JAVA_FCONST_2: // FCONST_2
            case JAVA_DCONST_0: // DCONST_0
            case JAVA_DCONST_1: // DCONST_1
#endif // JEL_FP_SUPPORT
                i++;
                break;

            case JAVA_BIPUSH: // BIPUSH
                i += 2;
                break;

            case JAVA_SIPUSH: // SIPUSH
            {
                // Turn the number in the native endian format
                int16_t temp = (code[i + 1] << 8) | code[i + 2];
                store_int16_un(code + i + 1, temp);
                i += 3;
            }
                break;

            case JAVA_LDC:
            {
                uint8_t tag;

                /* Translate to LDC_STRING if used to load a string reference,
                 * this will also throw an exception if the constant pool index
                 * is out of bounds */
                tag = cp_get_tag(cp, code[i + 1]);

                if ((tag == CONSTANT_String) || (tag == CONSTANT_Class)) {
                    code[i] = LDC_PRELINK;
                } else {
                    code[i] = LDC;
#if JEL_FP_SUPPORT
                    if (tag != CONSTANT_Float && tag != CONSTANT_Integer) {
                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
						c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "LDC instruction constant pool index refers to "
                                "an element which is neither CONSTANT_String "
                                "CONSTANT_Integer nor CONSTANT_Float");
                    }
#else
                    if (tag != CONSTANT_Integer) {
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "LDC instruction constant pool index refers to "
                                "an element which is neither CONSTANT_String "
                                "nor CONSTANT_Integer");
                    }
#endif // JEL_FP_SUPPORT
                }

                i += 2;
            }
                break;

            case JAVA_LDC_W:
            {
                /* Translate to LDC_W_STRING if used to load a string reference,
                 * this will also throw an exception if the constant pool index
                 * is out of bounds */
                uint16_t temp = (code[i + 1] << 8) | code[i + 2];
                uint8_t tag = cp_get_tag(cp, temp);

                if ((tag == CONSTANT_String) || (tag == CONSTANT_Class)) {
                    code[i] = LDC_W_PRELINK;
                } else {
                    code[i] = LDC_W;
                    store_int16_un(code + i + 1, temp);
#if JEL_FP_SUPPORT
                    if (tag != CONSTANT_Float && tag != CONSTANT_Integer) {
					                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "LDC_W instruction constant pool index refers "
                                "to an element which is neither CONSTANT_String"
                                ", CONSTANT_Integer nor CONSTANT_Float");
                    }
#else
                    if (tag != CONSTANT_Integer) {
					                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "LDC_W instruction constant pool index refers "
                                "to an element which is neither CONSTANT_String"
                                " nor CONSTANT_Integer");
                    }
#endif // JEL_FP_SUPPORT
                }

                i += 3;
            }
                break;

            case JAVA_LDC2_W:
            {
                uint16_t temp = (code[i + 1] << 8) | code[i + 2];
                uint8_t tag = cp_get_tag(cp, temp);

#if JEL_FP_SUPPORT
                if (tag != CONSTANT_Double && tag != CONSTANT_Long) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "LDC2_W instruction constant pool index refers to "
                            "an element which is neither CONSTANT_Double nor "
                            "CONSTANT_Long");
                }
#else
                if (tag != CONSTANT_Long) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "LDC2_W instruction constant pool index refers to "
                            "an element which is not CONSTANT_Long");
                }
#endif // JEL_FP_SUPPORT

                store_int16_un(code + i + 1, temp);
                i += 3;
            }
                break;

            case JAVA_ILOAD: // ILOAD
            case JAVA_LLOAD: // LLOAD
#if JEL_FP_SUPPORT
            case JAVA_FLOAD: // FLOAD
            case JAVA_DLOAD: // DLOAD
#endif // JEL_FP_SUPPORT
            case JAVA_ALOAD: // ALOAD
            case JAVA_ISTORE: // ISTORE
            case JAVA_LSTORE: // LSTORE
#if JEL_FP_SUPPORT
            case JAVA_FSTORE: // FSTORE
            case JAVA_DSTORE: // DSTORE
#endif // JEL_FP_SUPPORT
            case JAVA_ASTORE: // ASTORE
            {
                uint8_t temp = code[i + 1];

                if (temp > method->max_locals - 1) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "ILOAD, FLOAD, ALOAD, LLOAD, DLOAD, ISTORE, FSTORE,"
                            " ASTORE, LSTORE or DSTORE accesses a non-existing "
                            "local variable");
                }

                i += 2;
            }
                break;

            case JAVA_ILOAD_0: // ILOAD_0
            case JAVA_LLOAD_0: // LLOAD_0
#if JEL_FP_SUPPORT
            case JAVA_FLOAD_0: // FLOAD_0
            case JAVA_DLOAD_0: // DLOAD_0
#endif // JEL_FP_SUPPORT
            case JAVA_ALOAD_0: // ALOAD_0
            case JAVA_ISTORE_0: // ISTORE_0
            case JAVA_LSTORE_0: // LSTORE_0
#if JEL_FP_SUPPORT
            case JAVA_FSTORE_0: // FSTORE_0
            case JAVA_DSTORE_0: // DSTORE_0
#endif // JEL_FP_SUPPORT
            case JAVA_ASTORE_0: // ASTORE_0
                if (method->max_locals == 0) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "ILOAD_0, FLOAD_0, ALOAD_0, LLOAD_0, DLOAD_0, "
                            "ISTORE_0, LSTORE_0, FSTORE_0, DSTORE_0 or ASTORE_0"
                            " accesses a non-existing local variable");
                }

                i++;
                break;

            case JAVA_ILOAD_1: // ILOAD_1
            case JAVA_LLOAD_1: // LLOAD_1
#if JEL_FP_SUPPORT
            case JAVA_FLOAD_1: // FLOAD_1
            case JAVA_DLOAD_1: // DLOAD_1
#endif // JEL_FP_SUPPORT
            case JAVA_ALOAD_1: // ALOAD_1
            case JAVA_ISTORE_1: // ISTORE_1
            case JAVA_LSTORE_1: // LSTORE_1
#if JEL_FP_SUPPORT
            case JAVA_FSTORE_1: // FSTORE_1
            case JAVA_DSTORE_1: // DSTORE_1
#endif // JEL_FP_SUPPORT
            case JAVA_ASTORE_1: // ASTORE_1
                if (method->max_locals < 2) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "ILOAD_1, FLOAD_1, ALOAD_1, LLOAD_1, DLOAD_1, "
                            "ISTORE_1, LSTORE_1, FSTORE_1, DSTORE_1 or ASTORE_1"
                            " accesses a non-existing local variable");
                }

                i++;
                break;

            case JAVA_ILOAD_2: // ILOAD_2
            case JAVA_LLOAD_2: // LLOAD_2
#if JEL_FP_SUPPORT
            case JAVA_FLOAD_2: // FLOAD_2
            case JAVA_DLOAD_2: // DLOAD_2
#endif // JEL_FP_SUPPORT
            case JAVA_ALOAD_2: // ALOAD_2
            case JAVA_ISTORE_2: // ISTORE_2
            case JAVA_LSTORE_2: // LSTORE_2
#if JEL_FP_SUPPORT
            case JAVA_FSTORE_2: // FSTORE_2
            case JAVA_DSTORE_2: // DSTORE_2
#endif // JEL_FP_SUPPORT
            case JAVA_ASTORE_2: // ASTORE_2
                if (method->max_locals < 3) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "ILOAD_2, FLOAD_2, ALOAD_2, LLOAD_2, DLOAD_2, "
                            "ISTORE_2, LSTORE_2, FSTORE_2, DSTORE_2 or ASTORE_2"
                            " accesses a non-existing local variable");
                }

                i++;
                break;

            case JAVA_ILOAD_3: // ILOAD_3
            case JAVA_LLOAD_3: // LLOAD_3
#if JEL_FP_SUPPORT
            case JAVA_FLOAD_3: // FLOAD_3
            case JAVA_DLOAD_3: // DLOAD_3
#endif // JEL_FP_SUPPORT
            case JAVA_ALOAD_3: // ALOAD_3
            case JAVA_ISTORE_3: // ISTORE_3
            case JAVA_LSTORE_3: // LSTORE_3
#if JEL_FP_SUPPORT
            case JAVA_FSTORE_3: // FSTORE_3
            case JAVA_DSTORE_3: // DSTORE_3
#endif // JEL_FP_SUPPORT
            case JAVA_ASTORE_3: // ASTORE_3
                if (method->max_locals < 4) {
				                                fprintf(stderr,"\nError!"
								"\nClassname:%s,method name:%s\n",cl->name,method->name);
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "ILOAD_3, FLOAD_3, ALOAD_3, LLOAD_3, DLOAD_3, "
                            "ISTORE_3, LSTORE_3, FSTORE_3, DSTORE_3 or ASTORE_3"
                            " accesses a non-existing local variable");
                }

                i++;
                break;

            case JAVA_IALOAD: // IALOAD
            case JAVA_LALOAD: // LALOAD
#if JEL_FP_SUPPORT
            case JAVA_FALOAD: // FALOAD
            case JAVA_DALOAD: // DALOAD
#endif // JEL_FP_SUPPORT
            case JAVA_AALOAD: // AALOAD
            case JAVA_BALOAD: // BALOAD
            case JAVA_CALOAD: // CALOAD
            case JAVA_SALOAD: // SALOAD
            case JAVA_IASTORE: // IASTORE
            case JAVA_LASTORE: // LASTORE
#if JEL_FP_SUPPORT
            case JAVA_FASTORE: // FASTORE
            case JAVA_DASTORE: // DASTORE
#endif // JEL_FP_SUPPORT
            case JAVA_AASTORE: // AASTORE
            case JAVA_BASTORE: // BASTORE
            case JAVA_CASTORE: // CASTORE
            case JAVA_SASTORE: // SASTORE
            case JAVA_POP: // POP
            case JAVA_POP2: // POP2
            case JAVA_DUP: // DUP
            case JAVA_DUP_X1: // DUP_X1
            case JAVA_DUP_X2: // DUP_X2
            case JAVA_DUP2: // DUP2
            case JAVA_DUP2_X1: // DUP2_X1
            case JAVA_DUP2_X2: // DUP2_X2
            case JAVA_SWAP: // SWAP
            case JAVA_IADD: // IADD
            case JAVA_LADD: // LADD
#if JEL_FP_SUPPORT
            case JAVA_FADD: // FADD
            case JAVA_DADD: // DADD
#endif // JEL_FP_SUPPORT
            case JAVA_ISUB: // ISUB
            case JAVA_LSUB: // LSUB
#if JEL_FP_SUPPORT
            case JAVA_FSUB: // FSUB
            case JAVA_DSUB: // DSUB
#endif // JEL_FP_SUPPORT
            case JAVA_IMUL: // IMUL
            case JAVA_LMUL: // LMUL
#if JEL_FP_SUPPORT
            case JAVA_FMUL: // FMUL
            case JAVA_DMUL: // DMUL
#endif // JEL_FP_SUPPORT
            case JAVA_IDIV: // IDIV
            case JAVA_LDIV: // LDIV
#if JEL_FP_SUPPORT
            case JAVA_FDIV: // FDIV
            case JAVA_DDIV: // DDIV
#endif // JEL_FP_SUPPORT
            case JAVA_IREM: // IREM
            case JAVA_LREM: // LREM
#if JEL_FP_SUPPORT
            case JAVA_FREM: // FREM
            case JAVA_DREM: // DREM
#endif // JEL_FP_SUPPORT
            case JAVA_INEG: // INEG
            case JAVA_LNEG: // LNEG
#if JEL_FP_SUPPORT
            case JAVA_FNEG: // FNEG
            case JAVA_DNEG: // DNEG
#endif // JEL_FP_SUPPORT
            case JAVA_ISHL: // ISHL
            case JAVA_LSHL: // LSHL
            case JAVA_ISHR: // ISHR
            case JAVA_LSHR: // LSHR
            case JAVA_IUSHR: // IUSHR
            case JAVA_LUSHR: // LUSHR
            case JAVA_IAND: // IAND
            case JAVA_LAND: // LAND
            case JAVA_IOR: // IOR
            case JAVA_LOR: // LOR
            case JAVA_IXOR: // IXOR
            case JAVA_LXOR: // LXOR
                i++;
                break;

            case JAVA_IINC: // IINC
                if (code[i + 1] > method->max_locals - 1) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "IINC accesses a non-existing local variable");
                }

                i += 3;
                break;

            case JAVA_I2L: // I2L
#if JEL_FP_SUPPORT
            case JAVA_I2F: // I2F
            case JAVA_I2D:
#endif // JEL_FP_SUPPORT
            case JAVA_L2I: // L2I
#if JEL_FP_SUPPORT
            case JAVA_L2F: // L2F
            case JAVA_L2D: // L2D
            case JAVA_F2I: // F2I
            case JAVA_F2L: // F2L
            case JAVA_F2D: // F2D
            case JAVA_D2I: // D2I
            case JAVA_D2L: // D2L
            case JAVA_D2F: // D2F
#endif // JEL_FP_SUPPORT
            case JAVA_I2B: // I2B
            case JAVA_I2C: // I2C
            case JAVA_I2S: // I2S
            case JAVA_LCMP: // LCMP
#if JEL_FP_SUPPORT
            case JAVA_FCMPL: // FCMPL
            case JAVA_FCMPG: // FCMPG
            case JAVA_DCMPL: // DCMPL
            case JAVA_DCMPG: // DCMPG
#endif // JEL_FP_SUPPORT
                i++;
                break;

            case JAVA_IFEQ: // IFEW
            case JAVA_IFNE: // IFNE
            case JAVA_IFLT: // IFLT
            case JAVA_IFGE: // IFGE
            case JAVA_IFGT: // IFGT
            case JAVA_IFLE: // IFLE
            case JAVA_IF_ICMPEQ: // IF_ICMPEQ
            case JAVA_IF_ICMPNE: // IF_ICMPNE
            case JAVA_IF_ICMPLT: // IF_ICMPLT
            case JAVA_IF_ICMPGE: // IF_ICMPGE
            case JAVA_IF_ICMPGT: // IF_CMPGT
            case JAVA_IF_ICMPLE: // IF_CMPLE
            case JAVA_IF_ACMPEQ: // IF_ACMPEQ
            case JAVA_IF_ACMPNE: // IC_ACMPNE
            case JAVA_GOTO: // GOTO
            {
                int16_t temp;

                temp = (code[i + 1] << 8) | code[i + 2];

                if ((i + temp > code_length - 1) || (i + temp < 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Jump instruction address outside of the code "
                            "range");
                }

                store_int16_un(code + i + 1, temp);
                i += 3;
            }
                break;

            case JAVA_JSR:
            case JAVA_RET:
                // This opcodes shouldn't be present in preverified bytecode
                c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                        "JSR or RET opcode found");
                break;

            case JAVA_TABLESWITCH: // TABLESWITCH
            {
                /*
                 * The integers used by the table-switch instruction are loaded
                 * and turned automagically in the machines native endian format
                 * so that they can be loaded safely as 4-byte integers later,
                 * natural alignment of those integers is guaranteed by the
                 * bytecode
                 */
                int32_t temp;
                int32_t low;
                int32_t high;
                int32_t base;
                int32_t *aligned_ptr;

                base = i;
                i = size_ceil(i + 1, 4);
                aligned_ptr = (int32_t *) (code + i);

                temp = (code[i] << 24) | (code[i + 1] << 16)
                       | (code[i + 2] << 8) | code[i + 3];

                if ((base + temp > code_length - 1) || (base + temp < 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Default TABLESWITCH address outside of the code "
                            "range");
                }

                *aligned_ptr = temp;
                aligned_ptr++;
                i += 4;

                low = (code[i] << 24) | (code[i + 1] << 16)
                      | (code[i + 2] << 8) | code[i + 3];

                *aligned_ptr = low;
                aligned_ptr++;
                i += 4;

                high = (code[i] << 24) | (code[i + 1] << 16)
                       | (code[i + 2] << 8) | code[i + 3];

                *aligned_ptr = high;
                aligned_ptr++;
                i += 4;

                if (high < low) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "high < low in TABLESWITCH opcode");
                }

                j = 0;

                while (j < (high - low + 1)) {
                    temp = (code[i] << 24) | (code[i + 1] << 16)
                           | (code[i + 2] << 8) | code[i + 3];

                    if ((base + temp > code_length - 1) || (base + temp < 0)) {
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "TABLESWITCH address outside of the code "
                                "range");
                    }

                    *aligned_ptr = temp;
                    aligned_ptr++;
                    i += 4;

                    j++;
                }
            }
                break;

            case JAVA_LOOKUPSWITCH: // LOOKUPSWITCH
            {
                /*
                 * The integers used by the lookup-switch instruction are loaded
                 * and turned automagically in the machines native endian format
                 * so that they can be loaded safely as 4-byte integers later,
                 * natural alignment of those integers is guaranteed by the
                 * bytecode
                 */
                int32_t temp;
                int32_t npairs;
                int32_t match;
                int32_t match_old;
                int32_t offset;
                int32_t base;
                int32_t *aligned_ptr;

                base = i;
                match_old = 0;
                i = size_ceil(i + 1, 4);
                aligned_ptr = (int32_t *) (code + i);

                temp = (code[i] << 24) | (code[i + 1] << 16)
                       | (code[i + 2] << 8) | code[i + 3];

                if ((base + temp > code_length - 1) || (base + temp < 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Default LOOKUPSWITCH address outside of the code "
                            "range");
                }

                *aligned_ptr = temp;
                aligned_ptr++;
                i += 4;

                npairs = (code[i] << 24) | (code[i + 1] << 16)
                         | (code[i + 2] << 8) | code[i + 3];

                if (npairs < 0) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "npairs < 0 in LOOKUPSWITCH opcode");
                }

                *aligned_ptr = npairs;
                aligned_ptr++;
                i += 4;
                j = 0;

                while (j < npairs) {
                    match = (code[i] << 24) | (code[i + 1] << 16)
                            | (code[i + 2] << 8) | code[i + 3];

                    *aligned_ptr = match;
                    aligned_ptr++;
                    i += 4;

                    offset = (code[i] << 24) | (code[i + 1] << 16)
                             | (code[i + 2] << 8) | code[i + 3];

                    if ((base + offset > code_length - 1)
                        || (base + offset < 0))
                    {
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "LOOKUPSWITCH address outside of the code "
                                "range");
                    }

                    *aligned_ptr = offset;
                    aligned_ptr++;
                    i += 4;

                    if (j > 0 && match <= match_old) {
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "Unordered match keys in LOOKUPSWITCH opcode");
                    }

                    match_old = match;
                    j++;
                }
            }
                break;

            case JAVA_IRETURN: // IRETURN
                if (synchronized) {
                    code[i] = IRETURN_MONITOREXIT;
                }

                i++;
                break;

            case JAVA_LRETURN: // LRETURN
                if (synchronized) {
                    code[i] = LRETURN_MONITOREXIT;
                }

                i++;
                break;

#if JEL_FP_SUPPORT

            case JAVA_FRETURN: // FRETURN
                if (synchronized) {
                    code[i] = FRETURN_MONITOREXIT;
                }

                i++;
                break;

            case JAVA_DRETURN: // DRETURN
                if (synchronized) {
                    code[i] = DRETURN_MONITOREXIT;
                }

                i++;
                break;

#endif // JEL_FP_SUPPORT

            case JAVA_ARETURN: // ARETURN
                if (synchronized) {
                    code[i] = ARETURN_MONITOREXIT;
                }

                i++;
                break;

            case JAVA_RETURN: // RETURN
                if (synchronized) {
                    code[i] = RETURN_MONITOREXIT;
                }

                i++;
                break;

            case JAVA_GETSTATIC: // GETSTATIC_PRELINK
            case JAVA_PUTSTATIC: // PUTSTATIC_PRELINK
            case JAVA_GETFIELD: // GETFIELD_PRELINK
            case JAVA_PUTFIELD: // PUTFIELD_PRELINK
                i += 3;
                break;

            case JAVA_INVOKEVIRTUAL:
                code[i] = INVOKEVIRTUAL_PRELINK;
                i += 3;
                break;

            case JAVA_INVOKESPECIAL:
                code[i] = INVOKESPECIAL_PRELINK;
                i += 3;
                break;

            case JAVA_INVOKESTATIC:
                code[i] = INVOKESTATIC_PRELINK;
                i += 3;
                break;

            case JAVA_INVOKEINTERFACE:
                code[i] = INVOKEINTERFACE_PRELINK;

                if (code[i + 3] == 0) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "The 'count' of an INVOKEINTERFACE opcode is zero");
                }

                /* RETURN opcodes will find two NOPs after an interface
                 * declaration, this is slower than an optimal implementation
                 * but still works */
                code[i + 3] = NOP;
                code[i + 4] = NOP;
                i += 5;
                break;

            case JAVA_NEW:
                code[i] = NEW_PRELINK;
                i += 3;
                break;

            case JAVA_NEWARRAY:
                code[i] = NEWARRAY_PRELINK;

                switch (code[i + 1]) {
                    case T_BOOLEAN:
                    case T_CHAR:
#if JEL_FP_SUPPORT
                    case T_FLOAT:
                    case T_DOUBLE:
#endif // JEL_FP_SUPPORT
                    case T_BYTE:
                    case T_SHORT:
                    case T_INT:
                    case T_LONG:
                        break;

                    default:
#if JEL_FP_SUPPORT
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "NEWARRAY operand is not of type T_BOOLEAN, T_CHAR,"
                            "T_FLOAT, T_DOUBLE, T_BYTE, T_SHORT, T_INT or "
                            "T_LONG");
#else
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "NEWARRAY operand is not of type T_BOOLEAN, T_CHAR,"
                            "T_BYTE, T_SHORT, T_INT or T_LONG");
#endif // JEL_FP_SUPPORT
                }

                i += 2;
                break;

            case JAVA_ANEWARRAY:
                code[i] = ANEWARRAY_PRELINK;
                i += 3;
                break;

            case JAVA_ARRAYLENGTH: // ARRAYLENGTH
            case JAVA_ATHROW: // ATHROW
                i++;
                break;

            case JAVA_CHECKCAST:
                code[i] = CHECKCAST_PRELINK;
                i += 3;
                break;

            case JAVA_INSTANCEOF:
                code[i] = INSTANCEOF_PRELINK;
                i += 3;
                break;

            case JAVA_MONITORENTER: // MONITORENTER
            case JAVA_MONITOREXIT: // MONITOREXIT
                i++;
                break;

            case JAVA_WIDE:
                i++;

                switch (code[i]) {
                    case JAVA_ILOAD:
                    case JAVA_LLOAD:
#if JEL_FP_SUPPORT
                    case JAVA_FLOAD:
                    case JAVA_DLOAD:
#endif // JEL_FP_SUPPORT
                    case JAVA_ALOAD:
                    case JAVA_ISTORE:
                    case JAVA_LSTORE:
#if JEL_FP_SUPPORT
                    case JAVA_FSTORE:
                    case JAVA_DSTORE:
#endif // JEL_FP_SUPPORT
                    case JAVA_ASTORE:
                    {
                        uint16_t temp;

                        temp = (code[i + 1] << 8) | code[i + 2];

                        if (temp > method->max_locals - 1) {
                            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                    "WIDE ILOAD, FLOAD, ALOAD, LLOAD, DLOAD, "
                                    "ISTORE, FSTORE, ASTORE, LSTORE or DSTORE "
                                    "accesses a non-existing local variable");
                        }

                        store_int16_un(code + i + 1, temp);
                        i += 3;
                    }
                        break;

                    case JAVA_IINC:
                    {
                        uint16_t temp;

                        temp = (code[i + 1] << 8) | code[i + 2];

                        if (temp > method->max_locals - 1) {
                            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                    "WIDE IINC accesses a non-existing local "
                                    "variable");
                        }

                        store_int16_un(code + i + 1, temp);
                        temp = (code[i + 3] << 8) | code[i + 4];
                        store_int16_un(code + i + 3, temp);
                        i += 5;
                    }
                        break;

                    default:
                        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                                "Unknown or wrong opcode after WIDE opcode");
                }

                break;

            case JAVA_MULTIANEWARRAY:
                code[i] = MULTIANEWARRAY_PRELINK;

                if (code[i + 3] == 0) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "MULTIANEWARRAY opcode has 0 dimensions count");
                }

                i += 4;
                break;

            case JAVA_IFNULL: // IFNULL
            case JAVA_IFNONNULL: // IFNONNULL
            {
                int16_t temp;

                temp = (code[i + 1] << 8) | code[i + 2];

                if ((i + temp > code_length - 1) || (i + temp < 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Jump instruction address outside of the code "
                            "range");
                }

                store_int16_un(code + i + 1, temp);
                i += 3;
            }
                break;

            case JAVA_GOTO_W: // GOTO_W
            {
                int32_t temp;

                temp = (code[i + 1] << 24) | (code[i + 2] << 16)
                       | (code[i + 3] << 8) | code[i + 4];

                if ((i + temp > code_length - 1) || (i + temp < 0)) {
                    c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                            "Jump instruction address outside of the code "
                            "range");
                }

                store_int32_un(code + i + 1, temp);
                i += 5;
            }
                break;

            case JAVA_JSR_W:
                c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                        "JSR_W opcode found");
                break;

            default:
                c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                        "Unknown opcode found");
        }
    }

    if (i != code_length) {
        c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                "Last opcode in the code exceeds the specified length");
    }

    // Check consistency of the exception handlers
    for (i = 0; i < method->exception_table_length; i++) {
        if ((handlers[i].start_pc >= handlers[i].end_pc)
            || ((int32_t) handlers[i].start_pc >= code_length)
            || ((int32_t) handlers[i].end_pc > code_length))
        {
            c_throw(JAVA_LANG_NOCLASSDEFFOUNDERROR,
                    "Malformed exception handler");
        }
    }
} // translate_bytecode()
