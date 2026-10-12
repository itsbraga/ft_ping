#————————————————————————————————————————————————————————
#	ANSI
#————————————————————————————————————————————————————————

NC			:=	\033[0m
BOLD		:=	\033[1m
BLINK		:=	\033[5m

WHITE		:=	\033[97m
BLUE		:=	\033[34m
P_BLUE		:=	\033[38;2;179;235;242m
P_YELLOW	:=	\033[38;2;255;234;150m
P_GREEN		:=	\033[38;2;173;235;179m
P_PURPLE	:=	\033[38;2;211;211;255m
PINK		:=	\033[38;2;255;182;193m

#————————————————————————————————————————————————————————
#	PROGRAM NAME & COMPILATION DETAILS
#————————————————————————————————————————————————————————

NAME		:=	ft_ping
INC			:=	-I includes/

CC			:=	cc
CFLAGS		:=	-Wall -Wextra -Werror -D_GNU_SOURCE
DEPFLAGS	:=	-MMD -MP
LDFLAGS		:=	-lm
DEBUG		:=	-g3

define display_ascii_art
	@printf "%b\n" "$(1) ____  ____       ____  __  __ _   ___ \n(  __)(_  _)     (  _ \(  )(  ( \ / __)\n ) _)   )(  ____  ) __/ )( /    /( (_ \\n(__)   (__)(____)(__)  (__)\_)__) \___/\n"
endef

COMPILE		=	$(CC) $(CFLAGS) $(DEPFLAGS) $(INC) -c $< -o $@

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

TOTAL_FILES	:=	$(words $(SRCS))

BAR_WIDTH	:=	10
BAR_FILL	:=	=
BAR_EMPTY	:=	.

ifndef TO_BUILD
TO_BUILD	:=	$(shell $(MAKE) -n --no-print-directory TO_BUILD=0 $(NAME) 2>/dev/null \
					| grep -c '^$(CC) .*-o $(OBJS_DIR)')
endif

COMPILED	:=


define step
	@$(eval COMPILED += x)C=$(words $(COMPILED)); \
	T=$(TO_BUILD); \
	P=`expr $$C \* 100 / $$T`; \
	F=`expr $$C \* $(BAR_WIDTH) / $$T`; \
	B=""; V=""; I=0; \
	while [ $$I -lt $$F ]; do B="$$B$(BAR_FILL)"; I=`expr $$I + 1`; done; \
	while [ $$I -lt $(BAR_WIDTH) ]; do V="$$V$(BAR_EMPTY)"; I=`expr $$I + 1`; done; \
	printf "$(WHITE)[$(P_GREEN)%s$(WHITE)%s]$(NC) $(P_GREEN)%3d%%$(NC) %s\n" \
		"$$B" "$$V" "$$P" "$(1)"
endef

define recap
	$(if $(COMPILED),@printf "\n$(BOLD)$(P_PURPLE)Files compiled %d/%d$(NC)\n" $(words $(COMPILED)) $(TOTAL_FILES))
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
		@$(CC) $(CFLAGS) $(INC) $(OBJS) -o $(NAME) $(LDFLAGS)
		@printf "$(BOLD)========================================\n"
		$(call display_ascii_art,$(P_PURPLE))
		@printf "$(NC)$(BOLD)==============$(BLINK)$(PINK)   READY!   $(NC)$(BOLD)==============$(NC)\n\n"

clean:
		rm -rf $(OBJS_DIR)
		@rm -f $(PROGRESS_FILE)
		@printf "$(BOLD)$(BLUE)[clean]:$(NC) Objects successfully removed!\n"

fclean:	clean
		rm -rf $(NAME)
		@printf "$(BOLD)$(P_BLUE)[fclean]:$(NC) Executable successfully removed!\n\n"

re:	fclean
		@$(MAKE) --no-print-directory all
		@printf "$(BOLD)$(P_YELLOW)./ft_ping ---$(NC) Project successfully rebuilt! ✨\n"

setuid:	$(NAME)
		sudo chown root:root $(NAME)
		sudo chmod u+s $(NAME)

debug: fclean
		@$(MAKE) --no-print-directory all CFLAGS="$(CFLAGS) $(DEBUG)"

.PHONY: all clean fclean re setuid debug