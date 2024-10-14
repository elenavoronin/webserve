#executable
NAME 		= webserv

#compiler + flags
CC			= c++
# CPPFLAGS	= -Wall -Wextra -Werror -std=c++11 -g -fsanitize=address -I $(HEADERDIR)

#sources
SRC 		= 	./src/main.cpp		
				
#object files
OBJDIR 		= obj
OBJ			= $(addprefix $(OBJDIR)/, $(notdir $(SRC:.cpp=.o)))

# Header files
HEADERDIR	= ./include

#colours
DONE		:= \033[0m
RED			:= \033[1;91m
GREEN		:= \033[1;92m
YELLOW		:= \033[1;93m
BLUE		:= \033[1;94m
PURPLE		:= \033[1;95m
CYAN		:= \033[1;96m
WHITE		:= \033[1;97m
BLACK		:= \033[1;90m

# $(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)
#targets
all:	$(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)
	@ echo "${PURPLE}webserv made!${DONE}"

$(OBJDIR)/%.o: ./src/%.cpp
	@ mkdir -p $(OBJDIR)
	$(CC) $(CPPFLAGS) -c -o $@ $^

#cleaning
clean:
	@ rm -rf $(OBJDIR)

fclean: clean
	@ rm -f $(NAME)
	@ echo "${YELLOW}Cleaning done${DONE}"

re:	fclean all

.PHONY: all clean fclean re