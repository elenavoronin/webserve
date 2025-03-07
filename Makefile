#executable
NAME 		= webserv
TESTNAME 	= test_cgi  # Name of the test executable

# Header files
HEADERDIR	= ./include	

#compiler + flags
CC			= c++
CPPFLAGS	= -std=c++11 -Wall -Werror -Wextra -g -fsanitize=address -I $(HEADERDIR)

#sources
SRC 		= 	./src/start.cpp				\
				./src/CGI.cpp				\
				./src/Client.cpp			\
				./src/HttpRequest.cpp 		\
				./src/HttpResponse.cpp      \
				./src/Server.cpp			\
				./src/Location.cpp			\
				./src/Config.cpp            \
				./src/utils.cpp			    \
				./src/EventPoll.cpp			\
				
#object files
OBJDIR 		= obj
OBJ			= $(SRC:./src/%.cpp=$(OBJDIR)/%.o)

# Test object files
OBJTESTDIR 	= obj_test
OBJTEST 	=  $(TESTSRC:./tests/%.cpp=$(OBJTESTDIR)/%.o)

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

#targets
all:	$(NAME)

$(NAME): $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@ echo "${PURPLE}webserv made!${DONE}"

$(OBJDIR)/%.o: ./src/%.cpp
	@ mkdir -p $(dir $@)
	@ $(CC) $(CPPFLAGS) -c -I $(HEADERDIR) -o $@ $^

# Compile object files for the tests
$(OBJTESTDIR)/%.o: ./tests/%.cpp
	@ mkdir -p $(dir $@)
	@$ (CC) $(CPPFLAGS) -c -o $@ $^

# Test target: compile and run the tests
test: $(OBJTEST)
	@ $(CC) $(CPPFLAGS) $(OBJTEST) -o $(TESTNAME)
	@ echo "${CYAN}Running tests...${DONE}"
	./$(TESTNAME)

#cleaning
clean:
	@ rm -rf $(OBJDIR) $(OBJTESTDIR)
	@ rm  -rf *.txt
	@ rm  -rf ./www/html/upload/
	@ rm -rf ./www/html/cgi-bin/data

fclean: clean
	@ rm -f $(NAME) $(TESTNAME)
	@ echo "${YELLOW}Cleaning done${DONE}"

re:	fclean all

.PHONY: all clean fclean re test
