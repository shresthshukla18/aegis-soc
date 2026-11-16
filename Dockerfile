FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV AEGIS_READ_ONLY=1

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libpcap-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DAEGIS_ENABLE_PCAP=OFF \
    && cmake --build build -j2

EXPOSE 10000

CMD ["sh", "-c", "./build/aegis_soc --port ${PORT:-10000}"]
