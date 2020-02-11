###make by kdc 2020.2.10

NAME	= seeproc
NAMECOURSE = seeprocCourse

CC	= g++

RM	= rm -f

SRCS = ./src/ProcInfo.cpp \
	  ./src/MainFrame.cpp \
	  ./src/main.cpp

COURSE	= ./src/ProcInfo.cpp \
	  ./src/MainFrame.cpp \
	  ./src/main.cpp

OBJS	= $(SRCS:.cpp=.o)
OBJCOURSE	= $(COURSE:.cpp=.o)

DEFINES = -D_DEBUG -D_UNIX -DUNIX -D_PTHREAD_SAFE
CPPFLAGS = -g -fPIC $(DEFINES) -I ./src -Wno-deprecated

all: $(NAME)

course: $(NAMECOURSE)

$(NAME): $(OBJS)
	 $(CC) $(OBJS) $(CPPFLAGS) -lncurses -lpthread -o $(NAME) $(LDFLAGS)



$(NAMECOURSE): $(OBJCOURSE)
	 $(CC) $(OBJCOURSE) $(CPPFLAGS) -lncurses -lpthread -o $(NAMECOURSE) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

recourse: fcleancourse course

fclean: clean
	$(RM) $(NAME)

fcleancourse: clean
	$(RM) $(NAMECOURSE)

re: fclean all

.PHONY: all clean fclean re fcleancourse course recourse
