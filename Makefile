CC := g++
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

all: ${OBJS} raycaster1c

raycaster1c: ${OBJS}
	@echo "Checking.."
	${CC} $^ -o $@

%.o: %.c
	@echo "Creating object.."
	${CC} -c $<

clean:
	@echo "Cleaning up..."
	rm -rvf *.o raycaster raycaster1c

clean-ppm:
	@echo "Removing generated images..."
	rm -rvf *.ppm