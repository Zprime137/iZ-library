# Dockerfile for iZ-lib
# A mathematical library focused on prime number calculations

FROM ubuntu:22.04 AS builder

# Install required build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    gcc \
    python3 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy project files into the container
COPY . /iz-lib

# Set working directory to the project directory
WORKDIR /iz-lib

# Build the project
RUN make

# Use a smaller base image for the final image
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    python3 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create directories
RUN mkdir -p /iz-lib/build /iz-lib/output

# Copy only necessary files from builder
COPY --from=builder /iz-lib/build /iz-lib/build
COPY --from=builder /iz-lib/include /iz-lib/include
COPY --from=builder /iz-lib/output /iz-lib/output

# Set working directory
WORKDIR /iz-lib

# Set environment variables
ENV PATH="/iz-lib/build/src:${PATH}"

# Create volume for output
VOLUME /iz-lib/output

# Default command - assumes main is the executable
# Replace 'main' with your actual executable name
ENTRYPOINT ["/iz-lib/build/src/main"]
CMD []

# Add labels for better container documentation
LABEL maintainer="iZ-project Team"
LABEL version="1.0"
LABEL description="Mathematical library for prime sieve operations"
