#————————————————————————————————————————————————————————
#	ANSI
#————————————————————————————————————————————————————————

NC			:=	\033[0m
BOLD		:=	\033[1m
BLINK		:=	\033[5m

WHITE		:=	\033[97m
BLUE		:=	\033[34m
P_BLUE		:=	\033[38;2;179;235;242m
CYAN		:=	\033[36m
P_YELLOW	:=	\033[38;2;255;234;150m
GREEN		:=	\033[32m
P_GREEN		:=	\033[38;2;173;235;179m
PURPLE		:=	\033[35m
P_PURPLE	:=	\033[38;2;211;211;255m
PINK		:=	\033[38;2;255;182;193m

#————————————————————————————————————————————————————————
#	PROGRAM NAME & COMPILATION DETAILS
#————————————————————————————————————————————————————————

NAME		:=	ft_ping
INC			:=	-I includes/

CFLAGS		:=	-Wall -Wextra -Werror
DEPFLAGS	:=	-MMD -MP
LDFLAGS		:=	-lm
DEBUG		:=	-g3

define display_ascii_art
	@printf "%b\n" "$(1) ____  ____       ____  __  __ _   ___ \n(  __)(_  _)     (  _ \(  )(  ( \ / __)\n ) _)   )(  ____  ) __/ )( /    /( (_ \\n(__)   (__)(____)(__)  (__)\_)__) \___/\n"
endef

COMPILE		=	cc $(CFLAGS) $(DEPFLAGS) $(INC) -c $< -o $@

#————————————————————————————————————————————————————————
#	SOURCES
#————————————————————————————————————————————————————————

SRCS_DIR	:=	srcs/
SRCS		:=	$(sort $(shell find srcs -type f -name '*.c'))

OBJS_DIR	:=	objs/
OBJS		:=	$(patsubst $(SRCS_DIR)%.c,$(OBJS_DIR)%.o,$(SRCS))
DEPS		:=	$(OBJS:.o=.d)

#————————————————————————————————————————————————————————
#	COMPILATION PROGRESS
#————————————————————————————————————————————————————————

PROGRESS_FILE	:=	.progress_count
TOTAL_FILES		:=	$(words $(SRCS))

BAR_WIDTH		:=	10
BAR_FILL		:=	=
BAR_EMPTY		:=	.

define step
	@printf "x\n" >> $(PROGRESS_FILE); \
	C=`wc -l < $(PROGRESS_FILE)`; \
	T=$(TOTAL_FILES); \
	if [ $$T -lt $$C ]; then T=$$C; fi; \
	P=`expr $$C \* 100 / $$T`; \
	F=`expr $$C \* $(BAR_WIDTH) / $$T`; \
	B=""; V=""; I=0; \
	while [ $$I -lt $$F ]; do B="$$B$(BAR_FILL)"; I=`expr $$I + 1`; done; \
	while [ $$I -lt $(BAR_WIDTH) ]; do V="$$V$(BAR_EMPTY)"; I=`expr $$I + 1`; done; \
	printf "$(WHITE)[$(P_GREEN)%s$(WHITE)%s]$(NC) $(P_GREEN)%3d%%$(NC) $(P_PURPLE)(%d/%d)$(NC) %s\n" \
		"$$B" "$$V" "$$P" "$$C" "$$T" "$(1)"
endef

define recap
	@C=`cat $(PROGRESS_FILE) 2>/dev/null | wc -l`; \
	if [ $$C -gt 0 ]; then \
		printf "\n$(BOLD)$(P_PURPLE)Files compiled %d/%d$(NC)\n" "$$C" "$(TOTAL_FILES)"; \
	fi
	@$(RM_PROGRESS)
	@printf "$(BOLD)$(PINK)Linking $(NAME)...$(NC)\n\n"
endef

#————————————————————————————————————————————————————————
#	RULES
#————————————————————————————————————————————————————————

all: $(NAME)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c
		@mkdir -p $(dir $@)
		$(call step,$(COMPILE))
		@$(COMPILE)

-include $(DEPS)

$(NAME): $(OBJS)
		$(call recap)
		@cc $(CFLAGS) $(INC) $(OBJS) -o $(NAME) $(LDFLAGS)
		@printf "$(BOLD)========================================\n"
		$(call display_ascii_art,$(P_PURPLE))
		@printf "$(BOLD)==============$(BLINK)$(PINK)   READY!   $(NC)$(BOLD)==============$(NC)\n\n"

clean:
		rm -rf $(OBJS_DIR)
		@rm -f $(PROGRESS_FILE)
		@printf "$(BOLD)$(BLUE)[clean]:$(NC) Objects successfully removed!\n"

fclean:	clean
		rm -rf $(NAME)
		@printf "$(BOLD)$(CYAN)[fclean]:$(NC) Executable successfully removed!\n\n"

re:	fclean
		@$(MAKE) --no-print-directory all
		@printf "$(BOLD)$(P_YELLOW)./ft_ping -$(NC) Project successfully rebuilt! ✨\n"

debug: fclean
		@$(MAKE) --no-print-directory all CFLAGS="$(CFLAGS) $(DEBUG)"

.PHONY: all clean fclean re debug