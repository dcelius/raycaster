CC := g++
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

all: ${OBJS} raycaster1b

raycaster1b: ${OBJS}
	@echo "Checking.."
	${CC} $^ -o $@

%.o: %.c
	@echo "Creating object.."
	${CC} -c $<

clean:
	@echo "Cleaning up..."
	rm -rvf *.o raycaster