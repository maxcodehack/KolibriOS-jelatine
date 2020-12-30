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

/** \file print.c
 * Printing facilities */

#include "wrappers.h"

#include "class.h"
#include "constantpool.h"
#include "opcodes.h"
#include "print.h"
#include "thread.h"
#include "vm.h"

#if JEL_PRINT

/** Prints information about a method call
 * \param thread A pointer to the thread executing the call
 * \param method A pointer to the method being called */

void print_method_call(thread_t *thread, const method_t *method)
{
    if (!opts_get_print_methods()) {
        return;
    }

    for (int i = 0; i < thread->call_depth; i++) {
        fprintf(stderr, "    ");
    }

    fprintf(stderr, "CALL %s.%s:%s\n", cp_get_class(method->cp)->name,
                                       method->name, method->descriptor);
    thread->call_depth++;
} // print_method_call()

/** Prints information about the exit point of a method
 * \param thread The thread returning from the method
 * \param method The method from which we are returning */

void print_method_ret(thread_t *thread, const method_t *method)
{
    if (!opts_get_print_methods()) {
        return;
    }

    thread->call_depth--;

    for (int i = 0; i < thread->call_depth; i++) {
        fprintf(stderr, "    ");
    }

    fprintf(stderr, "RET %s.%s:%s\n", cp_get_class(method->cp)->name,
            method->name, method->descriptor);
} // print_method_ret()

/** Prints information about the abrupt termination of a method
 * \param thread The thread abruptly terminating the method
 * \param The method from which we are unwinding */

void print_method_unwind(thread_t *thread, method_t *method)
{
    if (!opts_get_print_methods()) {
        return;
    }

    if (thread->call_depth > 0) {
        thread->call_depth--;
    }

    for (int i = 0; i < thread->call_depth; i++) {
        fprintf(stderr, "    ");
    }

    fprintf(stderr, "UNWIND %s.%s:%s\n", cp_get_class(method->cp)->name,
            method->name, method->descriptor);
} // print_method_unwind()

/** Prints a Java or internal bytecode
 * \param thread A pointer to the thread executing the bytecode
 * \param pc A pointer to the current program counter
 * \param cp A pointer to the current constant-pool */

