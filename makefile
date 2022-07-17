EVA_PATH        = ${HOME}/Workspace//eva
EVA_LIB         = ${EVA_PATH}/.build_release/libeva.a

include ${EVA_PATH}/eva.mk

# ------------------------------------------------------------------------------
# configurations
# ------------------------------------------------------------------------------

CMD             =  cmd
SRC             =  src
INC             =  include

CFLAGS          += -I${EVA_PATH}/src -I${SRC} -I${INC}

LDFLAGS         += ${EVA_LIB}

# ------------------------------------------------------------------------------
# configurations, required by eva.mk
# ------------------------------------------------------------------------------
FMT_FOLDERS     =  ${CMD} ${SRC} ${INC}

# ------------------------------------------------------------------------------
# libs
# ------------------------------------------------------------------------------
ALL_LIBS        = ${BUILD}/ft.o ${BUILD}/ft_walk.o ${BUILD}/ft_visit.o \
		  ${BUILD}/ft_diff.o ${BUILD}/hlog.o

# ------------------------------------------------------------------------------
# actions
# ------------------------------------------------------------------------------

.DEFAULT_GOAL   = compile

# incrementally compile all libraries.
compile: ${BUILD} ${ALL_LIBS}

# ------------------------------------------------------------------------------
# rules for sub-modules
# ------------------------------------------------------------------------------
${BUILD}/%.o: ${SRC}/%.c ${INC}/*.h
	${EVA_CC} -o $@ -c $<

${BUILD}/integration_test.o: ${CMD}/test/integration_test.c
	${EVA_CC} -o $@ -c $<

# ------------------------------------------------------------------------------
# cmds
# ------------------------------------------------------------------------------

# shortcuts
v: vault

compile: $(patsubst ${CMD}/%/main.c,${BUILD}/%,$(wildcard ${CMD}/*/main.c))

$(eval $(call objs,vault,  $(BUILD), ${ALL_LIBS}))

# ------------------------------------------------------------------------------
# tests
# ------------------------------------------------------------------------------

# unit tests
TEST_LIBS       = ${BUILD}/ft_visit_test.o ${BUILD}/ft_diff_test.o \
		  ${BUILD}/hlog_test.o

# integration
TEST_LIBS       += ${BUILD}/integration_test.o

$(eval $(call objs,test,$(BUILD),$(ALL_LIBS) $(TEST_LIBS)))

