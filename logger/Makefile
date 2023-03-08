# ==============================================================================
#	Makefile setup and global variables
# ==============================================================================
MAKEFLAGS		+=		--no-print-directory -r -R
THIS_MAKEFILE	:=		$(lastword $(MAKEFILE_LIST))
ROOT_DIR		:=		$(PWD)
ifeq ($(shell uname), Linux)
	ECHO_BIN	:=	echo -e
else ifeq ($(shell uname), Darwin)
	ECHO_BIN	:=	echo
else
	$(error "Unsupported OS")
endif

# ==============================================================================
#	Progress bar
# ==============================================================================
ifndef ECHO
HIT_TOTAL	:=	$(shell $(MAKE) $(MAKECMDGOALS) -f $(THIS_MAKEFILE) --dry-run ECHO="HIT_MARK" | grep -c "HIT_MARK")
HIT_N		:=	0
HIT_COUNT	=	$(eval HIT_N = $(shell expr $(HIT_N) + 1))$(HIT_N)
ECHO		=	$(ECHO_BIN) "[`expr $(HIT_COUNT) '*' 100 / $(HIT_TOTAL)`%]\t"
endif

# ==============================================================================
#	Build mode management
# ==============================================================================
RELEASE_MODE_FILE		:=	.release_mode

# ==============================================================================
#	Project name
# ==============================================================================
RELEASE_NAME	:=	logger

# ==============================================================================
#	Project environment
# ==============================================================================
BIN_DIR			:=		bin
OBJ_DIR			:=		obj
RELEASE_OBJDIR	:=		$(OBJ_DIR)/release
SRC_DIR			:=		src

# ==============================================================================
#	Project sources
# ==============================================================================
SRCS_FILE			:=		main.cpp log.cpp
SRCS				:=		$(addprefix $(SRC_DIR)/, $(SRCS_FILE))
OBJS				:=		$(SRCS:.cpp=.o)
RELEASE_OBJS		:=		$(addprefix $(RELEASE_OBJDIR)/, $(OBJS))

# ==============================================================================
#	Compilers
# ==============================================================================
CXX				:=	g++

# ==============================================================================
#	Compilation and linking flags
# ==============================================================================
CXX_FLAGS	:=		-Wall -Wextra -Wconversion -Wundef -Wshadow -std=c++98
CXX_FLAGS	+=		-Werror -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wswitch-default -Wswitch-enum # Those can be commented out if needed
CXX_FLAGS	+=		-MD -DDEBUG
CXX_FLAGS	+=		-I$(SRC_DIR)

LD_FLAGS	:=

ifeq ($(shell uname), Linux)
	CXX_FLAGS	+=	-DPLATFORM_LINUX
else ifeq ($(shell uname), Darwin)
	CXX_FLAGS	+=	-DPLATFORM_MACOS
else
	$(error "Unsupported OS")
endif

# ==============================================================================
#	Build mode-specific flags
# ==============================================================================
RELEASE_CXX_FLAGS	:=	-O3
RELEASE_LD_FLAGS	:=

# ==============================================================================
#	Libs
# ==============================================================================

# ==============================================================================
#	Main commands
# ==============================================================================
.PHONY: default
default:
	@if [ -f "$(RELEASE_MODE_FILE)" ]; then $(MAKE) -f $(THIS_MAKEFILE) $(BIN_DIR)/$(RELEASE_NAME); \
	else $(MAKE) -f $(THIS_MAKEFILE) release; fi

.PHONY: all
all: $(RELEASE_MODE_FILE) $(BIN_DIR)/$(RELEASE_NAME) $(BIN_DIR)/$(DEBUG_NAME) $(BIN_DIR)/$(SANITIZE_NAME)
	@$(ECHO_BIN) "[Make all]: make, make run and make re will now target $(_GREEN)release$(_END) mode"

.PHONY: run
run: default
	@if [ -f "$(RELEASE_MODE_FILE)" ]; then ./$(BIN_DIR)/$(RELEASE_NAME); fi

.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR)

.PHONY: fclean
fclean: clean
	@rm -rf $(BIN_DIR)

.PHONY: re
re: fclean
	@$(MAKE) -f $(THIS_MAKEFILE) default

# ==============================================================================
#	Build mode commands
# ==============================================================================
.PHONY: release
release: $(RELEASE_MODE_FILE) $(BIN_DIR)/$(RELEASE_NAME)
	@$(ECHO_BIN) "[Make release]: make, make run and make re will now target $(_GREEN)release$(_END) mode"

# ==============================================================================
#	Build mode file creation
# ==============================================================================

$(RELEASE_MODE_FILE):
	@rm -f $(DEBUG_MODE_FILE) $(SANITIZE_MODE_FILE)
	@touch $(RELEASE_MODE_FILE)

# ==============================================================================
#	Project workspace setup
# ==============================================================================
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ==============================================================================
#	Compilation
# ==============================================================================

#====Release build====#
$(BIN_DIR)/$(RELEASE_NAME): $(RELEASE_OBJS) Makefile | $(BIN_DIR)
	@$(ECHO) "$(_GREEN)$@$(_END)"
	@$(CXX) $(RELEASE_OBJS) -o $(BIN_DIR)/$(RELEASE_NAME) $(LD_FLAGS) $(RELEASE_LD_FLAGS)
	@$(ECHO_BIN) "$(_GREEN)[Build mode]: Release$(_END)"

$(RELEASE_OBJDIR)/%.o: %.cpp Makefile
	@$(ECHO) "$(_GREEN)$<$(_END)"
	@mkdir -p $(dir $@)
	@$(CXX) $< $(CXX_FLAGS) $(RELEASE_CXX_FLAGS) -c -o $@

-include $(RELEASE_OBJS:.o=.d)

# ==============================================================================
#	Extra
# ==============================================================================
_GREY	= \033[30m
_RED	= \033[31m
_ORANGE	= \033[38;5;209m
_GREEN	= \033[32m
_YELLOW	= \033[33m
_BLUE	= \033[34m
_PURPLE	= \033[35m
_CYAN	= \033[36m
_WHITE	= \033[37m
_END	= \033[0m