void print_bytecode(thread_t *thread, const uint8_t *pc, const_pool_t *cp)
{
    if (!opts_get_print_opcodes()) {
        return;
    }

    switch (*pc) {
        case NOP:
            fprintf(stderr, "NOP\n");
            break;

        case ACONST_NULL:
            fprintf(stderr, "ACONST_NULL\n");
            break;

        case ICONST_M1:
            fprintf(stderr, "ICONST_M1\n");
            break;

        case ICONST_0:
            fprintf(stderr, "ICONST_0\n");
            break;

        case ICONST_1:
            fprintf(stderr, "ICONST_1\n");
            break;

        case ICONST_2:
            fprintf(stderr, "ICONST_2\n");
            break;

        case ICONST_3:
            fprintf(stderr, "ICONST_3\n");
            break;

        case ICONST_4:
            fprintf(stderr, "ICONST_4\n");
            break;

        case ICONST_5:
            fprintf(stderr, "ICONST_5\n");
            break;

        case LCONST_0:
            fprintf(stderr, "LCONST_0\n");
            break;

        case LCONST_1:
            fprintf(stderr, "LCONST_1\n");
            break;

        case FCONST_0:
            fprintf(stderr, "FCONST_0\n");
            break;

        case FCONST_1:
            fprintf(stderr, "FCONST_1\n");
            break;

        case FCONST_2:
            fprintf(stderr, "FCONST_2\n");
            break;

        case DCONST_0:
            fprintf(stderr, "DCONST_0\n");
            break;

        case DCONST_1:
            fprintf(stderr, "DCONST_1\n");
            break;

        case BIPUSH:
            fprintf(stderr, "BIPUSH byte = %d\n",
                    (int32_t) *((int8_t *) (pc + 1)));
            break;

        case SIPUSH:
            fprintf(stderr, "SIPUSH short = %d\n",
                    (int32_t) load_int16_un(pc + 1));
            break;

        case LDC:
            if (cp_get_tag(cp, *(pc + 1)) == CONSTANT_Integer) {
                fprintf(stderr, "LDC index = %u value = %d\n", *(pc + 1),
                        cp_get_integer(cp, *(pc + 1)));
            } else {
                 fprintf(stderr, "LDC index = %u value = %f\n", *(pc + 1),
                        cp_get_float(cp, *(pc + 1)));
            }
            break;

        case LDC_W:
            if (cp_get_tag(cp, load_uint16_un(pc + 1)) == CONSTANT_Integer) {
                fprintf(stderr, "LDC_W index = %u value = %d\n",
                        load_uint16_un(pc + 1),
                        cp_get_integer(cp, load_uint16_un(pc + 1)));
            } else {
                 fprintf(stderr, "LDC_W index = %u value = %f\n",
                        load_uint16_un(pc + 1),
                        cp_get_float(cp, load_uint16_un(pc + 1)));
            }
            break;

        case LDC2_W:
            if (cp_get_tag(cp, load_uint16_un(pc + 1)) == CONSTANT_Long) {
#if SIZEOF_LONG == 8
                fprintf(stderr, "LDC2_W index = %u value = %ld\n",
                        load_uint16_un(pc + 1),
                        cp_get_long(cp, load_uint16_un(pc + 1)));
#else
                fprintf(stderr, "LDC2_W index = %u value = %lld\n",
                        load_uint16_un(pc + 1),
                        cp_get_long(cp, load_uint16_un(pc + 1)));
#endif // SIZEOF_LONG == 8
            } else {
                 fprintf(stderr, "LDC2_W index = %u value = %f\n",
                        load_uint16_un(pc + 1),
                        cp_get_double(cp, load_uint16_un(pc + 1)));
            }
            break;

        case ILOAD:
            fprintf(stderr, "ILOAD index = %u\n", *(pc + 1));
            break;

        case LLOAD:
            fprintf(stderr, "LLOAD index = %u\n", *(pc + 1));
            break;

        case FLOAD:
            fprintf(stderr, "FLOAD index = %u\n", *(pc + 1));
            break;

        case DLOAD:
            fprintf(stderr, "DLOAD index = %u\n", *(pc + 1));
            break;

        case ALOAD:
            fprintf(stderr, "ALOAD index = %u\n", *(pc + 1));
            break;

        case ILOAD_0:
            fprintf(stderr, "ILOAD_0\n");
            break;

        case ILOAD_1:
            fprintf(stderr, "ILOAD_1\n");
            break;

        case ILOAD_2:
            fprintf(stderr, "ILOAD_2\n");
            break;

        case ILOAD_3:
            fprintf(stderr, "ILOAD_3\n");
            break;

        case LLOAD_0:
            fprintf(stderr, "LLOAD_0\n");
            break;

        case LLOAD_1:
            fprintf(stderr, "LLOAD_1\n");
            break;

        case LLOAD_2:
            fprintf(stderr, "LLOAD_2\n");
            break;

        case LLOAD_3:
            fprintf(stderr, "LLOAD_3\n");
            break;

        case FLOAD_0:
            fprintf(stderr, "FLOAD_0\n");
            break;

        case FLOAD_1:
            fprintf(stderr, "FLOAD_1\n");
            break;

        case FLOAD_2:
            fprintf(stderr, "FLOAD_2\n");
            break;

        case FLOAD_3:
            fprintf(stderr, "FLOAD_3\n");
            break;

        case DLOAD_0:
            fprintf(stderr, "DLOAD_0\n");
            break;

        case DLOAD_1:
            fprintf(stderr, "DLOAD_1\n");
            break;

        case DLOAD_2:
            fprintf(stderr, "DLOAD_2\n");
            break;

        case DLOAD_3:
            fprintf(stderr, "DLOAD_3\n");
            break;

        case ALOAD_0:
            fprintf(stderr, "ALOAD_0\n");
            break;

        case ALOAD_1:
            fprintf(stderr, "ALOAD_1\n");
            break;

        case ALOAD_2:
            fprintf(stderr, "ALOAD_2\n");
            break;

        case ALOAD_3:
            fprintf(stderr, "ALOAD_3\n");
            break;

        case IALOAD:
            fprintf(stderr, "IALOAD\n");
            break;

        case LALOAD:
            fprintf(stderr, "LALOAD\n");
            break;

        case FALOAD:
            fprintf(stderr, "FALOAD\n");
            break;

        case DALOAD:
            fprintf(stderr, "DALOAD\n");
            break;

        case AALOAD:
            fprintf(stderr, "AALOAD\n");
            break;

        case BALOAD:
            fprintf(stderr, "BALOAD\n");
            break;

        case CALOAD:
            fprintf(stderr, "CALOAD\n");
            break;

        case SALOAD:
            fprintf(stderr, "SALOAD\n");
            break;

        case ISTORE:
            fprintf(stderr, "ISTORE index = %u\n", *(pc + 1));
            break;

        case LSTORE:
            fprintf(stderr, "LSTORE index = %u\n", *(pc + 1));
            break;

        case FSTORE:
            fprintf(stderr, "FSTORE index = %u\n", *(pc + 1));
            break;

        case DSTORE:
            fprintf(stderr, "DSTORE index = %u\n", *(pc + 1));
            break;

        case ASTORE:
            fprintf(stderr, "ASTORE index = %u\n", *(pc + 1));
            break;

        case ISTORE_0:
            fprintf(stderr, "ISTORE_0\n");
            break;

        case ISTORE_1:
            fprintf(stderr, "ISTORE_1\n");
            break;

        case ISTORE_2:
            fprintf(stderr, "ISTORE_2\n");
            break;

        case ISTORE_3:
            fprintf(stderr, "ISTORE_3\n");
            break;

        case LSTORE_0:
            fprintf(stderr, "LSTORE_0\n");
            break;

        case LSTORE_1:
            fprintf(stderr, "LSTORE_1\n");
            break;

        case LSTORE_2:
            fprintf(stderr, "LSTORE_2\n");
            break;

        case LSTORE_3:
            fprintf(stderr, "LSTORE_3\n");
            break;

        case FSTORE_0:
            fprintf(stderr, "FSTORE_0\n");
            break;

        case FSTORE_1:
            fprintf(stderr, "FSTORE_1\n");
            break;

        case FSTORE_2:
            fprintf(stderr, "FSTORE_2\n");
            break;

        case FSTORE_3:
            fprintf(stderr, "FSTORE_3\n");
            break;

        case DSTORE_0:
            fprintf(stderr, "DSTORE_0\n");
            break;

        case DSTORE_1:
            fprintf(stderr, "DSTORE_1\n");
            break;

        case DSTORE_2:
            fprintf(stderr, "DSTORE_2\n");
            break;

        case DSTORE_3:
            fprintf(stderr, "DSTORE_3\n");
            break;

        case ASTORE_0:
            fprintf(stderr, "ASTORE_0\n");
            break;

        case ASTORE_1:
            fprintf(stderr, "ASTORE_1\n");
            break;

        case ASTORE_2:
            fprintf(stderr, "ASTORE_2\n");
            break;

        case ASTORE_3:
            fprintf(stderr, "ASTORE_3\n");
            break;

        case IASTORE:
            fprintf(stderr, "IASTORE\n");
            break;

        case LASTORE:
            fprintf(stderr, "LASTORE\n");
            break;

        case FASTORE:
            fprintf(stderr, "FASTORE\n");
            break;

        case DASTORE:
            fprintf(stderr, "DASTORE\n");
            break;

        case AASTORE:
            fprintf(stderr, "AASTORE\n");
            break;

        case BASTORE:
            fprintf(stderr, "BASTORE\n");
            break;

        case CASTORE:
            fprintf(stderr, "CASTORE\n");
            break;

        case SASTORE:
            fprintf(stderr, "SASTORE\n");
            break;

        case POP:
            fprintf(stderr, "POP\n");
            break;

        case POP2:
            fprintf(stderr, "POP2\n");
            break;

        case DUP:
            fprintf(stderr, "DUP\n");
            break;

        case DUP_X1:
            fprintf(stderr, "DUP_X1\n");
            break;

        case DUP_X2:
            fprintf(stderr, "DUP_X2\n");
            break;

        case DUP2:
            fprintf(stderr, "DUP2\n");
            break;

        case DUP2_X1:
            fprintf(stderr, "DUP2_X1\n");
            break;

        case DUP2_X2:
            fprintf(stderr, "DUP2_X2\n");
            break;

        case SWAP:
            fprintf(stderr, "SWAP\n");
            break;

        case IADD:
            fprintf(stderr, "IADD\n");
            break;

        case LADD:
            fprintf(stderr, "LADD\n");
            break;

        case FADD:
            fprintf(stderr, "FADD\n");
            break;

        case DADD:
            fprintf(stderr, "DADD\n");
            break;

        case ISUB:
            fprintf(stderr, "ISUB\n");
            break;

        case LSUB:
            fprintf(stderr, "LSUB\n");
            break;

        case FSUB:
            fprintf(stderr, "FSUB\n");
            break;

        case DSUB:
            fprintf(stderr, "DSUB\n");
            break;

        case IMUL:
            fprintf(stderr, "IMUL\n");
            break;

        case LMUL:
            fprintf(stderr, "LMUL\n");
            break;

        case FMUL:
            fprintf(stderr, "FMUL\n");
            break;

        case DMUL:
            fprintf(stderr, "DMUL\n");
            break;

        case IDIV:
            fprintf(stderr, "IDIV\n");
            break;

        case LDIV:
            fprintf(stderr, "LDIV\n");
            break;

        case FDIV:
            fprintf(stderr, "FDIV\n");
            break;

        case DDIV:
            fprintf(stderr, "DDIV\n");
            break;

        case IREM:
            fprintf(stderr, "IREM\n");
            break;

        case LREM:
            fprintf(stderr, "LREM\n");
            break;

        case FREM:
            fprintf(stderr, "FREM\n");
            break;

        case DREM:
            fprintf(stderr, "DREM\n");
            break;

        case INEG:
            fprintf(stderr, "INEG\n");
            break;

        case LNEG:
            fprintf(stderr, "LNEG\n");
            break;

        case FNEG:
            fprintf(stderr, "FNEG\n");
            break;

        case DNEG:
            fprintf(stderr, "DNEG\n");
            break;

        case ISHL:
            fprintf(stderr, "ISHL\n");
            break;

        case LSHL:
            fprintf(stderr, "LSHL\n");
            break;

        case ISHR:
            fprintf(stderr, "ISHR\n");
            break;

        case LSHR:
            fprintf(stderr, "LSHR\n");
            break;

        case IUSHR:
            fprintf(stderr, "IUSHR\n");
            break;

        case LUSHR:
            fprintf(stderr, "LUSHR\n");
            break;

        case IAND:
            fprintf(stderr, "IAND\n");
            break;

        case LAND:
            fprintf(stderr, "LAND\n");
            break;

        case IOR:
            fprintf(stderr, "IOR\n");
            break;

        case LOR:
            fprintf(stderr, "LOR\n");
            break;

        case IXOR:
            fprintf(stderr, "IXOR\n");
            break;

        case LXOR:
            fprintf(stderr, "LXOR\n");
            break;

        case IINC:
            fprintf(stderr, "IINC index = %u increment = %d\n", *(pc + 1),
                    (int32_t) *(pc + 2));
            break;

        case I2L:
            fprintf(stderr, "I2L\n");
            break;

        case I2F:
            fprintf(stderr, "I2F\n");
            break;

        case I2D:
            fprintf(stderr, "I2D\n");
            break;

        case L2I:
            fprintf(stderr, "L2I\n");
            break;

        case L2F:
            fprintf(stderr, "L2F\n");
            break;

        case L2D:
            fprintf(stderr, "L2D\n");
            break;

        case F2I:
            fprintf(stderr, "F2I\n");
            break;

        case F2L:
            fprintf(stderr, "F2L\n");
            break;

        case F2D:
            fprintf(stderr, "F2D\n");
            break;

        case D2I:
            fprintf(stderr, "D2I\n");
            break;

        case D2L:
            fprintf(stderr, "D2L\n");
            break;

        case D2F:
            fprintf(stderr, "D2F\n");
            break;

        case I2B:
            fprintf(stderr, "I2B\n");
            break;

        case I2C:
            fprintf(stderr, "I2C\n");
            break;

        case I2S:
            fprintf(stderr, "I2S\n");
            break;

        case LCMP:
            fprintf(stderr, "LCMP\n");
            break;

        case FCMPL:
            fprintf(stderr, "FCMPL\n");
            break;

        case FCMPG:
            fprintf(stderr, "FCMPG\n");
            break;

        case DCMPL:
            fprintf(stderr, "DCMPL\n");
            break;

        case DCMPG:
            fprintf(stderr, "DCMPG\n");
            break;

        case IFEQ:
            fprintf(stderr, "IFEQ offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFNE:
            fprintf(stderr, "IFNE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFLT:
            fprintf(stderr, "IFLT offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFGE:
            fprintf(stderr, "IFGE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFGT:
            fprintf(stderr, "IFGT offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFLE:
            fprintf(stderr, "IFLE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPEQ:
            fprintf(stderr, "IF_ICMPEQ offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPNE:
            fprintf(stderr, "IF_ICMPNE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPLT:
            fprintf(stderr, "IF_ICMPLT offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPGE:
            fprintf(stderr, "IF_ICMPGE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPGT:
            fprintf(stderr, "IF_ICMPGT offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ICMPLE:
            fprintf(stderr, "IF_ICMPLE offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ACMPEQ:
            fprintf(stderr, "IF_ACMPEQ offset = %d\n", load_int16_un(pc + 1));
            break;

        case IF_ACMPNE:
            fprintf(stderr, "IF_ACMPNE offset = %d\n", load_int16_un(pc + 1));
            break;

        case GOTO:
            fprintf(stderr, "GOTO offset = %d\n", load_int16_un(pc + 1));
            break;

        case LDC_REF:
        {
            uintptr_t ref = cp_get_ref(cp, *(pc + 1));
            java_lang_String_t *jstring;
            java_lang_Class_t *jclass;

            if (cp_get_tag(cp, *(pc + 1)) == CONSTANT_Class) {
                fprintf(stderr, "LDC_REF CONSTANT_Class ");
                jclass = JAVA_LANG_CLASS_REF2PTR(ref);
                jstring_print(JAVA_LANG_STRING_REF2PTR(jclass->name));
            } else { // CONSTANT_String
                fprintf(stderr, "LDC_REF CONSTANT_String ");
                jstring = JAVA_LANG_STRING_REF2PTR(ref);
                jstring_print(jstring);
            }

            fprintf(stderr, "\n");
        }
            break;

        case LDC_W_REF:
        {
            uintptr_t ref = cp_get_ref(cp, load_uint16_un(pc + 1));
            java_lang_String_t *jstring;
            java_lang_Class_t *jclass;

            if (cp_get_tag(cp, load_uint16_un(pc + 1)) == CONSTANT_Class) {
                fprintf(stderr, "LDC_REF CONSTANT_Class ");
                jclass = JAVA_LANG_CLASS_REF2PTR(ref);
                jstring_print(JAVA_LANG_STRING_REF2PTR(jclass->name));
            } else { // CONSTANT_String
                fprintf(stderr, "LDC_REF CONSTANT_String ");
                jstring = JAVA_LANG_STRING_REF2PTR(ref);
                jstring_print(jstring);
            }

            fprintf(stderr, "\n");
        }
            break;

        case TABLESWITCH:
            fprintf(stderr, "TABLESWITCH\n");
            break;

        case LOOKUPSWITCH:
            fprintf(stderr, "LOOKUPSWITCH\n");
            break;

        case IRETURN:
            fprintf(stderr, "IRETURN\n");
            break;

        case LRETURN:
            fprintf(stderr, "LRETURN\n");
            break;

        case FRETURN:
            fprintf(stderr, "FRETURN\n");
            break;

        case DRETURN:
            fprintf(stderr, "DRETURN\n");
            break;

        case ARETURN:
            fprintf(stderr, "ARETURN\n");
            break;

        case RETURN:
            fprintf(stderr, "RETURN\n");
            break;

        case GETSTATIC_PRELINK:
            fprintf(stderr, "GETSTATIC\n");
            break;

        case PUTSTATIC_PRELINK:
            fprintf(stderr, "PUTSTATIC\n");
            break;

        case GETFIELD_PRELINK:
            fprintf(stderr, "GETFIELD\n");
            break;

        case PUTFIELD_PRELINK:
            fprintf(stderr, "PUTFIELD\n");
            break;

        case INVOKEVIRTUAL: // TODO: Improve
        {
            uint16_t index = method_unpack_index(load_uint16_un(pc + 1));
            fprintf(stderr, "INVOKEVIRTUAL index = %u\n", index);
        }
            break;

        case INVOKESPECIAL: // TODO: Improve
        {
            uint16_t index = load_uint16_un(pc + 1);
            method_t *method = cp_get_resolved_method(cp, index);

            fprintf(stderr, "INVOKESPECIAL class = %s name = %s desc =%s\n",
                    cp_get_class(method->cp)->name, method->name,
                    method->descriptor);
        }
            break;

        case INVOKESTATIC: // TODO: Improve
        {
            uint16_t index = load_uint16_un(pc + 1);
            method_t *method = cp_get_resolved_method(cp, index);

            fprintf(stderr, "INVOKESTATIC class = %s name = %s desc =%s\n",
                    cp_get_class(method->cp)->name, method->name,
                    method->descriptor);
        }
            break;

        case INVOKEINTERFACE: // TODO: Improve
        {
            uint16_t index = method_unpack_index(load_uint16_un(pc + 1));
            fprintf(stderr, "INVOKEINTERFACE index = %u\n", index);
        }
            break;

        case NEW: // TODO: Improve
            fprintf(stderr, "NEW index = %u\n", load_uint16_un(pc + 1));
            break;

        case NEWARRAY:
            fprintf(stderr, "NEWARRAY type = ");

            switch (*(pc + 1)) {
                case T_BOOLEAN: fprintf(stderr, "T_BOOLEAN\n"); break;
                case T_CHAR:    fprintf(stderr, "T_CHAR\n");    break;
#if JEL_FP_SUPPORT
                case T_FLOAT:   fprintf(stderr, "T_FLOAT\n");   break;
                case T_DOUBLE:  fprintf(stderr, "T_DOUBLE\n");  break;
#endif // JEL_FP_SUPPORT
                case T_BYTE:    fprintf(stderr, "T_BYTE\n");    break;
                case T_SHORT:   fprintf(stderr, "T_SHORT\n");   break;
                case T_INT:     fprintf(stderr, "T_INT\n");     break;
                case T_LONG:    fprintf(stderr, "T_LONG\n");    break;
                default:        dbg_unreachable();
            }

            break;

        case ANEWARRAY: // TODO: Improve
            fprintf(stderr, "ANEWARRAY index = %u\n", load_uint16_un(pc + 1));
            break;

        case ARRAYLENGTH:
            fprintf(stderr, "ARRAYLENGTH\n");
            break;

        case ATHROW: // TODO: Improve
            fprintf(stderr, "ATHROW\n");
            break;

        case CHECKCAST: // TODO: Improve
        {
            uint16_t index = load_uint16_un(pc + 1);
            fprintf(stderr, "CHECKCAST index = %u\n", index);
        }
            break;

        case INSTANCEOF: // TODO: Improve
        {
            uint16_t index = load_uint16_un(pc + 1);
            fprintf(stderr, "INSTANCEOF index = %u\n", index);
        }
            break;

        case MONITORENTER:
            fprintf(stderr, "MONITORENTER\n");
            break;

        case MONITOREXIT:
            fprintf(stderr, "MONITOREXIT\n");
            break;

        case WIDE:
        {
            uint16_t index = load_uint16_un(pc + 2);

            fprintf(stderr, "WIDE ");

            switch (*(pc + 1)) {
                case ILOAD:
                    fprintf(stderr, "ILOAD index = %u\n", index);
                    break;

                case FLOAD:
                    fprintf(stderr, "FLOAD index = %u\n", index);
                    break;

                case ALOAD:
                    fprintf(stderr, "ALOAD index = %u\n", index);
                    break;

                case LLOAD:
                    fprintf(stderr, "LLOAD index = %u\n", index);
                    break;

                case DLOAD:
                    fprintf(stderr, "DLOAD index = %u\n", index);
                    break;

                case ISTORE:
                    fprintf(stderr, "ISTORE index = %u\n", index);
                    break;

                case FSTORE:
                    fprintf(stderr, "FSTORE index = %u\n", index);
                    break;

                case ASTORE:
                    fprintf(stderr, "ASTORE index = %u\n", index);
                    break;

                case LSTORE:
                    fprintf(stderr, "LSTORE index = %u\n", index);
                    break;

                case DSTORE:
                    fprintf(stderr, "DSTORE index = %u\n", index);
                    break;

                case IINC:
                    fprintf(stderr, "IINC index = %u increment = %d\n", index,
                            (int32_t) load_int16_un(pc + 4));
                    break;

                case METHOD_LOAD: // TODO: Improve
                    fprintf(stderr, "METHOD_LOAD\n");
                    break;

                case METHOD_ABSTRACT: // TODO: Improve
                    fprintf(stderr, "METHOD_ABSTRACT\n");
                    break;

                case INVOKE_NATIVE: // TODO: Improve
                    fprintf(stderr, "INVOKE_NATIVE\n");
                    break;

                case HALT:
                    fprintf(stderr, "HALT\n");
                    break;

                default:
                    dbg_unreachable();
            }
                break;

        case MULTIANEWARRAY:
            fprintf(stderr, "MULTIANEWARRAY\n");
            break;

        case IFNULL:
            fprintf(stderr, "IFNULL offset = %d\n", load_int16_un(pc + 1));
            break;

        case IFNONNULL:
            fprintf(stderr, "IFNONNULL offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GOTO_W:
            fprintf(stderr, "GOTO_W offset = %d\n", load_int32_un(pc + 1));
            break;

        case GETSTATIC_BYTE:
        // case GETSTATIC_BOOL:
            fprintf(stderr, "GETSTATIC_BYTE/BOOL offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_CHAR:
            fprintf(stderr, "GETSTATIC_CHAR offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_SHORT:
            fprintf(stderr, "GETSTATIC_SHORT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_INT:
            fprintf(stderr, "GETSTATIC_INT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_FLOAT:
            fprintf(stderr, "GETSTATIC_FLOAT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_LONG:
            fprintf(stderr, "GETSTATIC_LONG offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_DOUBLE:
            fprintf(stderr, "GETSTATIC_DOUBLE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETSTATIC_REFERENCE:
            fprintf(stderr, "GETSTATIC_REFERENCE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_BYTE:
            fprintf(stderr, "PUTSTATIC_BYTE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_BOOL:
            fprintf(stderr, "PUTSTATIC_BOOL offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_CHAR:
        // case PUTSTATIC_SHORT:
            fprintf(stderr, "PUTSTATIC_CHAR/SHORT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_INT:
            fprintf(stderr, "PUTSTATIC_INT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_FLOAT:
            fprintf(stderr, "PUTSTATIC_FLOAT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_LONG:
            fprintf(stderr, "PUTSTATIC_LONG offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_DOUBLE:
            fprintf(stderr, "PUTSTATIC_DOUBLE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTSTATIC_REFERENCE:
            fprintf(stderr, "PUTSTATIC_REFERENCE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_BYTE:
            fprintf(stderr, "GETFIELD_BYTE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_BOOL:
            fprintf(stderr, "GETFIELD_BOOL offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_CHAR:
            fprintf(stderr, "GETFIELD_CHAR offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_SHORT:
            fprintf(stderr, "GETFIELD_SHORT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_INT:
            fprintf(stderr, "GETFIELD_INT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_FLOAT:
            fprintf(stderr, "GETFIELD_FLOAT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_LONG:
            fprintf(stderr, "GETFIELD_LONG offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_DOUBLE:
            fprintf(stderr, "GETFIELD_DOUBLE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case GETFIELD_REFERENCE:
            fprintf(stderr, "GETFIELD_REFERENCE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_BYTE:
            fprintf(stderr, "PUTFIELD_BYTE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_BOOL:
            fprintf(stderr, "PUTFIELD_BOOL offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_CHAR:
        // case PUTFIELD_SHORT:
            fprintf(stderr, "PUTFIELD_CHAR/SHORT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_INT:
            fprintf(stderr, "PUTFIELD_INT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_FLOAT:
            fprintf(stderr, "PUTFIELD_FLOAT offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_LONG:
            fprintf(stderr, "PUTFIELD_LONG offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_DOUBLE:
            fprintf(stderr, "PUTFIELD_DOUBLE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case PUTFIELD_REFERENCE:
            fprintf(stderr, "PUTFIELD_REFERENCE offset = %d\n",
                    load_int16_un(pc + 1));
            break;

        case INVOKESUPER:
            fprintf(stderr, "INVOKESUPER\n");
            break;

        case INVOKEVIRTUAL_PRELINK:
            fprintf(stderr, "INVOKEVIRTUAL_PRELINK\n");
            break;

        case INVOKESPECIAL_PRELINK:
            fprintf(stderr, "INVOKESPECIAL_PRELINK\n");
            break;

        case INVOKESTATIC_PRELINK:
            fprintf(stderr, "INVOKESTATIC_PRELINK\n");
            break;

        case INVOKEINTERFACE_PRELINK:
            fprintf(stderr, "INVOKEINTERFACE_PRELINK\n");
            break;

        case NEW_PRELINK:
            fprintf(stderr, "NEW_PRELINK\n");
            break;


        case NEWARRAY_PRELINK:
            fprintf(stderr, "NEWARRAY_PRELINK type = ");

            switch (*(pc + 1)) {
                case T_BOOLEAN: fprintf(stderr, "T_BOOLEAN\n"); break;
                case T_CHAR:    fprintf(stderr, "T_CHAR\n");    break;
#if JEL_FP_SUPPORT
                case T_FLOAT:   fprintf(stderr, "T_FLOAT\n");   break;
                case T_DOUBLE:  fprintf(stderr, "T_DOUBLE\n");  break;
#endif // JEL_FP_SUPPORT
                case T_BYTE:    fprintf(stderr, "T_BYTE\n");    break;
                case T_SHORT:   fprintf(stderr, "T_SHORT\n");   break;
                case T_INT:     fprintf(stderr, "T_INT\n");     break;
                case T_LONG:    fprintf(stderr, "T_LONG\n");    break;
                default:        dbg_unreachable();
            }

            break;

        case ANEWARRAY_PRELINK:
            fprintf(stderr, "ANEWARRAY_PRELINK\n");
            break;

        case CHECKCAST_PRELINK:
            fprintf(stderr, "CHECKCAST_PRELINK\n");
            break;

        case INSTANCEOF_PRELINK:
            fprintf(stderr, "INSTANCEOF_PRELINK\n");
            break;

        case MULTIANEWARRAY_PRELINK:
            fprintf(stderr, "MULTIANEWARRAY_PRELINK\n");
            break;

        case MONITORENTER_SPECIAL:
            fprintf(stderr, "MONITORENTER_SPECIAL\n");
            break;

        case MONITORENTER_SPECIAL_STATIC:
            fprintf(stderr, "MONITORENTER_SPECIAL_STATIC\n");
            break;

        case IRETURN_MONITOREXIT:
            fprintf(stderr, "IRETURN_MONITOREXIT\n");
            break;

        case LRETURN_MONITOREXIT:
            fprintf(stderr, "LRETURN_MONITOREXIT\n");
            break;

        case FRETURN_MONITOREXIT:
            fprintf(stderr, "FRETURN_MONITOREXIT\n");
            break;

        case DRETURN_MONITOREXIT:
            fprintf(stderr, "DRETURN_MONITOREXIT\n");
            break;

        case ARETURN_MONITOREXIT:
            fprintf(stderr, "ARETURN_MONITOREXIT\n");
            break;

        case RETURN_MONITOREXIT:
            fprintf(stderr, "RETURN_MONITOREXIT\n");
            break;

        case NEW_FINALIZER: // TODO: Improve
            fprintf(stderr, "NEW_FINALIZER index = %u\n",
                    load_uint16_un(pc + 1));
            break;

        case LDC_PRELINK:
            fprintf(stderr, "LDC_PRELINK\n");
            break;

        case LDC_W_PRELINK:
            fprintf(stderr, "LDC_W_PRELINK\n");
            break;

        default:
            fprintf(stderr, "UNKNOWN OPCODE\n");
            abort();
        }
    }
} // print_bytecode()

#endif // JEL_PRINT
