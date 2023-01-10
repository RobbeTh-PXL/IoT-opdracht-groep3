FROM ubuntu:latest
COPY . .
# Prerequisites Azure IOT SDK 
RUN apt-get update && apt-get -y --no-install-recommends install \ 
    git \
    cmake \
    build-essential \
    curl \
    libcurl4-openssl-dev \
    libssl-dev uuid-dev \ 
    ca-certificates
# Fetch Azure IOT SDK    
RUN git clone -b LTS_07_2022_Ref02 https://github.com/Azure/azure-iot-sdk-c.git
RUN cd azure-iot-sdk-c \
    git submodule update --init
    
# Build IOT SDK
RUN cd azure-iot-sdk-c \
mkdir cmake \
cd cmake \
cmake .. \
cmake --build .

RUN make \
    ./iothub_app
    
# Build iothub_app.c
#ENTRYPOINT ["./hello"]
