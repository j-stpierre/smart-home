FROM golang:1.22-alpine

RUN apk add --no-cache git

WORKDIR /app

# We want to populate the module cache based on the go.{mod,sum} files.
COPY go.mod .
COPY go.sum .

RUN go mod download

COPY . .

RUN CGO_ENABLED=0 GOOS=linux go build -o /notifier

# Run
CMD ["/notifier"]
