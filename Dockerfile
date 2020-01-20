FROM ubuntu:rolling

WORKDIR /app
COPY configure.sh CMakeLists.txt ./
COPY source/ ./source
COPY header/ ./header
COPY include/ ./include
COPY templates/ ./templates
COPY cmake/ ./cmake

RUN apt update && apt install -y build-essential git cmake libboost-all-dev
RUN ./configure.sh && cd build && make

COPY build/ ./build

RUN ls

WORKDIR /app/build
CMD ["./go_cpp_server"]
