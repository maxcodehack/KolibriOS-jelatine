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

/** \file interpreter.c
 * Bytecode interpreter implementation */

#include "wrappers.h"

#include "array.h"
#include "class.h"
#include "classfile.h"
#include "field.h"
#include "interpreter.h"
#include "loader.h"
#include "method.h"
#include "opcodes.h"
#include "print.h"
#include "thread.h"
#include "util.h"
#include "vm.h"

#include "java_lang_Class.h"

/******************************************************************************
 * Helper functions and macros                                                *
 ******************************************************************************/

/** Create a safe point by saving the relevant part of the interpreter's
 * state */

#define SAVE_STATE \
    do { \
        thread->sp = sp; \
        thread->pc = pc; \
        thread->fp = fp; \
    } while (0)

/******************************************************************************
 * Interpreter implementation                                                 *
 ******************************************************************************/

/** Prepares the stack for executing a method when entering the interpreter
 * \param thread The calling thread
 * \param method The method to be called */

static void prepare_for_call(thread_t *thread, method_t *method)
{
    stack_frame_t *fp;

    // Check if we are not overflowing the stack
    if ((thread->sp + method->max_locals + method->max_stack)
        > (jword_t *) (thread->fp - 2))
    {
        c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                "Stack overflow, try using a larger stack with the --stack-size"
                " parameter");
    }

    // Prepare the first, fake stack-frame
    fp = thread->fp - 1;
    fp->cl = NULL;
    fp->method = &halt_method;
    fp->pc = halt_method.code + 2;
    fp->locals = thread->sp;

    // Prepare the real first stack frame
    fp = thread->fp - 2;
    fp->cl = cp_get_class(method->cp);
    fp->method = method;
    fp->pc = method->code;
    fp->locals = thread->sp;

    // Prepare the thread state
    thread->sp += method->max_locals;
    thread->fp -= 2;
} // prepare_for_call()

/** Launches the interpreter
 * \param main_method The entry point */

