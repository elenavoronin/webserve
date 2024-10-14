#executable
NAME 		= webserv
TESTNAME 	= test_cgi  # Name of the test executable

#compiler + flags
CC			= c++
# CPPFLAGS	= -Wall -Wextra -Werror -std=c++11 -g -fsanitize=address -I $(HEADERDIR)

#sources
SRC 		= 	./src/main.cpp	
TESTSRC		= 	./tests/test_cgi.cpp  # Test source file	
				
#object files
OBJDIR 		= obj
OBJ			= $(addprefix $(OBJDIR)/, $(notdir $(SRC:.cpp=.o)))

# Test object files
OBJTESTDIR 	= obj_test
OBJTEST 	= $(addprefix $(OBJTESTDIR)/, $(notdir $(TESTSRC:.cpp=.o)))

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

# Compile object files for the tests
$(OBJTESTDIR)/%.o: ./tests/%.cpp
	@ mkdir -p $(OBJTESTDIR)
	$(CC) $(CPPFLAGS) -c -o $@ $^

# Test target: compile and run the tests
test: $(OBJTEST)
	$(CC) $(CPPFLAGS) $(OBJTEST) -o $(TESTNAME)
	@ echo "${CYAN}Running tests...${DONE}"
	./$(TESTNAME)

#cleaning
clean:
	@ rm -rf $(OBJDIR) $(OBJTESTDIR)

fclean: clean
	@ rm -f $(NAME) $(TESTNAME)
	@ echo "${YELLOW}Cleaning done${DONE}"

re:	fclean all

.PHONY: all clean fclean re test