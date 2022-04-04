EVA_PATH        = ../eva
EVA_LIB         = ${EVA_PATH}/.build_release/libeva.a

include ${EVA_PATH}/eva.mk

# ------------------------------------------------------------------------------
# configurations
# ------------------------------------------------------------------------------

CMD             =  cmd
SRC             += ${CMD}

CFLAGS          += -I${EVA_PATH}/src -I${SRC}

LDFLAGS         += ${EVA_LIB}

# ------------------------------------------------------------------------------
# configurations, required by eva.mk
# ------------------------------------------------------------------------------
FMT_FOLDERS     =  ${SRC}

# ------------------------------------------------------------------------------
# libs
# ------------------------------------------------------------------------------
VM_HEADER       = ${INCLUDE}/vm.h ${VM_SRC}/op.h

VM_LIB          = ${BUILD}/vm_vm.o \
		  ${BUILD}/vm_shape.o \
		  ${BUILD}/vm_tensor.o \
                  ${BUILD}/vm_primitives.o

OBJ_HEADER      = ${INCLUDE}/obj.h
OBJ_LIB         = ${BUILD}/obj_obj.o

ALL_LIBS        =

# ------------------------------------------------------------------------------
# actions
# ------------------------------------------------------------------------------

.DEFAULT_GOAL   = compile

# incrementally compile all libraries.
compile: ${BUILD} ${ALL_LIBS}

# ------------------------------------------------------------------------------
# rules for sub-modules
# ------------------------------------------------------------------------------
${BUILD}/vm_%.o: ${VM_SRC}/%.c ${VM_HEADER}
	${EVA_CC} -o $@ -c $<

# ------------------------------------------------------------------------------
# cmds
# ------------------------------------------------------------------------------

compile: $(patsubst ${CMD}/%/main.c,${BUILD}/%,$(wildcard ${CMD}/*/main.c))

$(eval $(call objs,vault,  $(BUILD), ${ALL_LIBS}))

# ------------------------------------------------------------------------------
# tests
# ------------------------------------------------------------------------------
VM_TEST_LIBS    = ${BUILD}/vm_shape_test.o \
		  ${BUILD}/vm_tensor_test.o \
		  ${BUILD}/vm_vm_test.o \
		  ${BUILD}/vm_op_test.o

OBJ_TEST_LIBS   = ${BUILD}/obj_obj_test.o

TEST_LIBS       = ${VM_TEST_LIBS} ${OBJ_TEST_LIBS}

$(eval $(call objs,test,$(BUILD),$(VM_LIB) $(OBJ_LIB) $(TEST_LIBS)))