void interpreter(method_t *main_method)
{
    // Runtime state
    const uint8_t *pc; // Program counter
    jword_t *sp; // Stack-pointer
    stack_frame_t *fp; // Current frame-pointer
    jword_t *locals; // Local variables pointer
    jword_t *cp; // Current constant-pool
    thread_t *thread; // The executing thread

    // Cached globals
#if JEL_THREADED_INTERPRETER
    static const void *dispatch[256] = {
        &&NOP_label,
        &&ACONST_NULL_label,
        &&ICONST_M1_label,
        &&ICONST_0_label,
        &&ICONST_1_label,
        &&ICONST_2_label,
        &&ICONST_3_label,
        &&ICONST_4_label,
        &&ICONST_5_label,
        &&LCONST_0_label,
        &&LCONST_1_label,
#if JEL_FP_SUPPORT
        &&FCONST_0_label,
        &&FCONST_1_label,
        &&FCONST_2_label,
        &&DCONST_0_label,
        &&DCONST_1_label,
#else
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&BIPUSH_label,
        &&SIPUSH_label,
        &&LDC_label,
        &&LDC_W_label,
        &&LDC2_W_label,
        &&ILOAD_label,
        &&LLOAD_label,
#if JEL_FP_SUPPORT
        &&FLOAD_label,
        &&DLOAD_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ALOAD_label,
        &&ILOAD_0_label,
        &&ILOAD_1_label,
        &&ILOAD_2_label,
        &&ILOAD_3_label,
        &&LLOAD_0_label,
        &&LLOAD_1_label,
        &&LLOAD_2_label,
        &&LLOAD_3_label,
#if JEL_FP_SUPPORT
        &&FLOAD_0_label,
        &&FLOAD_1_label,
        &&FLOAD_2_label,
        &&FLOAD_3_label,
        &&DLOAD_0_label,
        &&DLOAD_1_label,
        &&DLOAD_2_label,
        &&DLOAD_3_label,
#else
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ALOAD_0_label,
        &&ALOAD_1_label,
        &&ALOAD_2_label,
        &&ALOAD_3_label,
        &&IALOAD_label,
        &&LALOAD_label,
#if JEL_FP_SUPPORT
        &&FALOAD_label,
        &&DALOAD_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&AALOAD_label,
        &&BALOAD_label,
        &&CALOAD_label,
        &&SALOAD_label,
        &&ISTORE_label,
        &&LSTORE_label,
#if JEL_FP_SUPPORT
        &&FSTORE_label,
        &&DSTORE_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ASTORE_label,
        &&ISTORE_0_label,
        &&ISTORE_1_label,
        &&ISTORE_2_label,
        &&ISTORE_3_label,
        &&LSTORE_0_label,
        &&LSTORE_1_label,
        &&LSTORE_2_label,
        &&LSTORE_3_label,
#if JEL_FP_SUPPORT
        &&FSTORE_0_label,
        &&FSTORE_1_label,
        &&FSTORE_2_label,
        &&FSTORE_3_label,
        &&DSTORE_0_label,
        &&DSTORE_1_label,
        &&DSTORE_2_label,
        &&DSTORE_3_label,
#else
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ASTORE_0_label,
        &&ASTORE_1_label,
        &&ASTORE_2_label,
        &&ASTORE_3_label,
        &&IASTORE_label,
        &&LASTORE_label,
#if JEL_FP_SUPPORT
        &&FASTORE_label,
        &&DASTORE_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&AASTORE_label,
        &&BASTORE_label,
        &&CASTORE_label,
        &&SASTORE_label,
        &&POP_label,
        &&POP2_label,
        &&DUP_label,
        &&DUP_X1_label,
        &&DUP_X2_label,
        &&DUP2_label,
        &&DUP2_X1_label,
        &&DUP2_X2_label,
        &&SWAP_label,
        &&IADD_label,
        &&LADD_label,
#if JEL_FP_SUPPORT
        &&FADD_label,
        &&DADD_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ISUB_label,
        &&LSUB_label,
#if JEL_FP_SUPPORT
        &&FSUB_label,
        &&DSUB_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&IMUL_label,
        &&LMUL_label,
#if JEL_FP_SUPPORT
        &&FMUL_label,
        &&DMUL_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&IDIV_label,
        &&LDIV_label,
#if JEL_FP_SUPPORT
        &&FDIV_label,
        &&DDIV_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&IREM_label,
        &&LREM_label,
#if JEL_FP_SUPPORT
        &&FREM_label,
        &&DREM_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&INEG_label,
        &&LNEG_label,
#if JEL_FP_SUPPORT
        &&FNEG_label,
        &&DNEG_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ISHL_label,
        &&LSHL_label,
        &&ISHR_label,
        &&LSHR_label,
        &&IUSHR_label,
        &&LUSHR_label,
        &&IAND_label,
        &&LAND_label,
        &&IOR_label,
        &&LOR_label,
        &&IXOR_label,
        &&LXOR_label,
        &&IINC_label,
        &&I2L_label,
#if JEL_FP_SUPPORT
        &&I2F_label,
        &&I2D_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&L2I_label,
#if JEL_FP_SUPPORT
        &&L2F_label,
        &&L2D_label,
        &&F2I_label,
        &&F2L_label,
        &&F2D_label,
        &&D2I_label,
        &&D2L_label,
        &&D2F_label,
#else
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&I2B_label,
        &&I2C_label,
        &&I2S_label,
        &&LCMP_label,
#if JEL_FP_SUPPORT
        &&FCMPL_label,
        &&FCMPG_label,
        &&DCMPL_label,
        &&DCMPG_label,
#else
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&IFEQ_label,
        &&IFNE_label,
        &&IFLT_label,
        &&IFGE_label,
        &&IFGT_label,
        &&IFLE_label,
        &&IF_ICMPEQ_label,
        &&IF_ICMPNE_label,
        &&IF_ICMPLT_label,
        &&IF_ICMPGE_label,
        &&IF_ICMPGT_label,
        &&IF_ICMPLE_label,
        &&IF_ACMPEQ_label,
        &&IF_ACMPNE_label,
        &&GOTO_label,
        &&LDC_REF_label,
        &&LDC_W_REF_label,
        &&TABLESWITCH_label,
        &&LOOKUPSWITCH_label,
        &&IRETURN_label,
        &&LRETURN_label,
#if JEL_FP_SUPPORT
        &&FRETURN_label,
        &&DRETURN_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ARETURN_label,
        &&RETURN_label,
        &&GETSTATIC_PRELINK_label,
        &&PUTSTATIC_PRELINK_label,
        &&GETFIELD_PRELINK_label,
        &&PUTFIELD_PRELINK_label,
        &&INVOKEVIRTUAL_label,
        &&INVOKESPECIAL_label,
        &&INVOKESTATIC_label,
        &&INVOKEINTERFACE_label,
        &&INVOKESUPER_label,
        &&NEW_label,
        &&NEWARRAY_label,
        &&ANEWARRAY_label,
        &&ARRAYLENGTH_label,
        &&ATHROW_label,
        &&CHECKCAST_label,
        &&INSTANCEOF_label,
        &&MONITORENTER_label,
        &&MONITOREXIT_label,
        &&WIDE_label,
        &&MULTIANEWARRAY_label,
        &&IFNULL_label,
        &&IFNONNULL_label,
        &&GOTO_W_label,
        &&GETSTATIC_BYTE_label,
        &&GETSTATIC_CHAR_label,
        &&GETSTATIC_SHORT_label,
        &&GETSTATIC_INT_label,
#if JEL_FP_SUPPORT
        &&GETSTATIC_FLOAT_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&GETSTATIC_LONG_label,
#if JEL_FP_SUPPORT
        &&GETSTATIC_DOUBLE_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&GETSTATIC_REFERENCE_label,
        &&PUTSTATIC_BYTE_label,
        &&PUTSTATIC_BOOL_label,
        &&PUTSTATIC_CHAR_label,
        &&PUTSTATIC_INT_label,
#if JEL_FP_SUPPORT
        &&PUTSTATIC_FLOAT_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&PUTSTATIC_LONG_label,
#if JEL_FP_SUPPORT
        &&PUTSTATIC_DOUBLE_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&PUTSTATIC_REFERENCE_label,
        &&GETFIELD_BYTE_label,
        &&GETFIELD_BOOL_label,
        &&GETFIELD_CHAR_label,
        &&GETFIELD_SHORT_label,
        &&GETFIELD_INT_label,
#if JEL_FP_SUPPORT
        &&GETFIELD_FLOAT_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&GETFIELD_LONG_label,
#if JEL_FP_SUPPORT
        &&GETFIELD_DOUBLE_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&GETFIELD_REFERENCE_label,
        &&PUTFIELD_BYTE_label,
        &&PUTFIELD_BOOL_label,
        &&PUTFIELD_CHAR_label,
        &&PUTFIELD_INT_label,
#if JEL_FP_SUPPORT
        &&PUTFIELD_FLOAT_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&PUTFIELD_LONG_label,
#if JEL_FP_SUPPORT
        &&PUTFIELD_DOUBLE_label,
#else
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&PUTFIELD_REFERENCE_label,
        &&INVOKEVIRTUAL_PRELINK_label,
        &&INVOKESPECIAL_PRELINK_label,
        &&INVOKESTATIC_PRELINK_label,
        &&INVOKEINTERFACE_PRELINK_label,
        &&NEW_PRELINK_label,
        &&NEWARRAY_PRELINK_label,
        &&ANEWARRAY_PRELINK_label,
        &&CHECKCAST_PRELINK_label,
        &&INSTANCEOF_PRELINK_label,
        &&MULTIANEWARRAY_PRELINK_label,
        &&MONITORENTER_SPECIAL_label,
        &&MONITORENTER_SPECIAL_STATIC_label,
        &&IRETURN_MONITOREXIT_label,
        &&LRETURN_MONITOREXIT_label,
#if JEL_FP_SUPPORT
        &&FRETURN_MONITOREXIT_label,
        &&DRETURN_MONITOREXIT_label,
#else
        &&NOP_label,
        &&NOP_label,
#endif // JEL_FP_SUPPORT
        &&ARETURN_MONITOREXIT_label,
        &&RETURN_MONITOREXIT_label,
#if JEL_FINALIZER
        &&NEW_FINALIZER_label,
#else
        &&NOP_label,
#endif // JEL_FINALIZER
        &&LDC_PRELINK_label,
        &&LDC_W_PRELINK_label
    };
#endif // JEL_THREADED_INTERPRETER

    // Set the runtime state from the main method
    thread = thread_self();
    prepare_for_call(thread, main_method);
    pc = main_method->code;
    sp = thread->sp;
    fp = (stack_frame_t *) thread->fp;
    locals = fp->locals;
    cp = main_method->cp->data;

    print_method_call(thread, main_method);

    INTERPRETER_PROLOG

    OPCODE(NOP)
        pc++;
        DISPATCH;

    OPCODE(ACONST_NULL)
        *((uintptr_t *) sp) = JNULL;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_M1)
        *((int32_t *) sp) = -1;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_0)
        *((int32_t *) sp) = 0;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_1)
        *((int32_t *) sp) = 1;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_2)
        *((int32_t *) sp) = 2;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_3)
        *((int32_t *) sp) = 3;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_4)
        *((int32_t *) sp) = 4;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ICONST_5)
        *((int32_t *) sp) = 5;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(LCONST_0)
        *((int64_t *) sp) = (int64_t) 0;
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(LCONST_1)
        *((int64_t *) sp) = (int64_t) 1;
        sp += 2;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FCONST_0)
        *((float *) sp) = 0.0f;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(FCONST_1)
        *((float *) sp) = 1.0f;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(FCONST_2)
        *((float *) sp) = 2.0f;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(DCONST_0)
        *((double *) sp) = 0.0;
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DCONST_1)
        *((double *) sp) = 1.0;
        sp += 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(BIPUSH)
        *((int32_t *) sp) = (int32_t) *((int8_t *) (pc + 1));
        sp++;
        pc += 2;
        DISPATCH;

    OPCODE(SIPUSH)
        *((int32_t *) sp) = (int32_t) load_int16_un(pc + 1);
        sp++;
        pc += 3;
        DISPATCH;

    OPCODE(LDC)
        *((int32_t *) sp) = cp_data_get_int32(cp, *(pc + 1));
        sp++;
        pc += 2;
        DISPATCH;

    OPCODE(LDC_W)
        *((int32_t *) sp) = cp_data_get_int32(cp, load_uint16_un(pc + 1));
        sp++;
        pc += 3;
        DISPATCH;

    OPCODE(LDC2_W)
        *((int64_t *) sp) = cp_data_get_int64(cp, load_uint16_un(pc + 1));
        sp += 2;
        pc += 3;
        DISPATCH;

    OPCODE(ILOAD)
        *((int32_t *) sp) = *((int32_t *) (locals + *(pc + 1)));
        sp++;
        pc += 2;
        DISPATCH;

    OPCODE(LLOAD)
        *((int64_t *) sp) = *((int64_t *) (locals + *(pc + 1)));
        sp += 2;
        pc += 2;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FLOAD)
        *((float *) sp) = *((float *) (locals + *(pc + 1)));
        sp++;
        pc += 2;
        DISPATCH;

    OPCODE(DLOAD)
        *((double *) sp) = *((double *) (locals + *(pc + 1)));
        sp += 2;
        pc += 2;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(ALOAD)
        *((uintptr_t *) sp) = *((uintptr_t *) (locals + *(pc + 1)));
        sp++;
        pc += 2;
        DISPATCH;

    OPCODE(ILOAD_0)
        *((int32_t *) sp) = *((int32_t *) locals);
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ILOAD_1)
        *((int32_t *) sp) = *((int32_t *) (locals + 1));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ILOAD_2)
        *((int32_t *) sp) = *((int32_t *) (locals + 2));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ILOAD_3)
        *((int32_t *) sp) = *((int32_t *) (locals + 3));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(LLOAD_0)
        *((int64_t *) sp) = *((int64_t *) locals);
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(LLOAD_1)
        *((int64_t *) sp) = *((int64_t *) (locals + 1));
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(LLOAD_2)
        *((int64_t *) sp) = *((int64_t *) (locals + 2));
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(LLOAD_3)
        *((int64_t *) sp) = *((int64_t *) (locals + 3));
        sp += 2;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FLOAD_0)
        *((float *) sp) = *((float *) locals);
        sp++;
        pc++;
        DISPATCH;

    OPCODE(FLOAD_1)
        *((float *) sp) = *((float *) (locals + 1));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(FLOAD_2)
        *((float *) sp) = *((float *) (locals + 2));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(FLOAD_3)
        *((float *) sp) = *((float *) (locals + 3));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(DLOAD_0)
        *((double *) sp) = *((double *) locals);
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DLOAD_1)
        *((double *) sp) = *((double *) (locals + 1));
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DLOAD_2)
        *((double *) sp) = *((double *) (locals + 2));
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DLOAD_3)
        *((double *) sp) = *((double *) (locals + 3));
        sp += 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(ALOAD_0)
        *((uintptr_t *) sp) = *((uintptr_t *) locals);
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ALOAD_1)
        *((uintptr_t *) sp) = *((uintptr_t *) (locals + 1));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ALOAD_2)
        *((uintptr_t *) sp) = *((uintptr_t *) (locals + 2));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(ALOAD_3)
        *((uintptr_t *) sp) = *((uintptr_t *) (locals + 3));
        sp++;
        pc++;
        DISPATCH;

    /*
     * Array indices should be a signed 32-bit integers but arrays of
     * negative length are not allowed so we treat negative numbers
     * as large unsigned integers which will fail the index check
     * anyway
     */

    OPCODE(IALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        int32_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((int32_t *) (sp - 2)) = *(data + index);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(LALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        int64_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((int64_t *) (sp - 2)) = *(data + index);
        pc++;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        float *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((float *) (sp - 2)) = *(data + index);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(DALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        double *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((double *) (sp - 2)) = *(data + index);
        pc++;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(AALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        uintptr_t *data = array_ref_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        // Reference arrays grow backwards!
        *((uintptr_t *) (sp - 2)) = *(data - index);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(BALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        int8_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        if (header_get_class(&(array->header))->elem_type == PT_BYTE) {
            *((uint32_t *) (sp - 2)) = *(data + index);
        } else {
            *((uint32_t *) (sp - 2)) = (*(data + (index >> 3)) >> (index & 0x7))
                                       & 0x1;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(CALOAD) {
        uint32_t index = *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        uint16_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((uint32_t *) (sp - 2)) = *(data + index);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(SALOAD) {
        uint32_t index =  *((uint32_t *) (sp - 1));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 2));
        int16_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *((int32_t *) (sp - 2)) = (int32_t) *(data + index);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(ISTORE) {
        uint8_t index = *(pc + 1);

        *((int32_t *) (locals + index)) = *((int32_t *) (sp - 1));
        sp--;
        pc += 2;
        DISPATCH;
    }

    OPCODE(LSTORE) {
        uint8_t index = *(pc + 1);

        *((int64_t *) (locals + index)) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc += 2;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FSTORE) {
        uint8_t index = *(pc + 1);

        *((float *) (locals + index)) = *((float *) (sp - 1));
        sp--;
        pc += 2;
        DISPATCH;
    }

    OPCODE(DSTORE) {
        uint8_t index = *(pc + 1);

        *((double *) (locals + index)) = *((double *) (sp - 2));
        sp -= 2;
        pc += 2;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(ASTORE) {
        uint8_t index = *(pc + 1);

        *((uintptr_t *) (locals + index)) = *((uintptr_t *) (sp - 1));
        sp--;
        pc += 2;
        DISPATCH;
    }

    OPCODE(ISTORE_0)
        *((int32_t *) locals) = *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ISTORE_1)
        *((int32_t *) (locals + 1)) = *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ISTORE_2)
        *((int32_t *) (locals + 2)) = *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ISTORE_3)
        *((int32_t *) (locals + 3)) = *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LSTORE_0)
        *((int64_t *) locals) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(LSTORE_1)
        *((int64_t *) (locals + 1)) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(LSTORE_2)
        *((int64_t *) (locals + 2)) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(LSTORE_3)
        *((int64_t *) (locals + 3)) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;


#if JEL_FP_SUPPORT

    OPCODE(FSTORE_0)
        *((float *) locals) = *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(FSTORE_1)
        *((float *) (locals + 1)) = *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(FSTORE_2)
        *((float *) (locals + 2)) = *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(FSTORE_3)
        *((float *) (locals + 3)) = *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(DSTORE_0)
        *((double *) locals) = *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(DSTORE_1)
        *((double *) (locals + 1)) = *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(DSTORE_2)
        *((double *) (locals + 2)) = *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(DSTORE_3)
        *((double *) (locals + 3)) = *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(ASTORE_0)
        *((uintptr_t *) locals) = *((uintptr_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ASTORE_1)
        *((uintptr_t *) (locals + 1)) = *((uintptr_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ASTORE_2)
        *((uintptr_t *) (locals + 2)) = *((uintptr_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ASTORE_3)
        *((uintptr_t *) (locals + 3)) = *((uintptr_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(IASTORE) {
        uint32_t index = *((uint32_t *) (sp - 2));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        int32_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((int32_t *) (sp - 1));
        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(LASTORE) {
        uint32_t index = *((uint32_t *) (sp - 3));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 4));
        int64_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((int64_t *) (sp - 2));
        sp -= 4;
        pc++;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FASTORE) {
        uint32_t index = *((uint32_t *) (sp - 2));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        float *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((float *) (sp - 1));
        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(DASTORE) {
        uint32_t index = *((uint32_t *) (sp - 3));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 4));
        double *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((double *) (sp - 2));
        sp -= 4;
        pc++;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(AASTORE) {
        class_t *src;
        class_t *dest;
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        uintptr_t *data = array_ref_get_data(array);
        uint32_t index = *((uint32_t *) (sp - 2));
        uintptr_t value;

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        value = *((uintptr_t *) (sp - 1));

        if (value == JNULL) {
            *(data - index) = JNULL;
        } else {
            header_t *header = (header_t *) value;

            src = header_get_class(header);
            dest = header_get_class(&(array->header))->elem_class;

            if ((src == dest) || bcl_is_assignable(src, dest)) {
                *(data - index) = value;
            } else {
                goto throw_arraystoreexception;
            }
        }

        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(BASTORE)
    {
        uint32_t index = *((uint32_t *) (sp - 2));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        uint8_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        if (header_get_class(&(array->header))->elem_type == PT_BYTE) {
            *(data + index) = *((int32_t *) (sp - 1));
        } else {
            uint32_t value = *((int32_t *) (sp - 1)) & 0x1;
            uint32_t nvalue = value ^ 0x1;

            *(data + (index >> 3)) |= (value << (index & 0x7));
            *(data + (index >> 3)) &= ~(nvalue << (index & 0x7));
        }

        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(CASTORE) {
        uint32_t index = *((uint32_t *) (sp - 2));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        uint16_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((int32_t *) (sp - 1));
        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(SASTORE) {
        uint32_t index = *((uint32_t *) (sp - 2));
        array_t *array = (array_t *) *((uintptr_t *) (sp - 3));
        uint16_t *data = array_get_data(array);

        if (array == NULL) {
            goto throw_nullpointerexception;
        } else if (index >= array_length(array)) {
            goto throw_arrayindexoutofboundsexception;
        }

        *(data + index) = *((int32_t *) (sp - 1));
        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(POP)
        sp--;
        pc++;
        DISPATCH;

    OPCODE(POP2)
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(DUP)
        *sp = *(sp - 1);
        sp++;
        pc++;
        DISPATCH;


    OPCODE(DUP_X1)
        *sp = *(sp - 1);
        *(sp - 1) = *(sp - 2);
        *(sp - 2) = *sp;
        sp++;
        pc++;
        DISPATCH;


    OPCODE(DUP_X2)
        *sp = *(sp - 1);
        *(sp - 1) = *(sp - 2);
        *(sp - 2) = *(sp - 3);
        *(sp - 3) = *sp;
        sp++;
        pc++;
        DISPATCH;

    OPCODE(DUP2)
        *(sp + 1) = *(sp - 1);
        *sp = *(sp - 2);
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DUP2_X1)
        *(sp + 1) = *(sp - 1);
        *sp = *(sp - 2);
        *(sp - 1) = *(sp - 3);
        *(sp - 2) = *(sp + 1);
        *(sp - 3) = *sp;
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(DUP2_X2)
        *(sp + 1) = *(sp - 1);
        *sp = *(sp - 2);
        *(sp - 1) = *(sp - 3);
        *(sp - 2) = *(sp - 4);
        *(sp - 3) = *(sp + 1);
        *(sp - 4) = *sp;
        sp += 2;
        pc++;
        DISPATCH;

    OPCODE(SWAP) {
        jword_t temp = *(sp - 1);

        *(sp - 1) = *(sp - 2);
        *(sp - 2) = temp;
        pc++;
        DISPATCH;
    }

    OPCODE(IADD)
        *((int32_t *) (sp - 2)) += *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LADD)
        *((int64_t *) (sp - 4)) += *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FADD)
        *((float *) (sp - 2)) += *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(DADD)
        *((double *) (sp - 4)) += *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(ISUB)
        *((int32_t *) (sp - 2)) -= *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;


    OPCODE(LSUB)
        *((int64_t *) (sp - 4)) -= *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FSUB)
        *((float *) (sp - 2)) -= *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(DSUB)
        *((double *) (sp - 4)) -= *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(IMUL)
        *((int32_t *) (sp - 2)) *= *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LMUL)
        *((int64_t *) (sp - 4)) *= *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FMUL)
        *((float *) (sp - 2)) *= *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(DMUL)
        *((double *) (sp - 4)) *= *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(IDIV) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));

        if (value2 == 0) {
            goto throw_arithmeticexception;
        } else if ((value1 == (int32_t) 0x80000000) && (value2 == -1)) {
            *((int32_t *) (sp - 2)) = value1;
        } else {
            *((int32_t *) (sp - 2)) = value1 / value2;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(LDIV) {
        int64_t value1 = *((int64_t *) (sp - 4));
        int64_t value2 = *((int64_t *) (sp - 2));

        if (value2 == 0) {
            goto throw_arithmeticexception;
        }

        *((int64_t *) (sp - 4)) = value1 / value2;
        sp -= 2;
        pc++;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FDIV)
        *((float *) (sp - 2)) /= *((float *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(DDIV)
        *((double *) (sp - 4)) /= *((double *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(IREM) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));

        if (value2 == 0) {
            goto throw_arithmeticexception;
        } else if ((value1 == (int32_t) 0x80000000) && (value2 == -1)) {
            *((int32_t *) (sp - 2)) = 0;
        } else {
            *((int32_t *) (sp - 2)) = value1 % value2;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(LREM) {
        int64_t value1 = *((int64_t *) (sp - 4));
        int64_t value2 = *((int64_t *) (sp - 2));

        if (value2 == 0) {
            goto throw_arithmeticexception;
        }

        *((int64_t *) (sp - 4)) = value1 % value2;
        sp -= 2;
        pc++;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FREM) {
        float value1 = *((float *) (sp - 2));
        float value2 = *((float *) (sp - 1));

        *((float *) (sp - 2)) = (value2 == 0.0f) ? NAN : fmod(value1, value2);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(DREM) {
        double value1 = *((double *) (sp - 4));
        double value2 = *((double *) (sp - 2));

        *((double *) (sp - 4)) = (value2 == 0.0) ? NAN : fmod(value1, value2);
        sp -= 2;
        pc++;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(INEG)
        *((int32_t *) (sp - 1)) = - *((int32_t *) (sp - 1));
        pc++;
        DISPATCH;

    OPCODE(LNEG)
        *((int64_t *) (sp - 2)) = - *((int64_t *) (sp - 2));
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(FNEG)
        *((float *) (sp - 1)) = - *((float *) (sp - 1));
        pc++;
        DISPATCH;

    OPCODE(DNEG)
        *((double *) (sp - 2)) = - *((double *) (sp - 2));
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(ISHL)
        *((int32_t *) (sp - 2)) <<= *((int32_t *) (sp - 1)) & 0x1f;
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LSHL)
        *((int64_t *) (sp - 3)) <<= *((int32_t *) (sp - 1)) & 0x3f;
        sp--;
        pc++;
        DISPATCH;

    OPCODE(ISHR)
        *((int32_t *) (sp - 2)) >>= *((int32_t *) (sp - 1)) & 0x1f;
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LSHR)
        *((int64_t *) (sp - 3)) >>= *((int32_t *) (sp - 1)) & 0x3f;
        sp--;
        pc++;
        DISPATCH;

    OPCODE(IUSHR) {
        uint32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));

        *((int32_t *) (sp - 2)) = value1 >> (value2 & 0x1f);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(LUSHR) {
        uint64_t value1 = *((int64_t *) (sp - 3));
        int32_t value2 = *((int32_t *) (sp - 1));

        *((int64_t *) (sp - 3)) = value1 >> (value2 & 0x3f);
        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(IAND)
        *((int32_t *) (sp - 2)) &= *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LAND)
        *((int64_t *) (sp - 4)) &= *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(IOR)
        *((int32_t *) (sp - 2)) |= *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LOR)
        *((int64_t *) (sp - 4)) |= *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(IXOR)
        *((int32_t *) (sp - 2)) ^= *((int32_t *) (sp - 1));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(LXOR)
        *((int64_t *) (sp - 4)) ^= *((int64_t *) (sp - 2));
        sp -= 2;
        pc++;
        DISPATCH;

    OPCODE(IINC) {
        uint8_t index = *(pc + 1);

        *((int32_t *) (locals + index)) += (int32_t) *((int8_t *) (pc + 2));
        pc += 3;
        DISPATCH;
    }

    OPCODE(I2L)
        *((int64_t *) (sp - 1)) = (int64_t) *((int32_t *) (sp - 1));
        sp++;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(I2F)
        *((float *) (sp - 1)) = (float) *((int32_t *) (sp - 1));
        pc++;
        DISPATCH;

    OPCODE(I2D)
        *((double *) (sp - 1)) = (double) *((int32_t *) (sp - 1));
        sp++;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(L2I)
        *((int32_t *) (sp - 2)) = (int32_t) *((int64_t *) (sp - 2));
        sp--;
        pc++;
        DISPATCH;

#if JEL_FP_SUPPORT

    OPCODE(L2F)
        *((float *) (sp - 2)) = (float) *((int64_t *) (sp - 2));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(L2D)
        *((double *) (sp - 2)) = (double) *((int64_t *) (sp - 2));
        pc++;
        DISPATCH;

    OPCODE(F2I)
        *((int32_t *) (sp - 1)) = (int32_t) *((float *) (sp - 1));
        pc++;
        DISPATCH;

    OPCODE(F2L)
        *((int64_t *) (sp - 1)) = (int64_t) *((float *) (sp - 1));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(F2D)
        *((double *) (sp - 1)) = (double) *((float *) (sp - 1));
        sp++;
        pc++;
        DISPATCH;

    OPCODE(D2I)
        *((int32_t *) (sp - 2)) = (int32_t) *((double *) (sp - 2));
        sp--;
        pc++;
        DISPATCH;

    OPCODE(D2L)
        *((int64_t *) (sp - 2)) = (int64_t) *((double *) (sp - 2));
        pc++;
        DISPATCH;

    OPCODE(D2F)
        *((float *) (sp - 2)) = (float) *((double *) (sp - 2));
        sp--;
        pc++;
        DISPATCH;

#endif // JEL_FP_SUPPORT

    OPCODE(I2B) {
        int8_t value1 = *((int32_t *) (sp - 1));

        *((int32_t *) (sp - 1)) = (int32_t) value1;
        pc++;
        DISPATCH;
    }

    OPCODE(I2C)
        *((int32_t *) (sp - 1)) &= 0xffff;
        pc++;
        DISPATCH;

    OPCODE(I2S) {
        int16_t value1 = *((int32_t *) (sp - 1));

        *((int32_t *) (sp - 1)) = (int32_t) value1;
        pc++;
        DISPATCH;
    }

    OPCODE(LCMP) {
        int64_t value1 = *((int64_t *) (sp - 4));
        int64_t value2 = *((int64_t *) (sp - 2));
        int64_t t = value1 - value2;

        /* HACK: This is a strict-aliasing violation but it should pose no
         * problem for the value to be written depends on the aliasing load */
        *((int32_t *) (sp - 4)) = (t == 0) ? 0 : ((t >> 63) | 1);
        sp -= 3;
        pc++;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FCMPL) {
        float value1 = *((float *) (sp - 2));
        float value2 = *((float *) (sp - 1));

        /* HACK: This is a strict-aliasing violation but it should pose no
         * problem for the value to be written depends on the aliasing load */
        if (value1 != value1 || value2 != value2) {
            *((int32_t *) (sp - 2)) = -1;
        } else if (value1 == value2) {
            *((int32_t *) (sp - 2)) = 0;
        } else if (value1 > value2) {
            *((int32_t *) (sp - 2)) = 1;
        } else {
            *((int32_t *) (sp - 2)) = -1;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(FCMPG) {
        float value1 = *((float *) (sp - 2));
        float value2 = *((float *) (sp - 1));

        /* HACK: This is a strict-aliasing violation but it should pose no
         * problem for the value to be written depends on the aliasing load */
        if (value1 != value1 || value2 != value2) {
            *((int32_t *) (sp - 2)) = 1;
        } else if (value1 == value2) {
            *((int32_t *) (sp - 2)) = 0;
        } else if (value1 > value2) {
            *((int32_t *) (sp - 2)) = 1;
        } else {
            *((int32_t *) (sp - 2)) = -1;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(DCMPL) {
        double value1 = *((double *) (sp - 4));
        double value2 = *((double *) (sp - 2));

        /* HACK: This is a strict-aliasing violation but it should pose no
         * problem for the value to be written depends on the aliasing load */
        if (value1 != value1 || value2 != value2) {
            *((int32_t *) (sp - 4)) = -1;
        } else if (value1 == value2) {
            *((int32_t *) (sp - 4)) = 0;
        } else if (value1 > value2) {
            *((int32_t *) (sp - 4)) = 1;
        } else {
            *((int32_t *) (sp - 4)) = -1;
        }

        sp -= 3;
        pc++;
        DISPATCH;
    }

    OPCODE(DCMPG) {
        double value1 = *((double *) (sp - 4));
        double value2 = *((double *) (sp - 2));

        /* HACK: This is a strict-aliasing violation but it should pose no
         * problem for the value to be written depends on the aliasing load */
        if (value1 != value1 || value2 != value2) {
            *((int32_t *) (sp - 4)) = 1;
        } else if (value1 == value2) {
            *((int32_t *) (sp - 4)) = 0;
        } else if (value1 > value2) {
            *((int32_t *) (sp - 4)) = 1;
        } else {
            *((int32_t *) (sp - 4)) = -1;
        }

        sp -= 3;
        pc++;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(IFEQ) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value == 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFNE) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value != 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFLT) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value < 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFGE) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value >= 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFGT) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value > 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFLE) {
        int32_t value = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (value <= 0) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPEQ) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 == value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPNE) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 != value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPLT) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 < value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPGE) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 >= value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPGT) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 > value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ICMPLE) {
        int32_t value1 = *((int32_t *) (sp - 2));
        int32_t value2 = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 <= value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ACMPEQ) {
        uintptr_t value1 = *((uintptr_t *) (sp - 2));
        uintptr_t value2 = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 == value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IF_ACMPNE) {
        uintptr_t value1 = *((uintptr_t *) (sp - 2));
        uintptr_t value2 = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp -= 2;
        pc += (value1 != value2) ? offset : 3;
        DISPATCH;
    }

    OPCODE(GOTO) {
        int16_t offset = load_int16_un(pc + 1);

        pc += offset;
        DISPATCH;
    }

    OPCODE(LDC_REF) {
        uint8_t index = *(pc + 1);

        *((uintptr_t *) sp) = cp_data_get_uintptr(cp, index);
        sp++;
        pc += 2;
        DISPATCH;
    }

    OPCODE(LDC_W_REF) {
        uint16_t index = load_uint16_un(pc + 1);

        *((uintptr_t *) sp) = cp_data_get_uintptr(cp, index);
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(TABLESWITCH) {
        int32_t *aligned_ptr = (int32_t *) size_ceil((uintptr_t) pc + 1, 4);
        int32_t default_offset = aligned_ptr[0];
        int32_t low = aligned_ptr[1];
        int32_t high = aligned_ptr[2];
        int32_t index = *((int32_t *) (sp - 1));

        aligned_ptr += 3;
        sp--;

        if (index < low || index > high) {
            pc += default_offset;
            DISPATCH;
        } else {
            pc += aligned_ptr[index - low];
            DISPATCH;
        }
    }

    OPCODE(LOOKUPSWITCH) {
        int32_t *aligned_ptr = (int32_t *) size_ceil((uintptr_t) pc + 1, 4);
        int32_t default_offset = aligned_ptr[0];
        int32_t n_pairs = aligned_ptr[1];
        int32_t key = *((int32_t *) (sp - 1));
        int32_t i;

        aligned_ptr += 2;
        sp--;

        for (i = 0; i < n_pairs; i++) {
            if (key == aligned_ptr[i * 2]) {
                break;
            }
        }

        pc += (i == n_pairs) ? default_offset : aligned_ptr[(i * 2) + 1];
        DISPATCH;
    }

    OPCODE(IRETURN) {
        int32_t ret_value = *((int32_t *) (sp - 1)); // Pop the return value

        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((int32_t *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(LRETURN) {
        int64_t ret_value = *((int64_t *) (sp - 2)); // Pop the return value

        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals + 2;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((int64_t *) (sp - 2)) = ret_value;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FRETURN) {
        float ret_value = *((float *) (sp - 1)); // Pop the return value

        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((float *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(DRETURN) {
        double ret_value = *((double *) (sp - 2)); // Pop the return value

        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals + 2;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

       // Push the return value on the stack
        *((double *) (sp - 2)) = ret_value;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(ARETURN) {
        // Pop the return value
        uintptr_t ret_value = *((uintptr_t *) (sp - 1));

        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((uintptr_t *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(RETURN) {
        print_method_ret(thread, fp->method);

        // Pop the current frame
        sp = locals;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;
        DISPATCH;
    }

    OPCODE(INVOKEVIRTUAL) {
        uint16_t offset, index;
        uintptr_t ref;
        class_t *new_cl;
        method_t *new_method;

        index = load_uint16_un(pc + 1);
        offset = method_unpack_arguments(index);
        index = method_unpack_index(index);
        locals = sp - offset;
        ref = *((uintptr_t *) locals);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        /* Fetch the new class from the obejct on top of the stack and the new
         * method from the class' dispatch table */
        new_cl = header_get_class((header_t *) ref);
        new_method = new_cl->dtable[index];

        print_method_call(thread, new_method);

        // Check if we are not overflowing the stack
        if ((locals + new_method->max_locals + new_method->max_stack)
            > (jword_t *) (fp - 1))
        {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Stack overflow, try using a larger stack with the "
                    "--stack-size parameter");
        }

        // Push a new stack frame
        fp->pc = pc + 3;
        fp--;
        fp->cl = new_cl;
        fp->method = new_method;
        fp->locals = locals;

        // Install the new state in the local variables
        cp = new_method->cp->data;
        sp = locals + new_method->max_locals;
        pc = new_method->code;
        DISPATCH;
    }

    OPCODE(INVOKESPECIAL) {
        uint16_t offset;
        uint16_t index;
        uintptr_t ref;
        method_t *new_method;

        index = load_uint16_un(pc + 1);
        new_method = (method_t *) cp_data_get_ptr(cp, index);
        offset = new_method->args_size;
        locals = sp - offset;
        ref = *((uintptr_t *) locals);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        print_method_call(thread, new_method);

        // Check if we are not overflowing the stack
        if ((locals + new_method->max_locals + new_method->max_stack)
            > (jword_t *) (fp - 1))
        {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Stack overflow, try using a larger stack with the "
                    "--stack-size parameter\n");
        }

        // Push a new stack frame
        fp->pc = pc + 3;
        fp--;
        fp->cl = cp_get_class(new_method->cp);
        fp->method = new_method;
        fp->locals = locals;

        // Install the new state in the local variables
        cp = new_method->cp->data;
        sp = locals + new_method->max_locals;
        pc = new_method->code;
        DISPATCH;
    }

    OPCODE(INVOKESTATIC) {
        uint16_t offset, index;
        method_t *new_method;

        index = load_uint16_un(pc + 1);
        new_method = (method_t *) cp_data_get_ptr(cp, index);
        offset = new_method->args_size;
        locals = sp - offset;

        print_method_call(thread, new_method);

        // Check if we are not overflowing the stack
        if ((locals + new_method->max_locals + new_method->max_stack)
            > (jword_t *) (fp - 1))
        {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Stack overflow, try using a larger stack with the "
                    "--stack-size parameter");
        }

        // Push a new stack frame
        fp->pc = pc + 3;
        fp--;
        fp->cl = cp_get_class(new_method->cp);
        fp->method = new_method;
        fp->locals = locals;

        // Install the new state in the local variables
        cp = new_method->cp->data;
        sp = locals + new_method->max_locals;
        pc = new_method->code;
        DISPATCH;
    }

    OPCODE(INVOKEINTERFACE) {
        uint16_t offset, index, match;
        uint32_t low, high, mid;
        uintptr_t ref;
        class_t *new_cl;
        method_t *new_method = NULL;

        index = load_uint16_un(pc + 1);
        offset = method_unpack_arguments(index);
        index = method_unpack_index(index);
        locals = sp -offset;
        ref = *((uintptr_t *) locals);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        new_cl = header_get_class((header_t *) ref); // Fetch the new class

        // Lookup the new method in the interface table
        low = 0;
        high = new_cl->itable_count;

        while (high >= low) {
            mid = (low + high) >> 1;
            match = new_cl->inames[mid];

            if (index == match) {
                break;
            } if (index > match) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        new_method = new_cl->itable[mid];
        print_method_call(thread, new_method);

        // Check if we are not overflowing the stack
        if ((locals + new_method->max_locals + new_method->max_stack)
            > (jword_t *) (fp - 1))
        {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Stack overflow, try using a larger stack with the "
                    "--stack-size parameter");
        }

        // Push a new stack frame
        fp->pc = pc + 3;
        fp--;
        fp->cl = new_cl;
        fp->method = new_method;
        fp->locals = locals;

        // Install the new state in the local variables
        cp = new_method->cp->data;
        sp = locals + new_method->max_locals;
        pc = new_method->code;
        DISPATCH;
    }

    OPCODE(INVOKESUPER) {
        uint16_t offset, index;
        uintptr_t ref;
        class_t *new_cl;
        method_t *new_method;

        index = load_uint16_un(pc + 1);
        offset = method_unpack_arguments(index);
        index = method_unpack_index(index);
        locals = sp - offset;
        ref = *((uintptr_t *) locals);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        /* Fetch the the new method from the current class' parent dispatch
         * table */
        new_cl = fp->cl->parent;
        new_method = new_cl->dtable[index];

        print_method_call(thread, new_method);

        // Check if we are not overflowing the stack
        if ((locals + new_method->max_locals + new_method->max_stack)
            > (jword_t *) (fp - 1))
        {
            c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                    "Stack overflow, try using a larger stack with the "
                    "--stack-size parameter");
        }

        // Push a new stack frame
        fp->pc = pc + 3;
        fp--;
        fp->cl = new_cl;
        fp->method = new_method;
        fp->locals = locals;

        // Install the new state in the local variables
        cp = new_method->cp->data;
        sp = locals + new_method->max_locals;
        pc = new_method->code;
        DISPATCH;

    }

    OPCODE(NEW) {
        uint16_t index = load_uint16_un(pc + 1);
        class_t *cl = (class_t *) cp_data_get_ptr(cp, index);

        SAVE_STATE;
        *((uintptr_t *) sp) = gc_new(cl);
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(NEWARRAY) {
        int32_t count = *((int32_t *) (sp - 1));
        uint8_t type = *(pc + 1);

        if (count < 0) {
            goto throw_negativearraysizeexception;
        }

        SAVE_STATE;
        *((uintptr_t *) (sp - 1)) = gc_new_array_nonref(type, count);
        pc += 2;
        DISPATCH;
    }

    OPCODE(ANEWARRAY) {
        class_t *cl;
        int32_t count = *((int32_t *) (sp - 1));
        uint16_t index = load_uint16_un(pc + 1);

        SAVE_STATE;
        cl = bcl_get_class_by_id(index);

        if (count < 0) {
            goto throw_negativearraysizeexception;
        }

        *((uintptr_t *) (sp - 1)) = gc_new_array_ref(cl, count);
        pc += 3;
        DISPATCH;
    }

    OPCODE(ARRAYLENGTH) {
        array_t *array = (array_t *) *((uintptr_t *) (sp - 1));

        if (array == NULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) = array_length(array);
        pc++;
        DISPATCH;
    }

    OPCODE(ATHROW) {
        uintptr_t exception_ref = *((uintptr_t *) (sp - 1));

        if (exception_ref == JNULL) {
            goto throw_nullpointerexception;
        }

        thread->exception = exception_ref;
        /* Do not update the pc, the current pc must be preserved so
         * that the exception handler can inspect it */
        goto exception_handler;
    }

    OPCODE(CHECKCAST) {
        uint16_t index = load_uint16_un(pc + 1);
        class_t *src;
        class_t *dest = (class_t *) cp_data_get_ptr(cp, index);
        header_t *header;
        uintptr_t ref = *((uintptr_t *) (sp - 1));

        if (ref != JNULL) {
            header = (header_t *) ref;
            src = header_get_class(header);

            if (src == dest) {
                ; // Shortcut
            } else if (!bcl_is_assignable(src, dest)) {
                goto throw_classcastexception;
            }
        }

        pc += 3;
        DISPATCH;
    }

    OPCODE(INSTANCEOF) {
        uint16_t index = load_uint16_un(pc + 1);
        class_t *src;
        class_t *dest = (class_t *) cp_data_get_ptr(cp, index);
        header_t *header;
        uintptr_t ref = *((uintptr_t *) (sp - 1));

        if (ref == JNULL) {
            *((int32_t *) (sp - 1)) = 0;
        } else {
            header = (header_t *) ref;
            src = header_get_class(header);

            if ((src == dest) || bcl_is_assignable(src, dest)) {
                *((int32_t *) (sp - 1)) = 1;
            } else {
                *((int32_t *) (sp - 1)) = 0;
            }
        }

        pc += 3;
        DISPATCH;
    }

    OPCODE(MONITORENTER) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        SAVE_STATE;
        monitor_enter(thread, ref);

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(MONITOREXIT) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        SAVE_STATE;

        if (!monitor_exit(thread, ref)) {
            goto throw_illegalmonitorstateexception;
        }

        sp--;
        pc++;
        DISPATCH;
    }

    OPCODE(WIDE) {
        uint16_t index;

        switch (*(pc + 1)) {
            case ILOAD:
                index = load_uint16_un(pc + 2);
                *((int32_t *) sp) = *((int32_t *) (locals + index));
                sp++;
                pc += 4;
                DISPATCH;

#if JEL_FP_SUPPORT

            case FLOAD:
                index = load_uint16_un(pc + 2);
                *((float *) sp) = *((float *) (locals + index));
                sp++;
                pc += 4;
                DISPATCH;

#endif // JEL_FP_SUPPORT

            case ALOAD:
                index = load_uint16_un(pc + 2);
                *((uintptr_t *) sp) = *((uintptr_t *) (locals + index));
                sp++;
                pc += 4;
                DISPATCH;

            case LLOAD:
                index = load_uint16_un(pc + 2);
                *((int64_t *) sp) = *((int64_t *) (locals + index));
                sp += 2;
                pc += 4;
                DISPATCH;

#if JEL_FP_SUPPORT

            case DLOAD:
                index = load_uint16_un(pc + 2);
                *((double *) sp) = *((double *) (locals + index));
                sp += 2;
                pc += 4;
                DISPATCH;

#endif // JEL_FP_SUPPORT

            case ISTORE:
                index = load_uint16_un(pc + 2);
                *((int32_t *) (locals + index)) = *((int32_t *) (sp - 1));
                sp--;
                pc += 4;
                DISPATCH;

#if JEL_FP_SUPPORT

            case FSTORE:
                index = load_uint16_un(pc + 2);
                *((float *) (locals + index)) = *((float *) (sp - 1));
                sp--;
                pc += 4;
                DISPATCH;

#endif // JEL_FP_SUPPORT

            case ASTORE:
                index = load_uint16_un(pc + 2);
                *((uintptr_t *) (locals + index)) = *((uintptr_t *) (sp - 1));
                sp--;
                pc += 4;
                DISPATCH;

            case LSTORE:
                index = load_uint16_un(pc + 2);
                *((int64_t *) (locals + index)) = *((int64_t *) (sp - 2));
                sp -= 2;
                pc += 4;
                DISPATCH;

#if JEL_FP_SUPPORT

            case DSTORE:
                index = load_uint16_un(pc + 2);
                *((double *) (locals + index)) = *((double *) (sp - 2));
                sp -= 2;
                pc += 4;
                DISPATCH;

#endif // JEL_FP_SUPPORT

            case IINC:
                index = load_uint16_un(pc + 2);
                *((int32_t *) (locals + index)) +=
                    (int32_t) load_int16_un(pc + 4);
                pc += 6;
                DISPATCH;

            case METHOD_LOAD:
                /* A recursive call to the interpreter may happen inside the
                 * bcl_link_method() call */
                SAVE_STATE;
                bcl_link_method(cp_get_class(fp->method->cp), fp->method);

                if (thread->exception != JNULL) {
                    goto exception_handler;
                }

                pc = fp->method->code;
                DISPATCH;

            case METHOD_ABSTRACT:
                c_throw(JAVA_LANG_VIRTUALMACHINEERROR,
                        "An abstract method was called");
                DISPATCH;

            case INVOKE_NATIVE:
            {
                method_t *native = fp->method;
                uintptr_t ref = JNULL;
                bool sync = method_is_synchronized(native);
                bool stat = method_is_static(native);
                native_proto_t func = native->data.function;

                SAVE_STATE;

                if (sync) {
                    if (stat) {
                        ref = class_get_object(fp->cl);
                    } else {
                        ref = *((uintptr_t *) locals);
                    }

                    monitor_enter(thread, ref);
                }

                /* HACK: Adjust the locals pointer because KNI indexes the
                 * arguments from 1 but static methods do not have an implicit
                 * argument */
                if (stat) {
                    fp->locals--;
                }

                switch (native->return_type) {
                    case RET_VOID:
                        func();
                        sp = locals;
                        break;

                    case RET_INT:
                        *((int32_t *) locals) = ((int32_t (*)( void )) func)();
                        sp = locals + 1;
                        break;

                    case RET_LONG:
                        *((int64_t *) locals) = ((int64_t (*)( void )) func)();
                        sp = locals + 2;
                        break;

                    case RET_OBJECT:
                        *((uintptr_t *) locals) =
                            ((uintptr_t (*)( void )) func)();
                        sp = locals + 1;
                        break;

#if JEL_FP_SUPPORT

                    case RET_FLOAT:
                        *((float *) locals) = ((float (*)( void )) func)();
                        sp = locals + 1;
                        break;

                    case RET_DOUBLE:
                        *((double *) locals) = ((double (*)( void )) func)();
                        sp = locals + 2;
                        break;

#endif // JEL_FP_SUPPORT

                    default: dbg_unreachable();
                }

                // HACK: Readjust the locals, see above
                if (stat) {
                    fp->locals++;
                }

                // Release the monitor if this method was synchronized
                if (sync) {
                    if (!monitor_exit(thread, ref)) {
                        /* The monitor realease failed the
                         * IllegalMonitorStateException overrides any other
                         * exceptions which might have been thrown */
                        goto throw_illegalmonitorstateexception;
                    }
                }

                // Check if an exception has been thrown
                if (thread->exception != JNULL) {
                    print_method_unwind(thread, native);
                    goto exception_handler;
                } else {
                    print_method_ret(thread, native);

                    // Pop the current frame
                    fp++;
                    cp = fp->method->cp->data;
                    pc = fp->pc;
                    locals = fp->locals;

                    // The return value has already been pushed on the stack
                    DISPATCH;
                }
            }

            case HALT:
                /* If the method terminates abruptly the 'exception' field of
                 * the thread structure will point to the uncaught exception.
                 * We also destroy the current stack frame as it is fake. */
                thread->fp = fp + 1;
                return;

            default:
                dbg_unreachable();
        }
    }

    OPCODE(MULTIANEWARRAY) {
        uint16_t index = load_uint16_un(pc + 1);
        class_t *cl = (class_t *) cp_data_get_ptr(cp, index);
        uintptr_t ref;
        uint8_t dimensions = *(pc + 3);

        SAVE_STATE;
        ref = gc_new_multiarray(cl, dimensions, sp - dimensions);
        *((uintptr_t *) (sp - dimensions)) = ref;
        sp = sp - dimensions + 1;
        pc += 4;
        DISPATCH;
    }

    OPCODE(IFNULL) {
        uintptr_t ref = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (ref == JNULL) ? offset : 3;
        DISPATCH;
    }

    OPCODE(IFNONNULL) {
        uintptr_t ref = *((int32_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        sp--;
        pc += (ref != JNULL) ? offset : 3;
        DISPATCH;
    }

    OPCODE(GOTO_W) {
        int32_t offset = load_int32_un(pc + 1);

        pc += offset;
        DISPATCH;
    }

    // OPCODE(GETSTATIC_BOOL)
    OPCODE(GETSTATIC_BYTE) {
        uint16_t index = load_uint16_un(pc + 1);

        /* We sign extend, though this shouldn't be done on booleans it does
         * no harm */
        *((int32_t *) sp) = *((int8_t *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETSTATIC_CHAR) {
        uint16_t index = load_uint16_un(pc + 1);

        // No sign extension, Java chars are unsigned
        *((uint32_t *) sp) = *((uint16_t *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETSTATIC_SHORT) {
        uint16_t index = load_uint16_un(pc + 1);

        // Sign extension for shorts is needed
        *((int32_t *) sp) = *((int16_t *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETSTATIC_INT) {
        uint16_t index = load_uint16_un(pc + 1);

        *((int32_t *) sp) = *((int32_t *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(GETSTATIC_FLOAT) {
        uint16_t index = load_uint16_un(pc + 1);

        *((float *) sp) = *((float *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(GETSTATIC_LONG) {
        uint16_t index = load_uint16_un(pc + 1);

        *((int64_t *) sp) = *((int64_t *) cp_data_get_ptr(cp, index));
        sp += 2;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT
    OPCODE(GETSTATIC_DOUBLE) {
        uint16_t index = load_uint16_un(pc + 1);

        *((double *) sp) = *((double *) cp_data_get_ptr(cp, index));
        sp += 2;
        pc += 3;
        DISPATCH;
    }
#endif // JEL_FP_SUPPORT

    OPCODE(GETSTATIC_REFERENCE) {
        uint16_t index = load_uint16_un(pc + 1);

        *((uintptr_t *) sp) = *((uintptr_t *) cp_data_get_ptr(cp, index));
        sp++;
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTSTATIC_BYTE) {
        uint16_t index = load_uint16_un(pc + 1);

        *((int8_t *) cp_data_get_ptr(cp, index)) = *((int32_t *) (sp - 1));
        sp--;
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTSTATIC_BOOL) {
        uint16_t index = load_uint16_un(pc + 1);

        *((uint8_t *) cp_data_get_ptr(cp, index)) =
            *((int32_t *) (sp - 1)) & 0x1;
        sp--;
        pc += 3;
        DISPATCH;
    }

    // OPCODE(PUTSTATIC_SHORT)
    OPCODE(PUTSTATIC_CHAR) {
        uint16_t index = load_uint16_un(pc + 1);

        *((uint16_t *) cp_data_get_ptr(cp, index)) = *((int32_t *) (sp - 1));
        sp--;
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTSTATIC_INT) {
        uint16_t index = load_uint16_un(pc + 1);

        *((int32_t *) cp_data_get_ptr(cp, index)) = *((int32_t *) (sp - 1));
        sp--;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(PUTSTATIC_FLOAT) {
        uint16_t index = load_uint16_un(pc + 1);

        *((float *) cp_data_get_ptr(cp, index)) = *((float *) (sp - 1));
        sp--;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(PUTSTATIC_LONG) {
        uint16_t index = load_uint16_un(pc + 1);

        *((int64_t *) cp_data_get_ptr(cp, index)) = *((int64_t *) (sp - 2));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(PUTSTATIC_DOUBLE) {
        uint16_t index = load_uint16_un(pc + 1);

        *((double *) cp_data_get_ptr(cp, index)) = *((double *) (sp - 2));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(PUTSTATIC_REFERENCE) {
        uint16_t index = load_uint16_un(pc + 1);

        *((uintptr_t *) cp_data_get_ptr(cp, index)) = *((uintptr_t *) (sp - 1));
        sp--;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_BYTE) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) = *((int8_t *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_BOOL) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) =
            (*((uint8_t *) (ref + (offset >> 3))) >> (offset & 0x7)) & 0x1;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_CHAR) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) = *((uint16_t *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_SHORT) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) = *((int16_t *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_INT) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (sp - 1)) = *((int32_t *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(GETFIELD_FLOAT) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((float *) (sp - 1)) = *((float *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(GETFIELD_LONG) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int64_t *) (sp - 1)) = *((int64_t *) (ref + offset));
        sp++;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(GETFIELD_DOUBLE) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((double *) (sp - 1)) = *((double *) (ref + offset));
        sp++;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(GETFIELD_REFERENCE) {
        uintptr_t ref = *((uintptr_t *) (sp - 1));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((uintptr_t *) (sp - 1)) = *((uintptr_t *) (ref + offset));
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTFIELD_BYTE) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int8_t *) (ref + offset)) = *((int32_t *) (sp - 1));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTFIELD_BOOL) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        uint32_t value;
        uint32_t nvalue;
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        value = *((int32_t *) (sp - 1)) & 0x1;
        nvalue = value ^ 0x1;
        *((uint8_t *) (ref + (offset >> 3))) |= (value << (offset & 0x7));
        *((uint8_t *) (ref + (offset >> 3))) &= ~(nvalue << (offset & 0x7));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

    // OPCODE(PUTFIELD_SHORT)
    OPCODE(PUTFIELD_CHAR) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int16_t *) (ref + offset)) = *((int32_t *) (sp - 1));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

    OPCODE(PUTFIELD_INT) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int32_t *) (ref + offset)) = *((int32_t *) (sp - 1));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(PUTFIELD_FLOAT) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((float *) (ref + offset)) = *((float *) (sp - 1));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(PUTFIELD_LONG) {
        uintptr_t ref = *((uintptr_t *) (sp - 3));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((int64_t *) (ref + offset)) = *((int64_t *) (sp - 2));
        sp -= 3;
        pc += 3;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(PUTFIELD_DOUBLE) {
        uintptr_t ref = *((uintptr_t *) (sp - 3));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((double *) (ref + offset)) = *((double *) (sp - 2));
        sp -= 3;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(PUTFIELD_REFERENCE) {
        uintptr_t ref = *((uintptr_t *) (sp - 2));
        int16_t offset = load_int16_un(pc + 1);

        if (ref == JNULL) {
            goto throw_nullpointerexception;
        }

        *((uintptr_t *) (ref + offset)) = *((uintptr_t *) (sp - 1));
        sp -= 2;
        pc += 3;
        DISPATCH;
    }

    OPCODE(GETFIELD_PRELINK)
    OPCODE(PUTFIELD_PRELINK)
    OPCODE(INVOKEVIRTUAL_PRELINK)
    OPCODE(INVOKESPECIAL_PRELINK)
    OPCODE(INVOKEINTERFACE_PRELINK)
    OPCODE(NEWARRAY_PRELINK)
    OPCODE(ANEWARRAY_PRELINK)
    OPCODE(CHECKCAST_PRELINK)
    OPCODE(INSTANCEOF_PRELINK)
    OPCODE(MULTIANEWARRAY_PRELINK)
    OPCODE(LDC_PRELINK)
    OPCODE(LDC_W_PRELINK)
        SAVE_STATE;
        pc = bcl_link_opcode(fp->method, pc, *pc);
        // Note that the current instruction will be replayed
        DISPATCH;

    OPCODE(GETSTATIC_PRELINK)
    OPCODE(PUTSTATIC_PRELINK)
    OPCODE(INVOKESTATIC_PRELINK)
    OPCODE(NEW_PRELINK)
        /* A recursive call to the interpreter may happen inside the
         * bcl_link_opcode() call */
        SAVE_STATE;
        pc = bcl_link_opcode(fp->method, pc, *pc);

        if (thread->exception != JNULL) {
            goto exception_handler;
        }

        // Note that the current instruction will be replayed
        DISPATCH;

    OPCODE(MONITORENTER_SPECIAL) {
        uintptr_t ref = *((uintptr_t *) locals);

        SAVE_STATE;
        monitor_enter(thread, ref);
        pc++;
        DISPATCH;
    }

    OPCODE(MONITORENTER_SPECIAL_STATIC) {
        class_t *cl = (class_t *) cp_data_get_ptr(cp, 0);
        uintptr_t ref = class_get_object(cl);

        SAVE_STATE;
        monitor_enter(thread, ref);
        pc++;
        DISPATCH;
    }

    OPCODE(IRETURN_MONITOREXIT) {
        bool res;
        int32_t ret_value;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the return value
        ret_value = *((int32_t *) (sp - 1));

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((int32_t *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(LRETURN_MONITOREXIT) {
        bool res;
        int64_t ret_value;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the return value
        ret_value = *((int64_t *) (sp - 2));

        // Pop the current frame
        sp = locals + 2;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((int64_t *) (sp - 2)) = ret_value;
        DISPATCH;
    }

#if JEL_FP_SUPPORT

    OPCODE(FRETURN_MONITOREXIT) {
        bool res;
        float ret_value;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the return value
        ret_value = *((float *) (sp - 1));

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((float *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(DRETURN_MONITOREXIT) {
        bool res;
        double ret_value;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the return value
        ret_value = *((double *) (sp - 2));

        // Pop the current frame
        sp = locals + 2;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((double *) (sp - 2)) = ret_value;
        DISPATCH;
    }

#endif // JEL_FP_SUPPORT

    OPCODE(ARETURN_MONITOREXIT) {
        bool res;
        uintptr_t ret_value;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the return value
        ret_value = *((uintptr_t *) (sp - 1));

        // Pop the current frame
        sp = locals + 1;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        // Push the return value on the stack
        *((uintptr_t *) (sp - 1)) = ret_value;
        DISPATCH;
    }

    OPCODE(RETURN_MONITOREXIT) {
        bool res;

        SAVE_STATE;
        print_method_ret(thread, fp->method);

        if (method_is_static(fp->method)) {
            uintptr_t cl_ref;
            class_t *cl;

            cl = (class_t *) cp_data_get_ptr(cp, 0);
            cl_ref = class_get_object(cl);
            res = monitor_exit(thread, cl_ref);
        } else {
            res = monitor_exit(thread, *((uintptr_t *) locals));
        }

        if (!res) {
            goto throw_illegalmonitorstateexception;
        }

        // Pop the current frame
        sp = locals;
        fp++;
        cp = fp->method->cp->data;
        pc = fp->pc;
        locals = fp->locals;

        DISPATCH;
    }

#if JEL_FINALIZER

    OPCODE(NEW_FINALIZER) {
        uint16_t index = load_uint16_un(pc + 1);
        class_t *cl = (class_t *) cp_data_get_ptr(cp, index);
        uintptr_t new_ref;

        SAVE_STATE;
        new_ref = gc_new(cl);
        gc_register_finalizable(new_ref);

        *((uintptr_t *) sp) = new_ref;
        sp++;
        pc += 3;
        DISPATCH;
    }

#endif // JEL_FINALIZER

    /*
     * In the following statements we will re-use the exception field of the
     * thread structure to hold an exception index instead of a Java object
     * reference. Once we are done with it we immediately set it to JNULL as
     * the GC excepts a real Java reference to be held in that field
     */

throw_arithmeticexception:
    thread->exception = (uintptr_t) "java/lang/ArithmeticException";
    goto throw_exception;

throw_arrayindexoutofboundsexception:
    thread->exception = (uintptr_t) "java/lang/ArrayIndexOutOfBoundsException";
    goto throw_exception;

throw_arraystoreexception:
    thread->exception = (uintptr_t) "java/lang/ArrayStoreException";
    goto throw_exception;

throw_classcastexception:
    thread->exception = (uintptr_t) "java/lang/ClassCastException";
    goto throw_exception;

throw_illegalmonitorstateexception:
    thread->exception = (uintptr_t) "java/lang/IllegalMonitorStateException";
    goto throw_exception;

throw_negativearraysizeexception:
    thread->exception = (uintptr_t) "java/lang/NegativeArraySizeException";
    goto throw_exception;

throw_nullpointerexception:
    thread->exception = (uintptr_t) "java/lang/NullPointerException";
    goto throw_exception;

throw_exception:
    {
        class_t *cl;
        uintptr_t exception_ref;
        const char *name = (const char *) thread->exception;

        SAVE_STATE;
        thread->exception = JNULL; // Needed because of the GC
        cl = bcl_resolve_class(fp->cl, name);

#if JEL_PRINT
        if (opts_get_print_opcodes()) {
            printf("Throwing class = %s\n", cl->name);
        }
#endif // JEL_PRINT

        // Create the exception and flag the current thread with it
        exception_ref = gc_new(cl);
        thread->exception = exception_ref;
    }

    goto exception_handler;

exception_handler:
    {
        class_t *catch_type;
        class_t *exception_type;
        method_t *method = fp->method;
        exception_handler_t *handlers = method->data.handlers;
        uintptr_t exception_ref = thread->exception;
        uint32_t i;
        uint32_t real_pc = pc - method->code;
        bool found = false;

        exception_type = header_get_class((header_t *) exception_ref);

        for (i = 0; i < method->exception_table_length; i++) {
            if ((real_pc >= handlers[i].start_pc)
                && (real_pc < handlers[i].end_pc))
            {
                catch_type = handlers[i].catch_type;

                if ((exception_type == catch_type)
                    || (class_is_parent(catch_type, exception_type)))
                {
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            /* We found an exception handler, install the new state in the
             * local variables and clear the thread exception */
            pc = handlers[i].handler_pc;

            if (method != &halt_method) {
                thread->exception = JNULL;
            }

            // Adjust the stack and push the exception on top of it
            sp = fp->locals + method->max_locals + 1;
            *((uintptr_t *) (sp - 1)) = exception_ref;

            // TODO: Create the stack trace
            DISPATCH;
        } else {
            // Abrupt method termination
            bool res = true;

            if (method != &halt_method) {
                print_method_unwind(thread, method);
            }

            if (method_is_synchronized(method)) {
                if (method_is_static(method)) {
                    res = monitor_exit(thread, class_get_object(fp->cl));
                } else {
                    res = monitor_exit(thread, *((uintptr_t *) fp->locals));
                }
            }

            /* Pop the current frame and substract one from the PC, it won't be
             * used directly but this trick is needed for the exception handler
             * of the caller method to work */
            fp++;
            cp = fp->method->cp->data;
            pc = fp->pc - 1;
            locals = fp->locals;

            if (!res) {
                /* If the monitor exit has failed ignore the current
                 * exception and throw an IllegalMonitorStateException
                 * instead */
                goto throw_illegalmonitorstateexception;
            } else {
                // Look for an appropriate handler in the popped method
                goto exception_handler;
            }
        }
    }

    INTERPRETER_EPILOG
} // interpreter()
