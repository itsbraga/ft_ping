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
G_NEON		:=	\033[38;2;57;255;20m
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
	@printf "%b\n" "$(1) ____  ____       ____  __  __ _   ___ \n(  __)(_  _)     (  _ \(  )(  ( \ / __)\n ) _)   )(  ____  ) __/ )( /    /( (_ \\\n(__)   (__)(____)(__)  (__)\_)__) \___/\n"
endef

#————————————————————————————————————————————————————————
#	SOURCES
#————————————————————————————————————————————————————————

SRCS_DIR	:=	srcs/
SRCS		:=	$(sort $(shell find srcs -type f -name '*.c'))

OBJS_DIR	:=	objs/
OBJS		:=	$(patsubst $(SRCS_DIR)%.c,$(OBJS_DIR)%.o,$(SRCS))
DEPS		:=	$(OBJS:.o=.d)

#————————————————————————————————————————————————————————
#	PROGRESS BAR
#————————————————————————————————————————————————————————

TOTAL_FILES		:=	$(words $(SRCS))
PROGRESS_FILE	:=	.progress_count

BAR_WIDTH		:=	55
BAR_FILL		:=	█
BAR_EMPTY		:=	░

define show_progress
	@bash -c '\
	CURRENT=$$(cat $(PROGRESS_FILE) 2>/dev/null || echo 0); \
	CURRENT=$$((CURRENT + 1)); \
	echo $$CURRENT > $(PROGRESS_FILE); \
	TOTAL=$(TOTAL_FILES); \
	TERM_WIDTH=$$(tput cols 2>/dev/null || echo 80); \
	if [ $$TOTAL -gt 0 ]; then \
		PERCENT=$$((CURRENT * 100 / TOTAL)); \
		FILLED=$$((CURRENT * $(BAR_WIDTH) / TOTAL)); \
		EMPTY=$$(($(BAR_WIDTH) - FILLED)); \
		printf "\r\033[K$(BOLD)$(WHITE)["; \
		for i in $$(seq 1 $$FILLED); do printf "$(P_GREEN)$(BAR_FILL)"; done; \
		for i in $$(seq 1 $$EMPTY); do printf "$(WHITE)$(BAR_EMPTY)"; done; \
		printf "$(WHITE)] $(P_GREEN)%3d%%\t$(P_PURPLE)(%d/%d)$(NC) " $$PERCENT $$CURRENT $$TOTAL; \
		if [ $$TERM_WIDTH -ge 100 ]; then \
			printf "$(P_PURPLE)$(1)$(NC)"; \
		fi; \
		if [ $$CURRENT -eq $$TOTAL ]; then printf "\n"; fi; \
	fi'
endef

#————————————————————————————————————————————————————————
#	RULES
#————————————————————————————————————————————————————————

all: $(NAME)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c
		@if [ ! -f $(PROGRESS_FILE) ]; then \
			echo "0" > $(PROGRESS_FILE); \
			printf "$(BOLD)╔══════════════════════════════════════════════════════╗$(NC)\n"; \
			printf "$(BOLD)║$(NC)\t\t  $(BOLD)$(PINK)Compiling $(NAME)...$(NC)                 $(BOLD)║$(NC)\n"; \
			printf "$(BOLD)╚══════════════════════════════════════════════════════╝$(NC)\n\n"; \
		fi
		@mkdir -p $(dir $@)
		@cc $(CFLAGS) $(DEPFLAGS) $(INC) -c $< -o $@
		@$(call show_progress,$<)

-include $(DEPS)

$(NAME): $(OBJS)
		@cc $(CFLAGS) $(INC) $(OBJS) -o $(NAME) $(LDFLAGS)
		@rm -f $(PROGRESS_FILE)
		@printf "\033[5A\033[J"
		@sleep 1
		@echo "\n$(BOLD)======================================================\n"
		@$(call display_ascii_art,$(P_GREEN))
		@echo "\n$(NC)$(BOLD)"
		@echo "=====================$(BLINK)$(G_NEON)   READY!   $(NC)$(BOLD)=====================\n\n"

setcap:	$(NAME)
		sudo setcap cap_net_raw+ep $(NAME)

clean:
		rm -rf $(OBJS_DIR)
		@rm -f $(PROGRESS_FILE)
		@echo "$(BOLD)$(BLUE)[clean]:\t$(NC)Objects successfully removed!\n"

fclean:	clean
		rm -rf $(NAME)
		@echo "$(BOLD)$(CYAN)[fclean]:\t$(NC)Executable successfully removed!\n"

re:	fclean
		@make --no-print-directory all
		@echo "$(BOLD)$(P_YELLOW)[ft_ping] $(NC)Project successfully rebuilt! ✨\n"

debug: fclean
		@make $(NAME) CFLAGS="$(CFLAGS) $(DEBUG)"


.PHONY: all clean fclean re debug