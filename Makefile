#executable
NAME 		= webserv
TESTNAME 	= test_cgi  # Name of the test executable

#compiler + flags
CC			= c++
CPPFLAGS	= -std=c++11 -g -fsanitize=address -I $(HEADERDIR)

#sources
SRC 		= 	./src/get_ready.cpp			\
				./src/handle_data.cpp		\
				./src/request.cpp			\
				./src/run_network.cpp		\
				./src/CGI.cpp						\
				./src/Client.cpp					\
				./src/HttpRequest.cpp 				\
				./src/Server.cpp			
TESTSRC		= 	./tests/test_cgi.cpp  # Test source file	
				
#object files
OBJDIR 		= obj
OBJ			= $(SRC:./src/%.cpp=$(OBJDIR)/%.o)

# Test object files
OBJTESTDIR 	= obj_test
OBJTEST 	=  $(TESTSRC:./tests/%.cpp=$(OBJTESTDIR)/%.o)

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

#targets
all:	$(NAME)

$(NAME): $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@ echo "${PURPLE}webserv made!${DONE}"

$(OBJDIR)/%.o: ./src/%.cpp
	@ mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -c -I $(HEADERDIR) -o $@ $^

# Compile object files for the tests
$(OBJTESTDIR)/%.o: ./tests/%.cpp
	@ mkdir -p $(dir $@)
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